#include <string>
#include <map>
#include <memory>
#include <cassert>
#include "llvm_ir_gen.h"

// TODO:
// we can't yet handle the array passing
// [ ] FuncFParams should added to the symbol table
// [ ] FuncFParams, // FuncFParams -> FuncFParam { ',' FuncFParam } 
// [ ] FuncFParam, // FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }] 
// [+] Decl, // Decl -> ConstDecl | VarDecl
// [+] ConstDecl, // ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
// [+] BType, // BType -> 'int'
// [ ] ConstInitVal, // ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
// [ ] VarDecl, // VarDecl -> BType VarDef { ',' VarDef } ';'
// [ ] VarDef, // VarDef -> Ident { '[' ConstExp ']' } | Ident { '[' ConstExp ']' } '=' InitVal 
// [ ] InitVal, // InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
// [+] FuncDef, // FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block 
// [+] FuncType, // FuncType -> 'void' | 'int'
// [+] Block, // Block -> '{' { BlockItem } '}' 
// [+] BlockItem, // BlockItem -> Decl | Stmt
// [+] Stmt, // Stmt -> LVal '=' Exp ';' | [Exp] ';' | Block
// [+]                                //| 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
// [+]                                //| 'while' '(' Cond ')' Stmt
// [+]                                //| 'break' ';' | 'continue' ';'
// [+]                                //| 'return' [Exp] ';'
// [+] Exp, // Exp -> AddExp
// [+] Cond, // Cond -> LOrExp
// [ ] LVal, // LVal -> Ident {'[' Exp ']'} 
// [+] PrimaryExp, // PrimaryExp -> '(' Exp ')' | LVal | Number
// [+] Number, // Number -> IntConst 
// [ ] UnaryExp, // UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
// [+] UnaryOp, // UnaryOp -> '+' | '-' | '!' 
// [ ] FuncRParams, // FuncRParams -> Exp { ',' Exp } 
// [+] MulExp, // MulExp -> UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
// [+] AddExp, // AddExp -> MulExp | AddExp ('+' | '−') MulExp 
// [+] RelExp, // RelExp -> AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
// [+] EqExp, // EqExp -> RelExp | EqExp ('==' | '!=') RelExp
// [+] LAndExp, // LAndExp -> EqExp | LAndExp '&&' EqExp
// [+] LOrExp, // LOrExp -> LAndExp | LOrExp '||' LAndExp
// [+] ConstExp, // ConstExp -> AddExp

// in the ir gen, i use no smart ptr, because i know nothing about the llvm
// currently the code is really bad, full of raw ptrs, i will refactor
// them later when i get to know better about the llvm

// maybe in the future i will refactor the ir-gen to use virtual methods of 
// each node. but currently i want to jump to the optimizer first

JITCompiler::JITCompiler(): builder_(context_) {
    module_ = std::make_unique<llvm::Module>("JIT! COOOL!", context_);
    named_values_.clear();
}

JITCompiler& JITCompiler::getInstance() {
    static JITCompiler instance;
    return instance;
}

llvm::Value* JITCompiler::compilerError(std::string msg, int line) {
    fprintf(stderr, "\033[1m\033[31mError in compiling\033[0m: line \033[1m%d\033[0m: %s\n", line, msg.c_str());
    return nullptr;
}

llvm::Value* JITCompiler::numberIRGen(AstNodePtr number) {
    int value = std::stoi(number->a_->getLiteral());
    return llvm::ConstantInt::get(context_, llvm::APInt(32, value, true));
}

llvm::Value* JITCompiler::primaryExpIRGen(AstNodePtr exp) {
    return expIRDispatcher(exp->a_);
}

llvm::Value* JITCompiler::unaryExpIRGen(AstNodePtr exp) {
    if (exp->a_->ebnf_type_ == SyEbnfType::PrimaryExp) {
        return primaryExpIRGen(exp->a_);
    }
    else if (exp->a_->ebnf_type_ == SyEbnfType::UnaryOp) {
        llvm::Value* ret = expIRDispatcher(exp->b_);
        switch (exp->a_->a_->ast_type_)
        {
        case SyAstType::ALU_ADD:
            return ret;
        case SyAstType::ALU_SUB:
            return builder_.CreateNSWSub(
              llvm::ConstantInt::get(context_, llvm::APInt(32, 0, true)), ret, "subtmp");
        case SyAstType::LOGIC_NOT:
            // %tmp = icmp ne i32 %ret, 0
            // %ans = xor i1 %tmp, true
            ret = builder_.CreateICmpNE(
              ret, llvm::ConstantInt::get(context_, llvm::APInt(32, 0, true)), "tmp");
            ret = builder_.CreateXor(ret, llvm::ConstantInt::get(context_, llvm::APInt(1, 1, true)), "nottmp");
            return builder_.CreateZExt(ret, llvm::Type::getInt32Ty(context_), "exttmp");
        default:
            break;
        }
        return nullptr;
    }
    else if (exp->a_->ast_type_ == SyAstType::IDENT) {
        // TODO: finish this
        return nullptr;
    }
    else {
        assert(0);
        return nullptr;
    }
}

