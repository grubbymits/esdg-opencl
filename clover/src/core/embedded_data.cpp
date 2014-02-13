#include "deviceinterface.h"
#include "embedded_data.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>

using namespace Coal;
using namespace llvm;

template <typename T> inline void
EmbeddedData::GlobalVariable<T>::AddFPElement(llvm::ConstantFP *CFP) {
  this->AddElement(CFP->getValueAPF().convertToFloat());
}

template <typename T> inline void
EmbeddedData::GlobalVariable<T>::AddIntElement(llvm::ConstantInt *CI) {
  this->AddElement(CI->getZExtValue());
}

template <typename T>
bool EmbeddedData::GlobalVariable<T>::InsertData(Constant *C) {
  bool success = true;

  if (isa<ConstantFP>(C))
    AddFPElement(cast<ConstantFP>(C));
  else if (isa<ConstantInt>(C))
    AddIntElement(cast<ConstantInt>(C));
  else if (isa<ConstantArray>(C))
    success = AddElements(cast<ConstantArray>(C));
  else if (isa<ConstantDataSequential>(C))
    success = AddElements(cast<ConstantDataSequential>(C));
  else {
#ifdef DBG_COMPILER
    std::cerr << "Unhandled initialised global variable" << std::endl;
#endif
    success = false;
  }

  return success;
}

template <typename T>
bool EmbeddedData::GlobalVariable<T>::AddElements(ConstantArray *CA) {
  llvm::Type *type = CA->getType()->getElementType();
  unsigned numElements = CA->getType()->getNumElements();
  bool success = false;

  if (type->isFloatTy()) {
    for (unsigned i = 0; i < numElements; ++i)
      this->AddFPElement(cast<ConstantFP>(CA->getAggregateElement(i)));
    success = true;
  }
  else if (type->isIntegerTy()) {
    for (unsigned i = 0; i < numElements; ++i)
      this->AddIntElement(cast<ConstantInt>(CA->getAggregateElement(i)));
    success = true;
  }
  else {
    std::cerr << "Unhandled ConstantArray type - ";
    if (type->isVectorTy())
      std::cerr << "vector" << std::endl;
    else
      std::cerr << std::endl;
  }

  return success;
}

// Think this will handle ConstantDataArrays and ConstantDataVectors
template <typename T>
bool EmbeddedData::GlobalVariable<T>::AddElements(ConstantDataSequential *CDS) {
  unsigned numElements = CDS->getNumElements();
  llvm::Type *type = CDS->getElementType();

  if (type->isIntegerTy()) {
    for (unsigned i = 0; i < numElements; ++i)
      AddElement(CDS->getElementAsInteger(i));
    return true;
  }
  else if (type->isFloatTy()) {
    for (unsigned i = 0; i < numElements; ++i)
      AddElement(CDS->getElementAsFloat(i));
    return true;
  }
  else
    return false;
}

bool EmbeddedData::GlobalStructVariable::AddElements(
  llvm::ConstantArray *CA) {
  unsigned numElements = CA->getType()->getNumElements();
  for (unsigned i = 0; i < numElements; ++i) {
    ConstantStruct *element =
      (cast<ConstantStruct>(CA->getAggregateElement(i)));
    this->AddElement(element);
  }
  return true;
}

unsigned EmbeddedData::GlobalStructVariable::getSize() {
  llvm::InitializeAllTargets();
  std::string Error;
  std::string Triple = device->getTriple();
  const llvm::Target *theTarget = llvm::TargetRegistry::lookupTarget(Triple,
                                                                     Error);
  llvm::TargetOptions Options;
  std::string FeatureSet;
  std::auto_ptr<llvm::TargetMachine>
    target(theTarget->createTargetMachine(Triple,
                                          device->getCPU(),
                                          FeatureSet,
                                          Options,
                                          llvm::Reloc::Static,
                                          llvm::CodeModel::Default,
                                          llvm::CodeGenOpt::Default));
  llvm::TargetMachine &TM = *target.get();
  const DataLayout *TD = TM.getDataLayout();
  unsigned size = TD->getTypeAllocSizeInBits(sType) >> 3;
  return size * dataSet.size();

  // ------------------------------------------------------------------------ //
  unsigned numElements = sType->getNumElements();
  unsigned *elementBytes = new unsigned[numElements];
  ConstantStruct *CS = this->dataSet.front();

  for (unsigned i = 0; i < numElements; ++i) {
    llvm::Type *type = CS->getAggregateElement(i)->getType();
    if (type->isFloatTy() || type->isIntegerTy(32))
      elementBytes[i] = 4;
    else if (type->isIntegerTy(16))
      elementBytes[i] = 2;
    else if (type->isIntegerTy(8))
      elementBytes[i] = 1;
    else {
      std::cerr << "Unhandled type when calculating GlobalStructVariable size"
        << std::endl;
      return -1;
    }
  }
  unsigned elementSize = elementBytes[0];
  for (unsigned i = 1; i < numElements; ++i) {
    if (elementBytes[i] == 4) {
      do {
        ++elementSize;
      } while (elementSize % 4);
      elementSize += 4;
    }
    if (elementBytes[i] == 2) {
      do {
        ++elementSize;
      } while (elementSize % 2);
      elementSize += 2;
    }
    else
      ++elementSize;
  }
  // FIXME Not sure how llvm will actually pack instances of structs. I have set
  // aggregate alignment to 32-bit in our backend.
  //  struct {
  //    int;
  //    int;
  //    char;
  //  }
  //  struct {
  //    short;
  //    int;
  //    short;
  //  }
  if (numElements > 1) {
    elementSize += (elementBytes[numElements-1] + elementBytes[0]) % 4;
  }
  return elementSize * this->dataSet.size();
}

