#include "LE1DataPrinter.h"
#include "LE1buffer.h"
#include "LE1device.h"
#include "LE1kernel.h"

#include "../../events.h"
#include "../../kernel.h"
#include "../../memobject.h"

#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>

#include <iomanip>
#include <iostream>

using namespace Coal;

static std::string ConvertToBinary(unsigned int value) {
  std::string binary_string;
    unsigned mask = 0x80000000;
    while(mask > 0x00) {
      if((value & mask) != 0)
        binary_string += "1";
      else
        binary_string += "0";
      mask = mask >> 1;
    }
    return binary_string;
}

static void ConvertToBinary(std::string *binary_string,
                            const void* value,
                            size_t type) {
  unsigned mask = 0;
  switch(type) {
  case sizeof(char): {
    mask = 0x80;
    char* byte = (char*)value;
    while(mask > 0x00) {
      if((*byte & mask) != 0)
        *binary_string += "1";
      else
        *binary_string += "0";
      mask = mask >> 1;
    }
    break;
  }
  case sizeof(short): {
    mask = 0x8000;
    short* half = (short*)value;
    while(mask > 0x00) {
      if((*half & mask) != 0)
        *binary_string += "1";
      else
        *binary_string += "0";
      mask = mask >> 1;
    }
    break;
  }
  case sizeof(int): {
    mask = 0x80000000;
    unsigned* word = (unsigned*)value;
    while(mask > 0x00) {
      if((*word & mask) != 0)
        *binary_string += "1";
      else
        *binary_string += "0";
      mask = mask >> 1;
    }
    break;
  }
  default:
    std::cerr << "Error! size is too great in convert_to_binary\n";
    break;
    //exit(1);
  }
}

LE1DataPrinter::LE1DataPrinter(LE1Device *device,
                               EmbeddedData &data,
                               KernelEvent *event,
                               const char* sourceName) {
  p_event = event;
  TheKernel = event->kernel();
  TheDevice = device;
  embeddedData = data;
  FinalSourceName = sourceName;
  NumCores = TheDevice->numLE1s();
  AttrAddrEnd = 0x38;

  // The attributes end at different address depending on the number of cores.
  // Each core has a word allocated to it to store group id data, we use one
  // byte per dimension and so waste a byte. The cores can access their group_id
  // using their cpuid: group_id[((cpuid >> 8) + dim)]
  for (unsigned i = 0; i < NumCores; ++i)
    AttrAddrEnd += 4;

  unsigned CurrentAddr = AttrAddrEnd;

  // Place the embedded data before the buffers.
#ifdef DBG_KERNEL
  std::cerr << "Size of embedded data = " << embeddedData.getTotalSize()
    << std::endl;
#endif

  // Align the CurrentAddr
  while (CurrentAddr % 4)
    ++CurrentAddr;

  // Calculate the address for any embedded data
  for (EmbeddedData::const_word_iterator WI = embeddedData.getWords()->begin(),
       WE = embeddedData.getWords()->end(); WI != WE; ++WI) {

    (*WI)->setAddr(CurrentAddr);
    CurrentAddr += (*WI)->getSize();
  }

  for (EmbeddedData::const_half_iterator WI = embeddedData.getHalves()->begin(),
       WE = embeddedData.getHalves()->end(); WI != WE; ++WI) {

    (*WI)->setAddr(CurrentAddr);
    CurrentAddr += (*WI)->getSize();
  }

  for (EmbeddedData::const_byte_iterator WI = embeddedData.getBytes()->begin(),
       WE = embeddedData.getBytes()->end(); WI != WE; ++WI) {

    (*WI)->setAddr(CurrentAddr);
    CurrentAddr += (*WI)->getSize();
  }

  // Align the CurrentAddr
  while (CurrentAddr % 4)
    ++CurrentAddr;

  // Calculate buffer addresses
  for (unsigned i = 0; i < TheKernel->numArgs(); ++i) {
    const Kernel::Arg& arg = TheKernel->arg(i);

    if (arg.kind() == Kernel::Arg::Buffer) {
      llvm::Type *type = arg.type();
      unsigned bytes = 0;
      if (type->isIntegerTy(8))
        bytes = 1;
      else if (type->isIntegerTy(16))
        bytes = 2;
      else
        bytes = 4;
      while (CurrentAddr % bytes)
        ++CurrentAddr;

#ifdef DBG_KERNEL
      std::cerr << "Arg " << i << "'s address being set to "
        << std::hex << CurrentAddr << std::endl;
#endif
      ArgAddrs.push_back(CurrentAddr);

      if (arg.file() != Kernel::Arg::Local) {
        LE1Buffer* buffer = static_cast<LE1Buffer*>((*(MemObject**)
                                      arg.data())->deviceBuffer(TheDevice));
        buffer->setAddr(CurrentAddr);
      }

      if (arg.file() == Kernel::Arg::Local)
        CurrentAddr += (arg.allocAtKernelRuntime() * NumCores);
      else
        CurrentAddr += (*(MemObject**)arg.data())->size();
    }
  }

  // FIXME Should still check whether DataSize is a legal size
  DataSize = CurrentAddr;
#ifdef DBG_KERNEL
  std::cerr << "DataSize = " << DataSize << std::endl;
#endif

}

