#ifndef _EMBEDDED_DATA_H
#define _EMBEDDED_DATA_H

#include <llvm/ADT/StringRef.h>

#include <iostream>
#include <vector>

namespace llvm {
  class Constant;
  class ConstantInt;
  class ConstantFP;
  class ConstantArray;
  class ConstantDataSequential;
  class ConstantStruct;
  class StructType;
}

namespace Coal {

  class EmbeddedData;

  template<class T, class Allocator = std::allocator<T> >
          class GlobalVariable;

class EmbeddedData {

public:

  template <typename T> class GlobalVariable {
  public:
    GlobalVariable(std::string ref) {
      name = ref;
    }
    bool InsertData(llvm::Constant *C);

    void setAddr(unsigned addr) {
      varAddr = addr;
    }

    virtual unsigned getSize() {
      totalSize = dataSet.size() * sizeof(T);
      totalSize += totalSize % 4;
      return totalSize;
    }

    unsigned getAddr() const {
      return varAddr;
    }

    std::string &getName() {
      return name;
    }

    const T* getData() const {
      return dataSet.data();
    }

  protected:
    virtual inline void AddElement(T element) {
      dataSet.push_back(element);
    }
  private:
    bool AddElements(llvm::ConstantArray *CA);
    bool AddElements(llvm::ConstantDataSequential *CDS);
    void AddFPElement(llvm::ConstantFP *CFP);
    void AddIntElement(llvm::ConstantInt *CI);

  protected:
    std::string name;
    unsigned varAddr;
    std::vector<T> dataSet;
    unsigned totalSize;

  };

  class GlobalStructVariable : public GlobalVariable<llvm::ConstantStruct*> {
  public:
    GlobalStructVariable(std::string ref, llvm::StructType *type) :
      GlobalVariable<llvm::ConstantStruct*>(ref), sType(type) { }

    bool AddElements(llvm::ConstantArray *CA);
    inline void AddElement(llvm::ConstantStruct *element) {
      this->dataSet.push_back(element);
    }
    unsigned getNumElements() {
      return this->dataSet.size();
    }
    unsigned getSize();
  private:
    std::string name;
    llvm::StructType *sType;

  };

  typedef typename std::vector<GlobalVariable<unsigned>*>::iterator
    word_iterator;
  typedef typename std::vector<GlobalVariable<unsigned short>*>::iterator
    half_iterator;
  typedef typename std::vector<GlobalVariable<unsigned char>*>::iterator
    byte_iterator;

  typedef typename std::vector<GlobalVariable<unsigned>*>::const_iterator
    const_word_iterator;
  typedef typename std::vector<GlobalVariable<unsigned short>*>::const_iterator
    const_half_iterator;
  typedef typename std::vector<GlobalVariable<unsigned char>*>::const_iterator
    const_byte_iterator;
  typedef typename std::vector<GlobalStructVariable*>::const_iterator
    const_struct_iterator;

public:
  bool AddVariable(llvm::Constant *C, std::string name);

  bool isEmpty() {
    return (globalWords.empty() && globalHalves.empty() && globalBytes.empty());
  }
  unsigned getTotalSize();

  const std::vector<GlobalVariable<unsigned>*>* getWords() const {
    return &globalWords;
  }

  const std::vector<GlobalVariable<unsigned short>*>* getHalves() const {
    return &globalHalves;
  }

  const std::vector<GlobalVariable<unsigned char>*>* getBytes() const {
    return &globalBytes;
  }

  const std::vector<GlobalStructVariable*>* getStructs()
    const {
      return &globalStructs;
    }

private:
  bool AddStructVariable(llvm::Constant *C, std::string &name);

  void addByteVariable(GlobalVariable<unsigned char> *var) {
    globalBytes.push_back(var);
  }
  void addHalfVariable(GlobalVariable<unsigned short> *var) {
    globalHalves.push_back(var);
  }
  void addWordVariable(GlobalVariable<unsigned> *var) {
    globalWords.push_back(var);
  }
  void addStructVariable(GlobalStructVariable *var) {
    globalStructs.push_back(var);
  }

private:
  unsigned totalSize;

  GlobalVariable<unsigned> *newWordVariable;
  GlobalVariable<unsigned short> *newHalfVariable;
  GlobalVariable<unsigned char> *newByteVariable;
  GlobalStructVariable *newStructVariable;
  std::vector<GlobalVariable<unsigned>*> globalWords;
  std::vector<GlobalVariable<unsigned short>*> globalHalves;
  std::vector<GlobalVariable<unsigned char>*> globalBytes;
  std::vector<GlobalStructVariable*> globalStructs;
};

}

#endif

