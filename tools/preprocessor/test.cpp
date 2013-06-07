// Preprocessor program using clang, taken from:
// http://www.ibm.com/developerworks/library/os-createcompilerllvm2/index.html
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/Token.h>
#include <llvm/Support/Host.h>

#include <iostream>

#define NUM_ARG_ATTRS 3

using namespace clang;

struct KernelArg {
 char memory[6];
 size_t type;
 unsigned arg_num;
};

KernelArg* create_kernel_arg(const char* mem, char ty, unsigned num) {
  KernelArg* arg = new KernelArg();

  // Copy the memory area string without the preceeding '__'
  memcpy(arg->memory, mem+2, 6);

  size_t type = 0;
  switch(ty) {
  case 'i':
    type = sizeof(int);
    break;
  }
  arg->type = type;
  arg->arg_num = num;
  return arg;
}

int preprocess(char* filename) {

  CompilerInstance compiler;

  // create DiagnosticsEngine
  compiler.createDiagnostics(0,NULL);
  TargetOptions targetOptions;
  targetOptions.Triple = llvm::sys::getDefaultTargetTriple();

  // create TargetInfo
  TargetInfo* pTargetInfo =
    TargetInfo::CreateTargetInfo(compiler.getDiagnostics(), targetOptions);
  compiler.setTarget(pTargetInfo);

  // create FileManager
  compiler.createFileManager();

  // create SourceManager
  compiler.createSourceManager(compiler.getFileManager());

  // create Preprocessor
  compiler.createPreprocessor();

  const FileEntry *pFile = compiler.getFileManager().getFile(filename);
  compiler.getSourceManager().createMainFileID(pFile);
  compiler.getPreprocessor().EnterMainSourceFile();
  compiler.getDiagnosticClient().BeginSourceFile(compiler.getLangOpts(),
                                                 &compiler.getPreprocessor());
  Token token;
  std::list<KernelArg*> args;

  // Iterate through the source file until the end
  do {
    compiler.getPreprocessor().Lex(token);
    compiler.getPreprocessor().DumpToken(token);
    std::cerr << std::endl;
    /*
    // Check for the string identifying the kernel name
    if(token.isAnyIdentifier()) {
      if(token.getIdentifierInfo()->isStr("vector_mult")) {

        bool isKernelPrototype = true;
        unsigned arg_attrs = 0;
        const char* attrs[NUM_ARG_ATTRS];

        // Loop until all arguments to the kernel are discovered
        do {

          compiler.getPreprocessor().Lex(token);

          // r_paren signifies the end of the function prototype, so we can quit
          // the loop.
          if(token.getKind() == tok::r_paren)
            isKernelPrototype = false;

          else if(token.isAnyIdentifier()) {
            attrs[arg_attrs] = token.getIdentifierInfo()->getNameStart();
            ++arg_attrs;

            // There are three attributes: memory space, data type and name
            if(arg_attrs >= NUM_ARG_ATTRS) {
              args.push_back(create_kernel_arg(attrs[0], attrs[1][0],
                                               args.size()));
              arg_attrs = 0;
            }
          }

        } while (isKernelPrototype);
        break;
      }
    }*/
  } while ( token.isNot(clang::tok::eof));

  // Print argument data out and delete
  for(std::list<KernelArg*>::iterator ArgIt = args.begin();
      ArgIt != args.end(); ++ArgIt) {
    std::cout << "Argument number = " << (*ArgIt)->arg_num << std::endl;
    std::cout << "Memory Area = " << (*ArgIt)->memory << std::endl;
    std::cout << "Size of data type = " << (*ArgIt)->type << std::endl;
    delete (*ArgIt);
  }

  compiler.getDiagnosticClient().EndSourceFile();
  return 0;
}

int main(int argc, char** argv)
{
  if(argc < 2) {
    std::cout << "Provide source filename to parse\n";
    exit(1);
  }

  char* filename = argv[1];
  return preprocess(filename);
}