bool LE1DataPrinter::AppendDataArea() {
#ifdef DBG_KERNEL
  std::cerr << "Entering LE1DataPrinter::AppendDataArea" << std::endl;
#endif

  if (DataSize >= LE1Device::MaxGlobalAddr) {
    std::cerr << "!! ERROR: DataSize is too great, maximum address = "
      << LE1Device::MaxGlobalAddr << ", but DataSize = " << DataSize
      << std::endl;
    return false;
  }

  FinalSource.open(FinalSourceName, std::ios_base::app);
  std::ostringstream Output (std::ostringstream::out);

  Output << "##Data Labels" << std::endl;
  Output << "000000 - work_dim" << std::endl;
  Output << "000004 - global_size" << std::endl;
  Output << "000010 - local_size" << std::endl;
  Output << "00001c - num_groups" << std::endl;
  Output << "000028 - global_offset" << std::endl;
  Output << "000034 - num_cores" << std::endl;
  Output << "000038 - group_id" << std::endl;

  // Print labels for any embedded data
  for (EmbeddedData::const_word_iterator WI = embeddedData.getWords()->begin(),
       WE = embeddedData.getWords()->end(); WI != WE; ++WI) {
    Output << std::hex << std::setw(6) << std::setfill('0') << (*WI)->getAddr()
      << " - " << (*WI)->getName() << std::endl;
  }
  for (EmbeddedData::const_half_iterator WI = embeddedData.getHalves()->begin(),
       WE = embeddedData.getHalves()->end(); WI != WE; ++WI) {
    Output << std::hex << std::setw(6) << std::setfill('0') << (*WI)->getAddr()
      << " - " << (*WI)->getName() << std::endl;
  }
  for (EmbeddedData::const_byte_iterator WI = embeddedData.getBytes()->begin(),
       WE = embeddedData.getBytes()->end(); WI != WE; ++WI) {
    Output << std::hex << std::setw(6) << std::setfill('0') << (*WI)->getAddr()
      << " - " << (*WI)->getName() << std::endl;
  }

  // Print labels for buffers
  for (unsigned i = 0, j = 0; i < TheKernel->numArgs(); ++i) {
    const Kernel::Arg& arg = TheKernel->arg(i);

    if (arg.kind() == Kernel::Arg::Buffer) {

      // Sanity check
      if (arg.file() != Kernel::Arg::Local) {

        LE1Buffer* buffer =
          static_cast<LE1Buffer*>((*(MemObject**)
                                 arg.data())->deviceBuffer(TheDevice));
        if (buffer->addr() != ArgAddrs[j]) {
          std::cout << "!! ERROR: Mismatch in buffer argument addresses!: "
            << std::hex << buffer->addr() << " != " << ArgAddrs[j] << std::endl;
          return false;
        }
      }

#ifdef DBG_KERNEL
      std::cerr << "Writing arg " << i << "'s address as " << ArgAddrs[j]
        << std::endl;
#endif
      Output << std::hex << std::setw(6) << std::setfill('0') << ArgAddrs[j]
        << " - BufferArg_" << std::dec << i << std::endl;
      ++j;
    }
  }
#ifdef DBG_KERNEL
  std::cerr << "Written buffer addresses\n";
#endif

  Output << std::endl;

  // Size taken by kernel attributes before the argument data
  // FIXME is this size right?
  Output << "##Data Section - " << DataSize << " - Data_align=32" << std::endl;
  Output << "000000 - " << std::hex << std::setw(8) << std::setfill('0')
    << p_event->work_dim() << " - "
    << ConvertToBinary(p_event->work_dim())
    << std::endl;

  unsigned PrintAddr = 0x4;
  for(unsigned i = 0; i < 3; ++i) {
    WriteKernelAttr(Output, PrintAddr, p_event->global_work_size(i));
    PrintAddr += 4;
#ifdef DBG_KERNEL
    std::cerr << "Written global work size = " << p_event->global_work_size(i)
      << std::endl;
#endif
  }

  for (unsigned i = 0; i < 3; ++i) {
    WriteKernelAttr(Output, PrintAddr, p_event->local_work_size(i));
    PrintAddr += 4;
#ifdef DBG_KERNEL
    std::cerr << "Written local work size = " << p_event->local_work_size(i)
      << std::endl;
#endif
  }

  // Print the number of workgroups
  for (unsigned i = 0; i < 3; ++i) {
    if ((p_event->global_work_size(i) == 0) ||
        (p_event->local_work_size(i) == 0)) {
      WriteKernelAttr(Output, PrintAddr, 0);
    }
    else {
      unsigned NumGroups =
        p_event->global_work_size(i) / p_event->local_work_size(i);
      WriteKernelAttr(Output, PrintAddr, NumGroups);
    }
    PrintAddr += 4;
  }

#ifdef DBG_KERNEL
  std::cerr << "Written work groups\n";
#endif

  for(unsigned i = 0; i < 3; ++i) {
    WriteKernelAttr(Output, PrintAddr, p_event->global_work_offset(i));
    PrintAddr += 4;
  }
#ifdef DBG_KERNEL
  std::cerr << "Written global work offset\n";
#endif

  // Set num_cores
  WriteKernelAttr(Output, PrintAddr, NumCores);
  PrintAddr += 4;

  // Zero initalise all the group ids
  for (unsigned i = 0; i < NumCores; ++i) {
    WriteKernelAttr(Output, PrintAddr, 0);
    PrintAddr += 4;
  }

  if (AttrAddrEnd != PrintAddr) {
    std::cout << "Miscalculation in kernel attribute addresses!" << std::endl;
    std::cout << "AttrAddrEnd = " << AttrAddrEnd << " while PrintAddr = "
      << PrintAddr << std::endl;
    return false;
  }

  FinalSource << Output.str();
  FinalSource.close();

  // Print Embedded Data
  for (EmbeddedData::const_word_iterator WI = embeddedData.getWords()->begin(),
       WE = embeddedData.getWords()->end(); WI != WE; ++WI) {
    size_t totalBytes = (*WI)->getSize();
    const unsigned* data = (*WI)->getData();
    PrintData(data, PrintAddr, 0, sizeof(int), totalBytes);
    PrintAddr += totalBytes;
  }

  for (EmbeddedData::const_half_iterator WI = embeddedData.getHalves()->begin(),
       WE = embeddedData.getHalves()->end(); WI != WE; ++WI) {
    size_t totalBytes = (*WI)->getSize();
    const unsigned short* data = (*WI)->getData();
    PrintData(data, PrintAddr, 0, sizeof(short), totalBytes);
    PrintAddr += totalBytes;
  }

  for (EmbeddedData::const_byte_iterator WI = embeddedData.getBytes()->begin(),
       WE = embeddedData.getBytes()->end(); WI != WE; ++WI) {
    size_t totalBytes = (*WI)->getSize();
    const unsigned char* data = (*WI)->getData();
    PrintData(data, PrintAddr, 0, sizeof(char), totalBytes);
    PrintAddr += totalBytes;
  }

  // Handle buffers
  for(unsigned i = 0; i < TheKernel->numArgs(); ++i) {
    const Kernel::Arg& Arg = TheKernel->arg(i);

    // TODO This will need to check for images too
    if (Arg.kind() != Kernel::Arg::Buffer)
      continue;

    if (Arg.file() != Kernel::Arg::Local) {
      if (!HandleBufferArg(Arg))
        return false;
    }
    else InitialiseLocal(Arg, ArgAddrs[i]);
  }

#ifdef DBG_KERNEL
  std::cerr << "Leaving LE1DataPrinter::AppendDataArea\n";
#endif

  return true;
}

