#ifndef _LLVM_IR_GEN_H_
#define _LLVM_IR_GEN_H_
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Pass.h>
#include <llvm/Passes/PassBuilder.h>

#include <map>

#include "syinterpret.h"

void codeGenInit();

class JITCompiler {
   private:
    llvm::LLVMContext context_;
    llvm::IRBuilder<> builder_;
    llvm::Module* module_;
    std::map<std::string, llvm::AllocaInst*> named_values_;

    llvm::ModulePassManager* module_pass_manager_;
    llvm::ModuleAnalysisManager* module_analysis_manager_;

    llvm::BasicBlock* merge_block_;
    llvm::BasicBlock* cond_block_;

    llvm::Value* expIRDispatcher(AstNodePtr exp);
    llvm::Value* stmtIRGen(AstNodePtr stmt);
    llvm::Value* blockIRGen(AstNodePtr block);
    llvm::Value* compilerError(std::string msg, int line);
    llvm::Value* numberIRGen(AstNodePtr number);
    llvm::Value* primaryExpIRGen(AstNodePtr exp);
    llvm::Value* unaryExpIRGen(AstNodePtr exp);
    llvm::Value* lValRightIRGen(AstNodePtr l_val);
    llvm::Value* lValLeftIRGen(AstNodePtr l_val);
    llvm::Value* constExpIRGen(AstNodePtr const_exp);
    llvm::Value* subExpIRGen(AstNodePtr exp);
    llvm::Value* condIRGen(AstNodePtr cond);
    llvm::Value* ifStmtIRGen(AstNodePtr stmt);
    llvm::Value* whileStmtIRGen(AstNodePtr stmt);
    llvm::Value* declIRGen(AstNodePtr decl, bool is_global);
    llvm::Value* blockItemIRGen(AstNodePtr block_item);
    llvm::AllocaInst* createFunctionArgReAlloca(llvm::Function* this_function,
                                                const llvm::Argument& arg);
    JITCompiler();

   public:
    static JITCompiler& getInstance();
    llvm::Function* functionDefinitionLLVMIrGen(AstNodePtr func);
    void addToLLVMSymbolTable(AstNodePtr func);

    ~JITCompiler(){};
};

#endif
