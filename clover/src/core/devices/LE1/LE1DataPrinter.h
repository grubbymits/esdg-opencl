#ifndef _LE1_DATA_PRINTER_H
#define _LE1_DATA_PRINTER_H

#include "../../deviceinterface.h"

#include <fstream>
#include <sstream>
#include <string>

namespace llvm {
  class StructType;
}

namespace Coal {
  class KernelEvent;
  class LE1Device;
  class LE1Kernel;


class LE1DataPrinter {
public:
  LE1DataPrinter(LE1Device *device, KernelEvent *event, const char* sourceName);
  bool AppendDataArea();
private:
  void WriteKernelAttr(std::ostringstream &Output,
                       unsigned addr,
                       size_t attr);
  void InitialiseLocal(const Kernel::Arg &arg, unsigned Addr);
  bool HandleBufferArg(const Kernel::Arg &arg);
  void PrintData(const void *data,
                 size_t addr,
                 size_t offset,
                 size_t size,
                 size_t total_bytes);
  void WriteStructData(llvm::StructType *StructArg,
                       const void *Data,
                       unsigned Addr,
                       unsigned TotalSize);
  void PrintSingleElement(const void *Data,
                          unsigned Addr,
                          unsigned *Offset,
                          size_t ElementSize);
  void PrintLine(unsigned Addr,
                 std::ostringstream &HexString,
                 std::string &BinaryString);
  private:
    KernelEvent *p_event;
    LE1Device *TheDevice;
    unsigned NumCores;
    Kernel *TheKernel;
    std::vector<unsigned> ArgAddrs;
    unsigned AttrAddrEnd;
    unsigned DataSize;
    std::ofstream FinalSource;
    const char *FinalSourceName;
};

}

#endif