void LE1DataPrinter::WriteKernelAttr(std::ostringstream &Output,
                                     unsigned addr,
                                     size_t attr) {
  Output << std::hex << std::setw(6) << std::setfill('0') << addr << " - "
      << std::hex << std::setw(8) << std::setfill('0')
      << attr << " - "
      << ConvertToBinary(attr) << std::endl;
    //addr += 4;
}

void LE1DataPrinter::InitialiseLocal(const Kernel::Arg &arg, unsigned Addr) {
  unsigned TotalSize = arg.allocAtKernelRuntime();
  llvm::Type* type = arg.type();

  void *Data = new unsigned char[TotalSize]();

  if (type->getTypeID() == llvm::Type::IntegerTyID) {
#ifdef DBG_KERNEL
      std::cerr << "Data is int\n";
#endif
      if (type->isIntegerTy(8)) {
        PrintData(Data, Addr, 0, sizeof(char), TotalSize);
      }
      else if (type->isIntegerTy(16)) {
        PrintData(Data, Addr, 0, sizeof(short), TotalSize);
      }
      else if (type->isIntegerTy(32)) {
        PrintData(Data, Addr, 0, sizeof(int), TotalSize);
      }
    }

  delete (unsigned char*)Data;
}

bool LE1DataPrinter::HandleBufferArg(const Kernel::Arg &arg) {
#ifdef DBG_KERNEL
  std::cerr << "HandleBufferArg\n";
#endif

  llvm::Type* type = arg.type();
  LE1Buffer* buffer =
    static_cast<LE1Buffer*>((*(MemObject**)arg.data())->deviceBuffer(TheDevice));

  unsigned TotalSize = (*(MemObject**)arg.data())->size();
  unsigned Addr = buffer->addr();
  void *Data = buffer->data();

  // FIXME Check we actually have data?
  if (Data == NULL)
    Data = new unsigned char [TotalSize]();

  /*
  // FIXME - Does this work when using local buffers?
  buffer->setAddr(addr);
#ifdef DBG_KERNEL
    std::cerr << "Set buffer arg address to " << addr << std::endl;
#endif
    */
    if (type->getTypeID() == llvm::Type::IntegerTyID) {
#ifdef DBG_KERNEL
      std::cerr << "Data is int\n";
#endif
      if (type->isIntegerTy(8)) {
        PrintData(Data, Addr, 0, sizeof(char), TotalSize);
      }
      else if (type->isIntegerTy(16)) {
        PrintData(Data, Addr, 0, sizeof(short), TotalSize);
      }
      else if (type->isIntegerTy(32)) {
        PrintData(Data, Addr, 0, sizeof(int), TotalSize);
      }
    }
    else if (type->getTypeID() == llvm::Type::FloatTyID) {
#ifdef DBG_KERNEL
      std::cerr << "Data is float\n";
#endif
      PrintData(Data, Addr, 0, sizeof(int), TotalSize);
    }
    else if (type->getTypeID() == llvm::Type::VectorTyID) {
#ifdef DBG_KERNEL
      std::cerr << "Data is vector type\n";
#endif
      llvm::Type* elementType =
        static_cast<llvm::VectorType*>(type)->getElementType();

      if (elementType->getTypeID() == llvm::Type::IntegerTyID) {
#ifdef DBG_KERNEL
        std::cerr << "Elements are integers\n";
#endif
        if (elementType->isIntegerTy(8)) {
          PrintData(Data, Addr, 0, sizeof(char), TotalSize);
        }
        else if (elementType->isIntegerTy(16)) {
          PrintData(Data, Addr, 0, sizeof(short), TotalSize);
        }
        else if (elementType->isIntegerTy(32)) {
          PrintData(Data, Addr, 0, sizeof(int), TotalSize);
        }
        else {
          std::cerr << "!! unhandled vector element type!!\n";
          return false;
        }
      }
      else if (elementType->isFloatTy()) {
        PrintData(Data, Addr, 0, sizeof(int), TotalSize);
      }
      else {
        std::cerr << "!! Unhandled vector element type!" << std::endl;
        return false;
      }
    }
    else if (type->getTypeID() == llvm::Type::StructTyID) {
#ifdef DBG_KERNEL
      std::cerr << "Data is Struct\n";
#endif
      llvm::StructType* StructArg = static_cast<llvm::StructType*>(type);
      WriteStructData(StructArg, Data, Addr, TotalSize);
      //addr += TotalSize;
    }
    else {
      std::cerr << "!! Unhandled argument type of size = " << TotalSize
        << std::endl;
      return false;
    }
#ifdef DBG_KERNEL
  std::cerr << "Leaving LE1DataPrinter::HandleBufferArg\n";
#endif
  //if (isBufferOnDevice)
    //free(Data);

  return true;
}

