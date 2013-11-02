#include "llvm/BasicBlock.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/Instruction.h"
#include "llvm/Module.h"
#include "llvm/Type.h"
#include "CodeGenModule.h"
#include "CGSoftFloatRuntime.h"

#include <iostream>

using namespace clang;
using namespace CodeGen;

CGSoftFloatRuntime::CGSoftFloatRuntime(CodeGenModule &CGM) : CGM(CGM) {
  llvm::Module &M = CGM.getModule();
  std::cerr << "Entering CGSoftFloatRuntime" << std::endl;

  // Iterate through all the functions of the module

  //llvm::Module::iterator FI = M.begin();
  //for (size_t i = 0; i < M.size(); ++i) {
  for (llvm::Module::iterator FI = M.begin(),
    FE = M.end(); FI != FE; ++FI) {


    llvm::Function *F = FI;

    std::cerr << "Iterating through function:" << F->getName().str()
      << std::endl;

    // Iterate through all the basic block of the function
    for (llvm::Function::BasicBlockListType::iterator BI = F->begin(),
         BE = F->end(); BI != BE; ++BI) {

      llvm::BasicBlock *B = BI;

      // Visit all the instructions witin the basic block
      for (llvm::BasicBlock::InstListType::iterator II = B->begin(),
           IE = B->end(); II != IE; ++II) {

        llvm::Instruction *I = II;

        if (II->getType()->getTypeID() == llvm::Type::FloatTyID) {
          std::cerr << "Found FP instruction" << std::endl;

          StringRef FuncName;
          llvm::FunctionType *FuncType; //IntegerType::getInt32Ty(C);
          std::vector<llvm::Type*> ParamTys;
          llvm::Type *Int32Ty = llvm::Type::getInt32Ty(CGM.getLLVMContext());
          llvm::Attributes Attrs
            = llvm::Attributes::get(CGM.getLLVMContext(),
                                    llvm::Attributes::NoUnwind);

          switch(I->getOpcode()) {
          default:
            break;
          case llvm::Instruction::FAdd:
            FuncName = "float32_add";
            ParamTys.push_back(Int32Ty);
            ParamTys.push_back(Int32Ty);
            FuncType = llvm::FunctionType::get(Int32Ty, ParamTys, false);
            CGM.CreateRuntimeFunction(FuncType, FuncName, Attrs);
            break;
          case llvm::Instruction::FSub:
            FuncName = "float32_sub";
            ParamTys.push_back(Int32Ty);
            ParamTys.push_back(Int32Ty);
            FuncType = llvm::FunctionType::get(Int32Ty, ParamTys, false);
            CGM.CreateRuntimeFunction(FuncType, FuncName, Attrs);
            break;
          case llvm::Instruction::FMul:
            FuncName = "float32_mul";
            ParamTys.push_back(Int32Ty);
            ParamTys.push_back(Int32Ty);
            FuncType = llvm::FunctionType::get(Int32Ty, ParamTys, false);
            CGM.CreateRuntimeFunction(FuncType, FuncName, Attrs);
            break;
          case llvm::Instruction::FDiv:
            FuncName = "float32_div";
            ParamTys.push_back(Int32Ty);
            ParamTys.push_back(Int32Ty);
            FuncType = llvm::FunctionType::get(Int32Ty, ParamTys, false);
            CGM.CreateRuntimeFunction(FuncType, FuncName, Attrs);
            break;
          case llvm::Instruction::FRem:
          case llvm::Instruction::FPToSI:
            FuncName = "float32_to_int32";
            break;
          case llvm::Instruction::UIToFP:
          case llvm::Instruction::SIToFP:
            FuncName = "int32_to_float32";
            break;
          case llvm::Instruction::FPTrunc:
          case llvm::Instruction::FPExt:
          case llvm::Instruction::FCmp:
            break;
          }
        }
      }
    }
  }
}

CGSoftFloatRuntime::~CGSoftFloatRuntime() { }
