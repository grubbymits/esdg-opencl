#include "embedded_data.h"
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>

using namespace Coal;
using namespace llvm;

template <typename T> inline void
EmbeddedData::GlobalVariable<T>::AddFPElement(llvm::ConstantFP *CFP) {
  this->addElement(CFP->getValueAPF().convertToFloat());
}

template <typename T> inline void
EmbeddedData::GlobalVariable<T>::AddIntElement(llvm::ConstantInt *CI) {
  this->addElement(CI->getZExtValue());
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
  else
    std::cerr << "Unhandled ConstantArray type" << std::endl;

  return success;
}

// Think this will handle ConstantDataArrays and ConstantDataVectors
template <typename T>
bool EmbeddedData::GlobalVariable<T>::AddElements(ConstantDataSequential *CDS) {
  unsigned numElements = CDS->getNumElements();
  llvm::Type *type = CDS->getElementType();

  if (type->isIntegerTy()) {
    for (unsigned i = 0; i < numElements; ++i)
      addElement(CDS->getElementAsInteger(i));
    return true;
  }
  else if (type->isFloatTy()) {
    for (unsigned i = 0; i < numElements; ++i)
      addElement(CDS->getElementAsFloat(i));
    return true;
  }
  else
    return false;
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
    bitWidth = getWidth(type);
  }
  else if (isa<ConstantDataSequential>(C)) {
#ifdef DBG_COMPILER
    std::cerr << "Global is ConstantDataSequential" << std::endl;
#endif
    type = (cast<ConstantDataSequential>(C))->getElementType();
    bitWidth = getWidth(type);
  }
  // ConstantStruct
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

  return totalSize;
}