// FIXME Just shoe-horned the address in here so probably doesn't work!
void LE1DataPrinter::WriteStructData(llvm::StructType *StructArg,
                                     const void *Data,
                                     unsigned Addr,
                                     unsigned TotalSize) {
  // Chars are aligned on byte boundaries, short on half word boundaries
  // and ints are word aligned. So it could mean we need to pack data like
  // this:
  // char | char | short    |
  // int                    |
  // Print data is not currently setup to handle that type of packing, it zero
  // pads to finish the final line, expecting a large lump of data, of the same
  // type. So we need a new function, and we need to remember that the data will
  // also already be packed for x86_64, but it looks like that it matches the
  // LE1.
  // FIXME are pointers allowed in structs?

  unsigned NumElements = StructArg->getNumElements();
#ifdef DBG_KERNEL
  std::cerr << "Data is struct type, size = " << TotalSize << " with "
    << NumElements << " elements" << std::endl;
#endif

  // DataWritten acts as a counter, as well as an offset value to the data
  // pointer that we pass to print from.
  unsigned DataWritten = 0;
  while (DataWritten < TotalSize) {

    for (unsigned i = 0; i < NumElements; ++i) {
      llvm::Type *ElementType = StructArg->getTypeAtIndex(i);

      if (ElementType->getTypeID() == llvm::Type::IntegerTyID) {
        if (ElementType->isIntegerTy(32))
          PrintSingleElement(Data, Addr, &DataWritten, sizeof(int));
        else if (ElementType->isIntegerTy(16))
          PrintSingleElement(Data, Addr, &DataWritten, sizeof(short));
        else if (ElementType->isIntegerTy(8))
          PrintSingleElement(Data, Addr, &DataWritten, sizeof(char));
      }
      else if (ElementType->isFloatTy())
        PrintSingleElement(Data, Addr, &DataWritten, sizeof(float));
    }
  }
}