llvm::Value* JITCompiler::lValRightIRGen(AstNodePtr l_val) {
    // currently, we don't consider the case that l_val is an array
    // TOOD: add support for array
    auto l_val_name = l_val->a_->getLiteral();
    auto l_val_ir = named_values_[l_val_name];
    if (l_val_ir == nullptr) {
        compilerError("undefined variable", l_val->line_);
        return nullptr;
    }
    else {
        return builder_.CreateLoad(llvm::Type::getInt32Ty(context_), l_val_ir, "loadtmp");
    }
}

llvm::Value* JITCompiler::lValLeftIRGen(AstNodePtr l_val) {
    // currently, we don't consider the case that l_val is an array
    // TOOD: add support for array
    auto l_val_name = l_val->a_->getLiteral();
    auto l_val_ir = named_values_[l_val_name];
    if (l_val_ir == nullptr) {
        // after interpreter, this should never happen
        compilerError("undefined variable", l_val->line_);
        return nullptr;
    }
    else {
        if (l_val->b_ != nullptr) {
            // TODO: finish this
            assert(0);
        }
        else {
            return l_val_ir;
        }
    }
}

llvm::Value* JITCompiler::constExpIRGen(AstNodePtr const_exp) {
    // to const exp, we know that const_exp->u_.const_val_ is valid, so just use that
    // just in case that the jit is called before interpret, we do a check const_exp->u_.const_val_
    if (const_exp->u_.const_val_ == 0xFFFFFFFF) {
        // we generate the addExp ir
        // and it will be folded in the later passes
        return subExpIRGen(const_exp->a_);
    }
    return llvm::ConstantInt::get(context_, llvm::APInt(32, const_exp->u_.const_val_, true));
}

llvm::Value* JITCompiler::expIRDispatcher(AstNodePtr exp) {
    llvm::Value* ret = nullptr;
    switch (exp->ebnf_type_) {
        // TODO: finish this, all `return ret` is undone
        case SyEbnfType::ConstExp:
            return constExpIRGen(exp);
        case SyEbnfType::Exp:
            // Exp -> AddExp
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
            return lValRightIRGen(exp);
        case SyEbnfType::Number:
            return numberIRGen(exp);
        default:
            // should not reach here, trigger a bug
            assert(0);
            return ret;
    }
}

