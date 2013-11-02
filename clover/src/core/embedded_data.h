#ifndef _EMBEDDED_DATA_H
#define _EMBEDDED_DATA_H

#include <llvm/ADT/StringRef.h>

#include <iostream>
#include <vector>

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
    inline void addElement(T element) {
      dataSet.push_back(element);
    }

    void setAddr(unsigned addr) {
      varAddr = addr;
    }

    unsigned getSize() {
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

    const T* getData() {
      return dataSet.data();
    }

  private:
    std::string name;
    unsigned varAddr;
    std::vector<T> dataSet;
    unsigned totalSize;

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

public:

  bool isEmpty() {
    return (globalWords.empty() && globalHalves.empty() && globalBytes.empty());
  }
  void addByteVariable(GlobalVariable<unsigned char> *var) {
    globalBytes.push_back(var);
  }
  void addHalfVariable(GlobalVariable<unsigned short> *var) {
    globalHalves.push_back(var);
  }
  void addWordVariable(GlobalVariable<unsigned> *var) {
    globalWords.push_back(var);
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

private:
  unsigned totalSize;

  std::vector<GlobalVariable<unsigned>*> globalWords;
  std::vector<GlobalVariable<unsigned short>*> globalHalves;
  std::vector<GlobalVariable<unsigned char>*> globalBytes;
};

}

#endif