inline static unsigned getWidth(llvm::Type *type) {
  if (type->isFloatTy())
    return 32;
  if (type->isIntegerTy(32))
    return 32;
  if (type->isIntegerTy(16))
    return 16;
  return 8;
}

bool EmbeddedData::AddVariable(Constant *C, std::string name) {
#ifdef DBG_COMPILER
  std::cerr << "EmbeddedData::AddVariable" << std::endl;
#endif
  unsigned bitWidth = 0;
  bool success = true;
  llvm::Type *type;

  if (isa<ConstantFP>(C)) {
#ifdef DBG_COMPILER
    std::cerr << "Global is ConstantFP" << std::endl;
#endif
    bitWidth = 32;
  }
  else if (isa<ConstantInt>(C)) {
#ifdef DBG_COMPILER
    std::cerr << "Global is ConstantInt" << std::endl;
#endif
    type = C->getType();
    bitWidth = getWidth(type);
  }
  else if (isa<ConstantArray>(C)) {
#ifdef DBG_COMPILER
  std::cerr << "Global is ConstantArray" << std::endl;
#endif
    type = (cast<ConstantArray>(C))->getType()->getElementType();
    if (type->isStructTy()) {
      return AddStructVariable(C, name);
    }
    bitWidth = getWidth(type);
  }
  else if (isa<ConstantDataSequential>(C)) {
#ifdef DBG_COMPILER
    std::cerr << "Global is ConstantDataSequential" << std::endl;
#endif
    type = (cast<ConstantDataSequential>(C))->getElementType();
    bitWidth = getWidth(type);
  }
  else if (isa<ConstantStruct>(C))
    return AddStructVariable(C, name);
  // ConstantVector
  // ConstantExpr
  else {
    std::cerr << "Unhandled global variable type" << std::endl;
    return false;
  }
  switch (bitWidth) {
  case 8:
    newByteVariable = new GlobalVariable<unsigned char>(name);
    success = newByteVariable->InsertData(C);
    if (success)
      addByteVariable(newByteVariable);
    break;
  case 16:
    newHalfVariable = new GlobalVariable<unsigned short>(name);
    success = newHalfVariable->InsertData(C);
    if (success)
      addHalfVariable(newHalfVariable);
    break;
  case 32:
    newWordVariable = new GlobalVariable<unsigned int>(name);
    success = newWordVariable->InsertData(C);
    if (success)
      addWordVariable(newWordVariable);
    break;
  }

  return success;
}

bool EmbeddedData::AddStructVariable(Constant *C, std::string &name) {
#ifdef DBG_COMPILER
  std::cerr << "Elements are Struct" << std::endl;
#endif
  ConstantStruct *CS = NULL;
  bool success = false;

  if (isa<ConstantStruct>(C)) {
    CS = cast<ConstantStruct>(C);
    StructType *sType = CS->getType();
    newStructVariable = new GlobalStructVariable(name, sType, device);
    newStructVariable->AddElement(CS);
    success = true;
  }
  else if (isa<ConstantArray>(C)) {
    ConstantArray *CA = cast<ConstantArray>(C);
    CS = cast<ConstantStruct>(CA->getAggregateElement(unsigned(0)));
    StructType *sType = CS->getType();
    newStructVariable = new GlobalStructVariable(name, sType, device);
    success = newStructVariable->AddElements(CA);
  }
  else {
    std::cerr << "Unhandled container of ConstantStruct" << std::endl;
    return false;
  }
  if (success)
    addStructVariable(newStructVariable);

  return success;
}


unsigned EmbeddedData::getTotalSize() {

  totalSize = 0;

  for (word_iterator WI = globalWords.begin(), WE = globalWords.end();
       WI != WE; ++WI)
    totalSize += (*WI)->getSize();

  for (half_iterator HI = globalHalves.begin(), HE = globalHalves.end();
       HI != HE; ++HI)
    totalSize += (*HI)->getSize();

  for (byte_iterator BI = globalBytes.begin(), BE = globalBytes.end();
       BI != BE; ++BI)
    totalSize += (*BI)->getSize();

  for (const_struct_iterator I = globalStructs.begin(), E = globalStructs.end();
       I != E; ++I)
    totalSize += (*I)->getSize();

  return totalSize;
}