llvm::Value* JITCompiler::subExpIRGen(AstNodePtr exp) {
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
    switch (exp->b_->ast_type_) {
    case SyAstType::ALU_ADD:
        ret_ir = builder_.CreateAdd(ir_a, ir_c, "addtmp");
        break;
    case SyAstType::ALU_SUB: 
        ret_ir = builder_.CreateSub(ir_a, ir_c, "subtmp");
        break;
    case SyAstType::ALU_DIV:
        ret_ir = builder_.CreateSDiv(ir_a, ir_c, "divtmp");
        break;
    case SyAstType::ALU_MUL:
        ret_ir = builder_.CreateMul(ir_a, ir_c, "multmp");
        break;
    case SyAstType::ALU_MOD:
        ret_ir = builder_.CreateSRem(ir_a, ir_c, "modtmp");
        break;
        // The ‘icmp’ instruction takes three operands. The first operand is the condition code indicating the kind of comparison to perform. It is not a value, just a keyword. The possible condition codes are:
        // eq: equal
        // ne: not equal
        // ugt: unsigned greater than
        // uge: unsigned greater or equal
        // ult: unsigned less than
        // ule: unsigned less or equal
        // sgt: signed greater than
        // sge: signed greater or equal
        // slt: signed less than
        // sle: signed less or equal
    case SyAstType::EQ:
        cmp_ir = builder_.CreateICmpEQ(ir_a, ir_c, "eqtmp");
        ret_ir = builder_.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(context_), true, "boolToInttmp");
        break;
    case SyAstType::NEQ:
        cmp_ir = builder_.CreateICmpNE(ir_a, ir_c, "netmp");
        ret_ir = builder_.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(context_), true, "boolToInttmp");
        break;
    case SyAstType::LNE:
        cmp_ir = builder_.CreateICmpSLT(ir_a, ir_c, "ltetmp");
        ret_ir = builder_.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(context_), true, "boolToInttmp");
        break;
    case SyAstType::LE:
        cmp_ir = builder_.CreateICmpSLE(ir_a, ir_c, "letmp");
        ret_ir = builder_.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(context_), true, "boolToInttmp");
        break;
    case SyAstType::GNE:
        cmp_ir = builder_.CreateICmpSGT(ir_a, ir_c, "gtntmp");
        ret_ir = builder_.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(context_), true, "boolToInttmp");
        break;
    case SyAstType::GE:
        cmp_ir = builder_.CreateICmpSGE(ir_a, ir_c, "getmp");
        ret_ir = builder_.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(context_), true, "boolToInttmp");
        break;
    case SyAstType::LOGIC_AND:
        ret_ir = builder_.CreateAnd(ir_a, ir_c, "andtmp");
        ret_ir = builder_.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(context_), true, "boolToInttmp");
        break;
    case SyAstType::LOGIC_OR:
        ret_ir = builder_.CreateOr(ir_a, ir_c, "ortmp");
        ret_ir = builder_.CreateIntCast(cmp_ir, llvm::Type::getInt32Ty(context_), true, "boolToInttmp");
        break;
    default:
        ret_ir = compilerError("invalid binary operator", exp->line_);
        break;
    }
    return ret_ir;
}

llvm::Value* JITCompiler::condIRGen(AstNodePtr cond) {
    return expIRDispatcher(cond->a_);
}

llvm::Value* JITCompiler::ifStmtIRGen(AstNodePtr stmt) {
    // we assert that stmt->a_->ast_type_ == SyAstType::STM_IF
    #ifdef DEBUG
    assert(stmt->a_->ast_type_ == SyAstType::STM_IF);
    #endif
    // TODO

    // thought it is a logical expression, it returns i32
    auto cond_ir = condIRGen(stmt->b_); 
    // here we convert the i32 to i1
    cond_ir = builder_.CreateICmpEQ(cond_ir, 
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 1), "ifcond");

    llvm::Function* this_function = builder_.GetInsertBlock()->getParent();

    // create a basic block for the "then" branch
    llvm::BasicBlock* then_basic_block = llvm::BasicBlock::Create(context_, "then", this_function);
    // create a basic block for the "else" branch
    // note: don't add it to the function yet
    llvm::BasicBlock* else_basic_block = llvm::BasicBlock::Create(context_, "else");

    // create a basic block for the "merge" branch
    // note: don't add it to the function yet
    llvm::BasicBlock* merge_basic_block = llvm::BasicBlock::Create(context_, "if-then-merge");

    // wow, amazing, llvm is so easy to use!
    builder_.CreateCondBr(cond_ir, then_basic_block, else_basic_block);
    
    // emit the "then" block
    builder_.SetInsertPoint(then_basic_block);
    stmtIRGen(stmt->c_);
    // jump to the merge block
    builder_.CreateBr(merge_basic_block);

    // set the then_basic_block as the current insert point is mainly for the PHI of this 
    // if-else return val in the document, as our if-else return nothing, we don't need to
    // thus the following two lines of code won't be included
    // Codegen of 'Then' can change the current block, update then_basic_block for the PHI.
    // then_basic_block = Builder.GetInsertBlock();

    // emit the "else" block
    this_function->getBasicBlockList().push_back(else_basic_block);
    builder_.SetInsertPoint(else_basic_block);
    // the else block is optional
    if (stmt->d_ != nullptr) {
        stmtIRGen(stmt->d_);
    }
    builder_.CreateBr(merge_basic_block);

    // emit the "merge" block
    this_function->getBasicBlockList().push_back(merge_basic_block);
    builder_.SetInsertPoint(merge_basic_block);
    
    return nullptr;
}

