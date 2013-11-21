#ifndef _LE1_NDRANGE_H
#define _LE1_NDRANGE_H

namespace Coal {

  class KernelEvent;
  class LE1Program;
  class LE1Device;

  class LE1NDRange {
  public:
    LE1NDRange(KernelEvent *event, LE1Device *device);
    ~LE1NDRange();
    bool CompileSource();
    bool RunSim();
    bool RunRTL() { return false; }

  private:
    void CreateLauncher();
    bool Finalise();
    bool UpdateBuffers();

  private:
    static std::map<std::string, std::pair<unsigned*, unsigned*> > kernelRanges;
    LE1Device *theDevice;
    Kernel *theKernel;
    KernelEvent *theEvent;
    EmbeddedData embeddedData;
    unsigned totalCores;
    unsigned disabledCores;
    unsigned workDims;
    unsigned *globalWorkSize;
    unsigned *localWorkSize;
    unsigned workgroupsPerCore[3];

    std::string OriginalSource;
    std::string dram;
    std::string iram;
    std::string OriginalSourceName;
    std::string KernelName;
    std::string CoarsenedSourceName;
    std::string CoarsenedBCName;
    std::string TempAsmName;
    std::string FinalBCName;
    std::string FinalAsmName;
    std::string CompleteFilename;
    std::string LauncherString;
  };
}

#endif
