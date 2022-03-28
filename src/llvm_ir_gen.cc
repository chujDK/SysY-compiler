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

// in the ir gen, i use no smart ptr, because i know nothing about the llvm

static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);
static std::unique_ptr<llvm::Module> TheModule;
static std::map<std::string, llvm::Value *> NamedValues;

// helper function predefine
static llvm::Value* subExpIRGen(AstNodePtr exp);
static llvm::Value* expIRDispatcher(AstNodePtr exp);

llvm::Value* compilerError(std::string msg, int line) {
    fprintf(stderr, "\033[1m\033[31mError in compiling\033[0m: line \033[1m%d\033[0m: %s\n", line, msg.c_str());
    return nullptr;
}

static llvm::Value* numberIRGen(AstNodePtr number) {
    int value = std::stoi(number->literal_);
    return llvm::ConstantInt::get(TheContext, llvm::APInt(32, value, true));
}

static llvm::Value* primaryExpIRGen(AstNodePtr exp) {
    return expIRDispatcher(exp->a_);
}

static llvm::Value* unaryExpIRGen(AstNodePtr exp) {
    if (exp->a_->ebnf_type_ == SyEbnfType::PrimaryExp) {
        return primaryExpIRGen(exp->a_);
    }
    else {
        // TODO: finish this
        return nullptr;
    }
}

static llvm::Value* expIRDispatcher(AstNodePtr exp) {
    llvm::Value* ret = nullptr;
    switch (exp->ebnf_type_) {
        // TODO: finish this, all `return ret` is undone
        case SyEbnfType::ConstExp:
            return ret;
        case SyEbnfType::Exp:
            return expIRDispatcher(exp->a_);
        case SyEbnfType::AddExp:
        case SyEbnfType::MulExp:
        case SyEbnfType::RelExp:
        case SyEbnfType::EqExp:
        case SyEbnfType::LAndExp:
        case SyEbnfType::LOrExp:
            return subExpIRGen(exp);
        case SyEbnfType::UnaryExp:
            return unaryExpIRGen(exp);
        case SyEbnfType::PrimaryExp:
            return primaryExpIRGen(exp);
        case SyEbnfType::LVal:
            return ret;
        case SyEbnfType::Number:
            return numberIRGen(exp->a_);
        default:
            // should not reach here, trigger a bug
            assert(0);
            return ret;
    }
}

static llvm::Value* subExpIRGen(AstNodePtr exp) {
    auto exp_a = exp->a_;
    if (exp->b_ == nullptr) {
        #ifdef DEBUG
        assert(exp->c_ == nullptr);
        #endif
        return expIRDispatcher(exp->a_);
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

static llvm::Value* stmtIRGen(AstNodePtr stmt) {
    #ifdef DEBUG
    assert(stmt != nullptr && stmt->ebnf_type_ == SyEbnfType::Stmt);
    #endif

    llvm::Value* ret = nullptr;
    llvm::Value* callee_ret = nullptr;
    if (stmt->a_ == nullptr) {
        // null statement
        return ret;
    }
    switch (stmt->a_->ast_type_)
    {
        // TODO
    case SyAstType::STM_IF:
        // if (cond) stmt
        return ret;
    case SyAstType::STM_WHILE:
        return ret;
    case SyAstType::STM_BREAK:
        return ret;
    case SyAstType::STM_CONTINUE:
        return ret;
    case SyAstType::STM_RETURN:
        if (stmt->b_ != nullptr) {
            callee_ret = expIRDispatcher(stmt->b_);
        }
        return callee_ret;
    default:
        break;
    }
    // LVal '=' Exp ';' | [Exp] ';' | Block
    if (stmt->a_->ebnf_type_ == SyEbnfType::LVal) {
        // LVal '=' Exp ';'
        // TODO
    }
    else if (stmt->a_->ebnf_type_ == SyEbnfType::Exp) {
        // Exp ';'
        callee_ret = expIRDispatcher(stmt->a_);
    }
    else {
        #ifdef DEBUG
        assert(stmt->a_->ebnf_type_ == SyEbnfType::Block);
        #endif
        // TODO
    }
    return callee_ret;
}

static llvm::Value* blockItemIRGen(AstNodePtr block_item) {
    #ifdef DEBUG
    assert(block_item != nullptr && block_item->ebnf_type_ == SyEbnfType::BlockItem);
    #endif

    llvm::Value* last_val = nullptr;
    switch (block_item->a_->ebnf_type_)
    {
    case SyEbnfType::Decl:
        // TODO
        break;
    case SyEbnfType::Stmt:
        last_val = stmtIRGen(block_item->a_);
        break;
    default:
        // should not reach here
        assert(0);
        break;
    }
    return last_val;
}

static llvm::Value* blockIRGen(AstNodePtr block) {
    #ifdef DEBUG
    assert(block != nullptr && block->ebnf_type_ == SyEbnfType::Block);
    #endif
    llvm::Value* ret;
    // block should return a value for it's funciton
    for (auto block_item = block->a_; block_item != nullptr; block_item = block_item->d_) {
        ret = blockItemIRGen(block_item);
    }
    return ret;
}

llvm::Function* SYFunction::functionDefinitionLLVMIrGen() {
    // TODO: currently function only support one basic block
    llvm::Function* this_func = TheModule->getFunction(func_->b_->literal_);

    #ifdef DEBUG
    if (!this_func) {
        // this function haven't been declared yet
        assert(0);
    }
    if (!this_func->empty()) {
        // this function has generated IR code or redefined
        // this error should be caught by the interpreter
        assert(0);
    }
    #endif

    // start the function ir gen
    llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(TheContext, "entry", this_func);
    Builder.SetInsertPoint(entry_block);

    // record the function arguments in the NamedValues map
    NamedValues.clear();
    for (auto &arg : this_func->args()) {
        // i don't know why the vscode report error when i use arg.getName()
        // to pass the string to NamedValues.
        NamedValues[arg.getName()] = &arg;
    }

    // generate the function body
    auto ret_val = blockIRGen(func_->d_);
    if (ret_val != nullptr) {
        // TODO: should we do a check if this function return void?
        Builder.CreateRet(ret_val); 
    }
    else {
        // ir gen failed
        // TODO: error handling
        this_func->eraseFromParent();
    }
    return this_func;
}

void SYFunction::compile() {
    auto func_ir = functionDefinitionLLVMIrGen();
    TheModule->print(llvm::errs(), nullptr);
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
    llvm::FunctionType* func_type;
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
    llvm::Function* func_ir = llvm::Function::Create(func_type, 
      llvm::Function::ExternalLinkage, func_ident->literal_, TheModule.get());

    // set the function's argument
    func_f_param = func_->c_;
    for (auto &arg_iter : func_ir->args()) {
        arg_iter.setName(func_f_param->b_->literal_);
        func_f_param = func_f_param->d_;
    }
}

void codeGenInit() {
    TheModule = std::make_unique<llvm::Module>("JIT! COOOL!", TheContext);
}