// This function is used for writing elements of a structure. It takes single
// elements and automatically inserts the correct padding and adjusts the data
// pointer offset accordingly. The data is actually output a line at a time, so
// this function may be called several times before the output file is even
// changed.
// FIXME This needs to be updated to use a dynamic address
void LE1DataPrinter::PrintSingleElement(const void *Data,
                                        unsigned Addr,
                                        unsigned *Offset,
                                        size_t ElementSize) {
  static unsigned ByteCount = 0;
  static std::string BinaryString;
  static std::ostringstream HexString;
  unsigned ZeroPad = 0;

  // FIXME Where should we update 'addr'?

  switch(ElementSize) {
  case sizeof(char): {
    const char* Byte = static_cast<const char*>(Data) + *Offset;
    std::string ByteString;
    ConvertToBinary(&ByteString, Byte, sizeof(char));
    BinaryString.append(ByteString);
    HexString << std::setw(2) << std::setfill('0') << *Byte;
    ++ByteCount;
    ++(*Offset);
    break;
  }
  case sizeof(short): {
    if ((ByteCount == 0) || (ByteCount == 2)) {
      std::string HalfString;
      const short* Half = static_cast<const short*>(Data) + (*Offset >> 1);
      ConvertToBinary(&HalfString, Half, sizeof(short));
      BinaryString.append(HalfString);
      HexString << std::setw(4) << std::setfill('0') << *Half;
      ByteCount += 2;
      *Offset += 2;
    }
    else if (ByteCount == 1) {
      std::string PadString;
      ConvertToBinary(&PadString, &ZeroPad, sizeof(char));
      ++(*Offset);
      const short *Half = static_cast<const short*>(Data) + (*Offset >> 1);
      std::string HalfString;
      ConvertToBinary(&HalfString, Half, sizeof(short));
      BinaryString.append(PadString).append(HalfString);
      HexString << std::setw(2) << std::setfill('0') << 0;
      HexString << std::setw(4) << std::setfill('0') << *Half;
      ByteCount += 3;
      *Offset += 2;
    }
    else {
      std::string PadString;
      ConvertToBinary(&PadString, &ZeroPad, sizeof(char));
      BinaryString.append(PadString);
      HexString << std::setw(2) << std::setfill('0') << 0;
      // Write the pad finished line
      PrintLine((Addr + *Offset), HexString, BinaryString);

      // Start a new line
      ++(*Offset);
      const short *Half = static_cast<const short*>(Data) + (*Offset >> 1);
      std::string HalfString;
      ConvertToBinary(&HalfString, Half, sizeof(short));
      BinaryString.append(HalfString);
      HexString << std::setw(4) << std::setfill('0') << *Half;
    }
    break;
  }
  case sizeof(int): {
    if (ByteCount == 0) {
      std::string WordString;
      const int *Word = static_cast<const int*>(Data) + (*Offset >> 2);
      ConvertToBinary(&WordString, Word, sizeof(int));
      BinaryString.append(WordString);
      HexString << std::hex << std::setw(8) << std::setfill('0') << *Word;
      *Offset += 4;
    }
    else {
      std::string PadString;
      ConvertToBinary(&PadString, &ZeroPad, sizeof(char));
      for (unsigned i = 0; i < (4 - ByteCount); ++i) {
        BinaryString.append(PadString);
        HexString << std::setw(2) << std::setfill('0') << 0;
        ++(*Offset);
      }
      // Need to write the line that just got pad finished
      PrintLine((Addr + *Offset), HexString, BinaryString);

      // Then create the new line
      const int *Word = static_cast<const int*>(Data) + (*Offset >> 2);
      ConvertToBinary(&BinaryString, Word, sizeof(int));
      HexString << std::setw(8) << std::setfill('0') << *Word;
      *Offset += 4;
    }
    ByteCount = 0;
    break;
  }
  }

  ByteCount = ByteCount % 4;

  // If ByteCount is 0, it means we need to write the data line to the file,
  // and start a new line.
  if (ByteCount == 0) {
    PrintLine((Addr + *Offset), HexString, BinaryString);
  }
}

