#include <string>
#include <map>
#include <memory>
#include <cassert>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Value.h>
#include "llvm_ir_gen.h"

static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);
static std::unique_ptr<llvm::Module> TheModule;
static std::map<std::string, llvm::Value *> NamedValues;

llvm::Value* compilerError(std::string msg, int line) {
    fprintf(stderr, "\033[1m\033[31mError in compiling\033[0m: line \033[1m%d\033[0m: %s\n", line, msg.c_str());
    return nullptr;
}

static llvm::Value* numberIRGen(AstNodePtr node) {
    int value = std::stoi(node->literal_);
    return llvm::ConstantInt::get(TheContext, llvm::APInt(32, value, true));
}

static llvm::Value* expIRDispatcher(AstNodePtr exp) {
}

static llvm::Value* subExpIRGen(AstNodePtr exp) {
    auto exp_a = exp->a_;
    if (exp->b_ == nullptr) {
        #ifdef DEBUG
        assert(exp->c_ == nullptr);
        #endif
        return expIRDispatcher(exp);
    }
    auto exp_c = exp->c_;
    auto ir_a = expIRDispatcher(exp_a);
    auto ir_c = expIRDispatcher(exp_c);
    llvm::Value* ret_ir;
    llvm::Value* cmp_ir;
    switch (exp->b_->ast_type_)
    {
    case SyAstType::ALU_ADD:
        ret_ir = Builder.CreateFAdd(ir_a, ir_c, "addtmp");
        break;
    case SyAstType::ALU_SUB: 
        ret_ir = Builder.CreateFSub(ir_a, ir_c, "subtmp");
        break;
    case SyAstType::ALU_DIV:
        ret_ir = Builder.CreateFDiv(ir_a, ir_c, "divtmp");
        break;
    case SyAstType::ALU_MUL:
        ret_ir = Builder.CreateFMul(ir_a, ir_c, "multmp");
        break;
    case SyAstType::ALU_MOD:
        ret_ir = Builder.CreateFRem(ir_a, ir_c, "modtmp");
        break;
    case SyAstType::EQ:
        cmp_ir = Builder.CreateFCmpOEQ(ir_a, ir_c, "eqtmp");
        ret_ir = Builder.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(TheContext), true, "boolToInttmp");
        break;
    case SyAstType::NEQ:
        cmp_ir = Builder.CreateFCmpONE(ir_a, ir_c, "netmp");
        ret_ir = Builder.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(TheContext), true, "boolToInttmp");
        break;
    case SyAstType::LNE:
        cmp_ir = Builder.CreateFCmpOLT(ir_a, ir_c, "ltetmp");
        ret_ir = Builder.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(TheContext), true, "boolToInttmp");
        break;
    case SyAstType::LE:
        cmp_ir = Builder.CreateFCmpOLE(ir_a, ir_c, "letmp");
        ret_ir = Builder.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(TheContext), true, "boolToInttmp");
        break;
    case SyAstType::GNE:
        cmp_ir = Builder.CreateFCmpOGT(ir_a, ir_c, "gtntmp");
        ret_ir = Builder.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(TheContext), true, "boolToInttmp");
        break;
    case SyAstType::GE:
        cmp_ir = Builder.CreateFCmpOGE(ir_a, ir_c, "getmp");
        ret_ir = Builder.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(TheContext), true, "boolToInttmp");
        break;
    case SyAstType::LOGIC_AND:
        ret_ir = Builder.CreateAnd(ir_a, ir_c, "andtmp");
        ret_ir = Builder.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(TheContext), true, "boolToInttmp");
        break;
    case SyAstType::LOGIC_OR:
        ret_ir = Builder.CreateOr(ir_a, ir_c, "ortmp");
        ret_ir = Builder.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(TheContext), true, "boolToInttmp");
        break;
    default:
        ret_ir = compilerError("invalid binary operator", exp->line_);
        break;
    }
    return ret_ir;
}

void SYFunction::addToLLVMSymbolTable() {
    // create this function
    // currently ignore the array type
    int argc = 0;
    auto func_type_enum = func_->a_->a_->ast_type_;
    auto func_ident = func_->b_;
    auto func_f_param = func_->c_;
    auto func_body = func_->d_;

    // set the argv's type
    std::vector<llvm::Type*> argv;
    for (; func_f_param != nullptr; func_f_param = func_f_param->d_) {
        argc++;
        switch (func_f_param->a_->a_->ast_type_)
        {
        case SyAstType::TYPE_INT:
            argv.push_back(llvm::Type::getInt32Ty(TheContext));
            break;
        default:
            compilerError("invalid argument type", func_f_param->line_);
            assert(0);
            break;
        }
    }
    
    // set the function's return type
    llvm::FunctionType *func_type;
    switch (func_type_enum)
    {
    case SyAstType::TYPE_INT:
        func_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(TheContext), argv, false);
        break;
    case SyAstType::TYPE_VOID:
        func_type = llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), argv, false);
        break;
    default:
        compilerError("invalid function type", func_->a_->line_);
        assert(0);
        break;
    }

    // TODO: i don't know if this is the right way to do it
    llvm::Function *func_ir = llvm::Function::Create(func_type, 
      llvm::Function::ExternalLinkage, func_ident->literal_, TheModule.get());

    // set the function's argument
    func_f_param = func_->c_;
    for (auto &arg_iter : func_ir->args()) {
        arg_iter.setName(func_f_param->b_->literal_);
        func_f_param = func_f_param->d_;
    }
}