llvm::Value* JITCompiler::whileStmtIRGen(AstNodePtr stmt) {
    // we assert that stmt->a_->ast_type_ == SyAstType::STM_WHILE
    #ifdef DEBUG
    assert(stmt->a_->ast_type_ == SyAstType::STM_WHILE);
    #endif
    //| 'while' '(' Cond ')' Stmt

    llvm::Function* this_function = builder_.GetInsertBlock()->getParent();

    llvm::BasicBlock* cond_basic_block = 
      llvm::BasicBlock::Create(context_, "cond", this_function);
    llvm::BasicBlock* loop_basic_block = 
      llvm::BasicBlock::Create(context_, "loop");
    llvm::BasicBlock* merge_basic_block = 
      llvm::BasicBlock::Create(context_, "while-merge");

    cond_block_ = cond_basic_block;
    merge_block_ = merge_basic_block;

    // first we set the loop condition
    builder_.CreateBr(cond_basic_block);
    builder_.SetInsertPoint(cond_basic_block);
    llvm::Value* cond_ir = condIRGen(stmt->b_);
    cond_ir = builder_.CreateICmpEQ(cond_ir, 
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 1), "whilecond");

    // then we set the cond jmp
    builder_.CreateCondBr(cond_ir, loop_basic_block, merge_basic_block);

    // emit the loop block
    this_function->getBasicBlockList().push_back(loop_basic_block);
    builder_.SetInsertPoint(loop_basic_block);
    stmtIRGen(stmt->c_);
    builder_.CreateBr(cond_basic_block);

    this_function->getBasicBlockList().push_back(merge_basic_block);
    builder_.SetInsertPoint(merge_basic_block);

    cond_block_ = nullptr;
    merge_block_ = nullptr;

    return nullptr;
}

llvm::Value* JITCompiler::stmtIRGen(AstNodePtr stmt) {
    #ifdef DEBUG
    assert(stmt != nullptr && stmt->ebnf_type_ == SyEbnfType::Stmt);
    #endif

    llvm::Value* ret = nullptr;
    llvm::Value* callee_ret = nullptr;
    if (stmt->a_ == nullptr) {
        // null statement
        return ret;
    }
    switch (stmt->a_->ast_type_) {
    case SyAstType::STM_IF:
        // if (cond) stmt
        return ifStmtIRGen(stmt);
    case SyAstType::STM_WHILE:
        return whileStmtIRGen(stmt);
    case SyAstType::STM_BREAK:
        builder_.CreateBr(merge_block_);
        return ret;
    case SyAstType::STM_CONTINUE:
        builder_.CreateBr(cond_block_);
        return ret;
    case SyAstType::STM_RETURN:
        // TODO: maybe a return type checking? though this shoudn't be a problem here
        if (stmt->b_ != nullptr) {
            callee_ret = expIRDispatcher(stmt->b_);
            builder_.CreateRet(callee_ret);
        }
        else {
            builder_.CreateRet(nullptr);
        }
        return callee_ret;
    default:
        break;
    }
    // LVal '=' Exp ';' | [Exp] ';' | Block
    if (stmt->a_->ebnf_type_ == SyEbnfType::LVal) {
        // LVal '=' Exp ';'
        auto l_val_ir = lValLeftIRGen(stmt->a_);
        auto exp_ir = expIRDispatcher(stmt->b_);
        builder_.CreateStore(exp_ir, l_val_ir);
    }
    else if (stmt->a_->ebnf_type_ == SyEbnfType::Exp) {
        // Exp ';'
        callee_ret = expIRDispatcher(stmt->a_);
    }
    else {
        #ifdef DEBUG
        assert(stmt->a_->ebnf_type_ == SyEbnfType::Block);
        #endif
        callee_ret = blockIRGen(stmt->a_);
    }
    return callee_ret;
}

// is_global shouldn't be used, here we add it for future use
llvm::Value* JITCompiler::declIRGen(AstNodePtr decl, bool is_global) {
    #ifdef DEBUG
    assert(decl != nullptr && decl->ebnf_type_ == SyEbnfType::Decl);
    #endif
    // Decl -> ConstDecl | VarDecl
    // ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
    // VarDecl   ->         BType VarDef   { ',' VarDef } ';'

    auto btype = decl->a_->a_;
    for (auto def = decl->a_->b_; def != nullptr; def = def->d_) {
        auto ident = def->a_;
        if (def->b_ != nullptr) {
            // array
            // TODO: finish this
        }
        else {
            // alloc the memory for the variable from stack
            // thought after interpret we can use the ident->ebnf_type_ to
            // identify the type of the variable, we still use the btype
            // for the sake of simplicity if we compile this function before
            // interpret in some cases.
            llvm::AllocaInst* var_ir = nullptr;
            switch (btype->a_->ast_type_)
            {
            case SyAstType::TYPE_INT:
                var_ir = builder_.CreateAlloca(
                  llvm::Type::getInt32Ty(context_), nullptr, ident->getLiteral());
                break;
            default:
                assert(0);
                break;
            }

            // push the variable into the symbol table
            named_values_[ident->getLiteral()] = var_ir;
            auto init_val = def->c_;
            if (init_val != nullptr) {
                llvm::Value* init_val_ir = expIRDispatcher(init_val->a_);
                builder_.CreateStore(init_val_ir, var_ir);
            }
        }
    }
    return nullptr;
}