inline void LE1DataPrinter::PrintLine(unsigned Addr,
                                      std::ostringstream &HexString,
                                      std::string &BinaryString) {

  FinalSource.open(FinalSourceName, std::ios_base::app);
  std::ostringstream DataLine(std::ostringstream::out);

  // Write the address
  DataLine << std::hex << std::setw(6) << std::setfill('0')
    << (Addr - 4) << " - ";

  // Write data in hex
  DataLine << HexString.str();

  // Write data in binary
  DataLine << " - " << BinaryString << std::endl;
  BinaryString.erase();
  HexString.str("");

  FinalSource << DataLine.str();
  FinalSource.close();
}


void LE1DataPrinter::PrintData(const void* data,
                               size_t addr,
                               size_t offset,
                               size_t size,
                               size_t total_bytes) {
#ifdef DBG_KERNEL
  std::cerr << "Entered print_data\n";
  std::cerr << "Start address = " << addr << std::endl;
  std::cerr << "Offset = " << offset << ", total bytes = " << total_bytes
    << std::endl;
#endif

  unsigned device_mem_ptr = addr;
  FinalSource.open(FinalSourceName, std::ios_base::app);

  // Divide the number by 4 to get the number of full 32-bit data lines
  // index is used to calculate the jump with elements since we're outputting
  // 32-bits at a time.
  //unsigned total_bytes = size * num_items;
  unsigned index = 0;

  for (unsigned i = 0; i < (total_bytes >> 2); ++i) {
    std::ostringstream data_line (std::ostringstream::out);
    std::string binary_string("\0");

    switch(size) {
    default:
      std::cerr << "!!! ERROR: Unhandled type!\n";
      exit(-1);
      break;
    case sizeof(char): {
      const char* bytes = static_cast<const char*>(data) + offset;
      index = i << 2;
      bytes = &bytes[index];

      // Write formatted data to stream
      data_line << std::hex << std::setw(6) << std::setfill('0')
        << device_mem_ptr << " - ";
      for (unsigned i = 0; i < 4 ; ++i) {
        ConvertToBinary(&binary_string, &bytes[i], sizeof(char));
        data_line << std::setw(2) << std::setfill('0')
          << static_cast<int>(bytes[i]);
      }
      data_line << " - " << binary_string << std::endl;
      break;
    }
    case sizeof(short): {
      const short* halves = static_cast<const short*>(data) + offset;
      index = i << 1;
      halves = &halves[index];

      // Write formatted data to stream
      data_line << std::hex << std::setw(6) << std::setfill('0')
        << device_mem_ptr << " - ";
      for (unsigned i = 0; i < 2; ++i) {
        ConvertToBinary(&binary_string, &halves[i], sizeof(short));
        data_line << std::setw(4) << std::setfill('0') << halves[i];
      }
      data_line << " - "  << binary_string << std::endl;
      break;
    }
    case sizeof(int): {
      const unsigned* word = static_cast<const unsigned*>(data) + offset;
      index = i;
      //word = &word[index];
      ConvertToBinary(&binary_string, &word[index], sizeof(int));
      // Write formatted data to stream
      data_line << std::hex << std::setw(6) << std::setfill('0')
        << device_mem_ptr << " - ";
      data_line << std::setw(8) << std::setfill('0') << word[index];
      data_line << " - " << binary_string << std::endl;
      break;
    }
    }
  
    // Write stream to file
    FinalSource << data_line.str();
    device_mem_ptr += 4;
  }
  // Mask total number of bytes with 0011. If the result is 0
  // then the number of bytes is divisible by 4. If not the function can be
  // called with a value that is zero padded.
  unsigned remaining_bytes = total_bytes & 0x3;

  if(remaining_bytes != 0) {

    std::ostringstream data_line (std::ostringstream::out);
    std::string binary_string("\0");
    switch(size) {
      case sizeof(char): {
        // Write a zero padded array and integer value to fill a whole line
        const char* remaining_data = static_cast<const char*>(data) + offset;
        //remaining_data = &remaining_data[index+4];
        unsigned padded_data = 0;
        char padded_array[4] = {0};
        for(unsigned i = 0; i < remaining_bytes; ++i) {
          padded_data |= remaining_data[i] << (12 << i);
          padded_array[i] = remaining_data[i];
        }
        // Write data to the stream
        data_line  << std::hex << std::setw(6) << std::setfill('0')
          << device_mem_ptr << " - ";
        for(unsigned i = 0; i < 4; ++i) {
          ConvertToBinary(&binary_string, &padded_array[i], sizeof(char));
          data_line << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(padded_array[i]);
        }
        data_line << " - " << binary_string << std::endl;
        break;
      }
      case sizeof(short): {
        // Write a zero padded array and integer value to fill a whole line
        const short* remaining_data = static_cast<const short*>(data) + offset;
        //remaining_data = &remaining_data[index+4];
        unsigned padded_data = 0;
        short padded_array[2] = {0};
        for(unsigned i = 0; i < (remaining_bytes >> 1); ++i) {
          padded_data |= remaining_data[i] << (i << 16);
          padded_array[i] = remaining_data[i];
        }
        // Write data to the stream
        data_line << std::hex << std::setw(6) << device_mem_ptr << " - ";
        for (unsigned i = 0; i < 2; ++i) {
          ConvertToBinary(&binary_string, &padded_array[i], sizeof(short));
          data_line << std::hex << std::setw(4) << std::setfill('0')
            << static_cast<int>(padded_array[0]);
        }
        data_line << " - " << binary_string << std::endl;
        break;
      }
    }
    // Write stream out to file
    FinalSource << data_line.str();
    device_mem_ptr += 4;
  }

  FinalSource.close();
  //addr = device_mem_ptr;
#ifdef DBG_KERNEL
  std::cerr << "Leaving print_data\n";
#endif

}
