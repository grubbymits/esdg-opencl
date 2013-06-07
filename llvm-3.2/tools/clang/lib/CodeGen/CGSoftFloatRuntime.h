#ifndef CLANG_CODEGEN_SOFTFLOATRUNTIME_H
#define CLANG_CODEGEN_SOFTFLOATRUNTIME_H

namespace clang {

  namespace CodeGen {

    class CodeGenFunction;
    class CodeGenModule;

    class CGSoftFloatRuntime {

    protected:
      CodeGenModule &CGM;

    public:
      CGSoftFloatRuntime(CodeGenModule &CGM);
      virtual ~CGSoftFloatRuntime();

    };
  }
}

#endif