llvm::Value* JITCompiler::blockItemIRGen(AstNodePtr block_item) {
    #ifdef DEBUG
    assert(block_item != nullptr && block_item->ebnf_type_ == SyEbnfType::BlockItem);
    #endif

    llvm::Value* last_val = nullptr;
    switch (block_item->a_->ebnf_type_) {
    case SyEbnfType::Decl:
        last_val = declIRGen(block_item->a_, 0);
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

llvm::Value* JITCompiler::blockIRGen(AstNodePtr block) {
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

llvm::AllocaInst* JITCompiler::createFunctionArgReAlloca(llvm::Function* this_function, const llvm::Argument& arg) {
    llvm::IRBuilder<> tmp_builder(&this_function->getEntryBlock(),
      this_function->getEntryBlock().begin());
    llvm::AllocaInst* alloca_inst = tmp_builder.CreateAlloca(arg.getType(), nullptr, arg.getName());
    return alloca_inst;
}

llvm::Function* SYFunction::functionDefinitionLLVMIrGen() {
    return JITCompiler::getInstance().functionDefinitionLLVMIrGen(func_);
}

llvm::Function* JITCompiler::functionDefinitionLLVMIrGen(AstNodePtr func) {
    // TODO: currently function only support one basic block
    llvm::Function* this_func = module_->getFunction(func->b_->getLiteral());

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
    // always on top!
    llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(context_, "entry", this_func);
    builder_.SetInsertPoint(entry_block);

    // do the reallocation for the arguments


    // record the function arguments in the NamedValues map
    named_values_.clear();
    for (auto &arg : this_func->args()) {
        named_values_[arg.getName()] = createFunctionArgReAlloca(this_func, arg);
    }

    // generate the function body
    // TODO: maybe we should do a error handling?
    blockIRGen(func->d_);

    #ifdef DEBUG
    module_->print(llvm::errs(), nullptr);
    #endif

    return this_func;
}

void SYFunction::compile() {
    auto func_ir = functionDefinitionLLVMIrGen();
}

void SYFunction::addToLLVMSymbolTable() {
    JITCompiler::getInstance().addToLLVMSymbolTable(func_);
}

void JITCompiler::addToLLVMSymbolTable(AstNodePtr func) {
    // create this function
    // currently ignore the array type
    int argc = 0;
    auto func_type_enum = func->a_->a_->ast_type_;
    auto func_ident = func->b_;
    auto func_f_param = func->c_;
    auto func_body = func->d_;

    // set the argv's type
    std::vector<llvm::Type*> argv;
    for (; func_f_param != nullptr; func_f_param = func_f_param->d_) {
        argc++;
        switch (func_f_param->a_->a_->ast_type_) {
        case SyAstType::TYPE_INT:
            argv.push_back(llvm::Type::getInt32Ty(context_));
            break;
        default:
            compilerError("invalid argument type", func_f_param->line_);
            assert(0);
            break;
        }
    }
    
    // set the function's return type
    llvm::FunctionType* func_type;
    switch (func_type_enum) {
    case SyAstType::TYPE_INT:
        func_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context_), argv, false);
        break;
    case SyAstType::TYPE_VOID:
        func_type = llvm::FunctionType::get(llvm::Type::getVoidTy(context_), argv, false);
        break;
    default:
        compilerError("invalid function type", func->a_->line_);
        assert(0);
        break;
    }

    // TODO: i don't know if this is the right way to do it
    llvm::Function* func_ir = llvm::Function::Create(func_type, 
      llvm::Function::ExternalLinkage, func_ident->getLiteral(), module_.get());

    // set the function's argument
    func_f_param = func->c_;
    for (auto &arg_iter : func_ir->args()) {
        arg_iter.setName(func_f_param->b_->getLiteral());
        func_f_param = func_f_param->d_;
    }
}

void codeGenInit() {
}