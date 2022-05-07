#include "sysemantic.h"

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "sydebug.h"
#include "syparse.h"
#include "sytype.h"
#include "utils.h"

// pre-define some static helpers
static std::tuple<bool, std::string> typeCheckHelper(SyAstType lhs_type,
                                                     SyAstType rhs_type);

static SyAstType bTypeToValType(SyAstType type);
static std::tuple<Value, SyAstType> literalToValue(const std::string &literal);

void SemanticAnalysisVisitor::semanticError(std::string msg, int line) {
    fprintf(
        stderr,
        "\033[1m\033[31mError in semantic check\033[0m: line \033[1m%d\033[0m: "
        "%s\n",
        line, msg.c_str());
    error_ = 1;
}

void SemanticAnalysisVisitor::visitCompUnit(CompUnitAstNode &node) {
    node.getChild()->accept(*this);
}

void SemanticAnalysisVisitor::visitDecl(DeclAstNode &node) {
    // just visit the child
    node.const_decl_or_var_decl()->accept(*this);
}

static SyAstType bTypeToValType(SyAstType type) {
    switch (type) {
        case SyAstType::TYPE_INT:
            return SyAstType::VAL_TYPE_INT;
        default:
            DEBUG_ASSERT_NOT_REACH
            return SyAstType::END_OF_ENUM;
    }
}

void SemanticAnalysisVisitor::defHelper(bool is_const, SyAstType type,
                                        AstNodeBase *def) {
    // ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
    // here, we can treat the VarDef and CosntDef as the same
    // the most tricky part is to calc the init val

    // A. get the ident
    auto ident      = def->getChildAt(0);
    auto ident_name = ident->getLiteral();

    // B. test if the ident is an array. instead of using the type, we
    // directly use the ConstExp chain
    auto const_exp_chain = def->getChildAt(1);
    std::vector<int> size_for_each_dimension;
    uint32_t array_dimension = 0;
    int64_t array_size       = 1;
    if (nullptr != const_exp_chain) {
        // this is the array case
        // B.a get the dimension and it's size info
        for (auto const_exp : *const_exp_chain) {
            // visit the const_exp, and get the const_val for visitor itself
            const_exp->accept(*this);
            int val = const_exp_val_.i32;
            size_for_each_dimension.push_back(val);
            array_size *= val;
            array_dimension++;
        }
        // B.b add to the symbol table
        ArrayMemoryPtr array_mem =
            std::dynamic_pointer_cast<ArrayMemoryAPI>(symbol_table_->addSymbol(
                ident_name, valTypeToValArrayType(type), array_size, is_const));

        // B.c set the dimension and size
        array_mem->setDimension(array_dimension);
        for (int i = 0; i < array_dimension; i++) {
            array_mem->setSizeForDimension(i, size_for_each_dimension[i]);
        }

        // B.d check the init val. to const case, the init val is exist
        // promised by the parser, so we can do a assert here
        auto init_val = def->getChildAt(2);
        DEBUG_ASSERT(is_const == true ? init_val != nullptr : 1);
        if (init_val != nullptr) {
            // here we need give some context to the init val check, which
            // means the `array_mem_'
            array_mem_      = array_mem;
            const_exp_flag_ = is_const;
            init_val->accept(*this);
            // after checking, remember to clear the context
            const_exp_flag_ = false;
            array_mem_      = nullptr;
        }
    } else {
        // this is the non-array case
        // B.a add to the symbol table
        ident_mem_ = symbol_table_->addSymbol(ident_name, type, 0, is_const);
        // B.d check the init val
        auto init_val = def->getChildAt(2);
        if (init_val != nullptr) {
            // here we need give some context to the init val check, which
            // means the `ident_mem_'
            const_exp_flag_ = is_const;
            init_val->accept(*this);
            // after checking, remember to clear the context
            const_exp_flag_ = false;
            ident_mem_      = nullptr;
        }
    }
}

void SemanticAnalysisVisitor::defListHelper(bool is_const, SyAstType type,
                                            AstNodePtr defs) {
    for (auto def : *defs) {
        // delegate to defHelper for each def
        defHelper(is_const, type, def);
    }
}

void SemanticAnalysisVisitor::visitVarDecl(VarDeclAstNode &node) {
    auto b_type   = node.b_type();
    auto def_list = node.var_def();

    node.b_type()->accept(*this);
    auto type = type_;
    // delegate all the rest to the defListHelper
    defListHelper(false, type, def_list);
}

void SemanticAnalysisVisitor::visitConstDecl(ConstDeclAstNode &node) {
    auto b_type   = node.b_type();
    auto def_list = node.const_def();

    node.b_type()->accept(*this);
    auto type = type_;
    // delegate all the rest to the defListHelper
    defListHelper(true, type, def_list);
}

void SemanticAnalysisVisitor::visitConstDef(ConstDefAstNode &node) {
    // this part is delegated to the defHelper
    DEBUG_ASSERT_NOT_REACH
}

static std::tuple<bool, std::string> typeCheckHelper(SyAstType lhs_type,
                                                     SyAstType rhs_type) {
    // error can occurr when one type is non-array and one type is array
    if ((!isArrayType(lhs_type)) && (!isArrayType(rhs_type))) {
        return std::make_tuple(true, "");
    } else {
        // this part is in fact a pointer arithmetic
        return std::make_tuple(false, "pointer arithmetic is not allowed");
    }
}

void SemanticAnalysisVisitor::visitExpBase(ExpBaseAstNode &node) {
    // A. see if this is a lhs (op) rhs
    auto op  = node.op();
    auto lhs = node.lhs();
    if (op == nullptr) {
        DEBUG_ASSERT(lhs != nullptr && node.rhs() == nullptr);
        lhs->accept(*this);
        return;
    }

    // B. get the rhs
    auto rhs = node.rhs();

    // C. check the lhs and rhs
    Value lhs_val, rhs_val;
    SyAstType lhs_type, rhs_type;
    lhs->accept(*this);
    lhs_type = val_type_;
    if (const_exp_flag_) {
        lhs_val = const_exp_val_;
    }
    rhs->accept(*this);
    rhs_type = val_type_;
    if (const_exp_flag_) {
        rhs_val = const_exp_val_;
    }
    auto [check_pass, fail_msg] = typeCheckHelper(lhs_type, rhs_type);
    if (!check_pass) {
        semanticError(fail_msg, node.getLine());
    }

    // D. do the add if we are computing a const exp
    if (const_exp_flag_) {
        const_exp_val_ =
            ValueAlu(lhs_val, lhs_type, rhs_val, rhs_type, op->getAstType());
    }
}

void SemanticAnalysisVisitor::visitAddExp(AddExpAstNode &node) {
    node.ExpBaseAstNode::accept(*this);
}

void SemanticAnalysisVisitor::visitBlock(BlockAstNode &node) {
    // Block -> '{' { BlockItem } '}'
    // simply visit all the BlockItem
    for (auto block_item : node) {
        block_item->accept(*this);
    }
}

void SemanticAnalysisVisitor::visitBType(BTypeAstNode &node) {
    type_ = bTypeToValType(node.type()->getAstType());
}

void SemanticAnalysisVisitor::visitBlockItem(BlockItemAstNode &node) {
    // BlockItem -> Decl | Stmt
    // simply visit the child
    node.decl_or_stmt()->accept(*this);
}

void SemanticAnalysisVisitor::visitCond(CondAstNode &node) {
    node.l_or_exp()->accept(*this);
}

void SemanticAnalysisVisitor::visitVarDef(VarDefAstNode &node) {
    // this part is delegated to the defHelper
    DEBUG_ASSERT_NOT_REACH
}

void SemanticAnalysisVisitor::visitUnaryOp(UnaryOpAstNode &node) {
    // FIXME: add an unary op helper
    // check if the type of the operand can be used in the unary op and do
    // the op if const_exp_flag_ is set
    auto op = node.op();
    switch (op->getAstType()) {
        case SyAstType::ALU_ADD:
            if (const_exp_flag_) {  // do nothing
            }
            return;
        case SyAstType::ALU_SUB:
            if (const_exp_flag_) {
                const_exp_val_.i32 = -const_exp_val_.i32;
            }
            return;
        case SyAstType::LOGIC_NOT:
            if (const_exp_flag_) {
                const_exp_val_.i32 = !const_exp_val_.i32;
            }
            return;
        default:
            DEBUG_ASSERT_NOT_REACH
            break;
    }
    DEBUG_ASSERT_NOT_REACH
}

void SemanticAnalysisVisitor::visitRelExp(RelExpAstNode &node) {
    node.ExpBaseAstNode::accept(*this);
}

void SemanticAnalysisVisitor::visitMulExp(MulExpAstNode &node) {
    node.ExpBaseAstNode::accept(*this);
}

void SemanticAnalysisVisitor::visitConstInitVal(ConstInitValAstNode &node) {
    // InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
    if (array_mem_ != nullptr) {
        // FIXME: unimplemented
    } else {
        DEBUG_ASSERT(ident_mem_ != nullptr);
        // InitVal -> Exp
        auto const_exp  = node.const_exp();
        Value *init_val = nullptr;
        if (const_exp == nullptr) {
            // wow, error here
            semanticError("excess elements in scalar initializer",
                          node.getLine());
            // to this kind of error, just set it to 0.
            init_val = new Value(0);
        } else {
            const_exp->accept(*this);
            // type_ is set
            auto [check_pass, msg] =
                typeCheckHelper(ident_mem_->getType(), type_);
            if (!check_pass) {
                semanticError(msg, node.getLine());
            }
            // even error here, we can still set
            init_val = new Value(const_exp_val_);
        }
        ident_mem_->setInitVal(init_val);
    }
}

static std::tuple<Value, SyAstType> literalToValue(const std::string &literal) {
    // currently, only have the int literal
    // FIXME: add support for the floating point literal
    return std::make_tuple(Value(std::stoi(literal)), SyAstType::VAL_TYPE_INT);
}

void SemanticAnalysisVisitor::visitConstExp(ConstExpAstNode &node) {
    // the following code is not good, visitor shouldn't set the
    // ConstExpAstNode's data. however, it is usefull, so keep it.
    auto [calced, const_exp_val] = node.const_val();
    if (calced) {
        // good, just set the context and return
        const_exp_val_ = const_exp_val;
        return;
    } else {
        // do the calc
        const_exp_flag_ = true;
        node.add_exp()->accept(*this);
        node.setConstVal(const_exp_val_);
        const_exp_flag_ = false;
    }
}

void SemanticAnalysisVisitor::visitFuncType(FuncTypeAstNode &node) {}

void SemanticAnalysisVisitor::visitFuncDef(FuncDefAstNode &node) {}

void SemanticAnalysisVisitor::visitStmt(StmtAstNode &node) {}

void SemanticAnalysisVisitor::visitPrimaryExp(PrimaryExpAstNode &node) {
    // PrimaryExp -> '(' Exp ')' | LVal | Number
    // A. check if it is a exp
    auto exp = node.exp();
    if (exp) {
        exp->accept(*this);
        return;
    }

    // B. check if it is a lval
    auto l_val = node.l_val();
    if (l_val) {
        l_val->accept(*this);
        return;
    }

    // C. check if it is a number
    auto number = node.number();
    if (number) {
        number->accept(*this);
        return;
    }
    DEBUG_ASSERT_NOT_REACH
}

void SemanticAnalysisVisitor::visitLAndExp(LAndExpAstNode &node) {
    node.ExpBaseAstNode::accept(*this);
}

void SemanticAnalysisVisitor::visitToken(TokenAstNode &node) {
    // seems like we don't need to do anything here
}

void SemanticAnalysisVisitor::visitLOrExp(LOrExpAstNode &node) {
    node.ExpBaseAstNode::accept(*this);
}

void SemanticAnalysisVisitor::visitInitVal(InitValAstNode &node) {
    // InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
    if (array_mem_ != nullptr) {
        // FIXME: unimplemented
    } else {
        DEBUG_ASSERT(ident_mem_ != nullptr);
        // InitVal -> Exp
        auto exp = node.exp();
        if (exp == nullptr) {
            // wow, error here
            semanticError("excess elements in scalar initializer",
                          node.getLine());
            // to this kind of error, just set it to 0.
        } else {
            exp->accept(*this);
            // type_ is set
            auto [check_pass, msg] =
                typeCheckHelper(ident_mem_->getType(), type_);
            if (!check_pass) {
                semanticError(msg, node.getLine());
            }
        }
    }
}

void SemanticAnalysisVisitor::visitFuncFParams(FuncFParamsAstNode &node) {}

void SemanticAnalysisVisitor::visitFuncFParam(FuncFParamAstNode &node) {}

void SemanticAnalysisVisitor::visitExp(ExpAstNode &node) {
    node.add_exp()->accept(*this);
}

std::tuple<Value *, uint32_t>
SemanticAnalysisVisitor::arrayIndexToValueMemoryChecker(
    IdentMemoryPtr ident_mem, AstNodePtr exp_chain) {
    // in this check, only if the exp_chain is exceed the array dimension is
    // error.
    auto array_mem = std::dynamic_pointer_cast<ArrayMemoryAPI>(ident_mem);
    auto init_val  = array_mem->getInitVal();

    uint64_t offset       = 0;
    int current_dimension = 0;
    int current_delta =
        array_mem->getSize() / array_mem->getSizeForDimension(0);
    int array_dimension = array_mem->getDimension();
    for (auto exp : *exp_chain) {
        current_dimension++;
        if (current_dimension > array_dimension) {
            // this is an error
            semanticError(
                "subscripted value is neither array nor pointer nor vector",
                exp->getLine());
        }
        // here we didn't set the const_exp_flag_, because the callers will
        // take care of that
        exp->accept(*this);
        typeCheckHelper(SyAstType::VAL_TYPE_INT, type_);
        DEBUG_ASSERT(current_delta >= 1);
        offset += const_exp_val_.i32 * current_delta;
    }

    return init_val == nullptr
               ? std::make_tuple(nullptr, current_dimension)
               : std::make_tuple(init_val + offset, current_dimension);
}

void SemanticAnalysisVisitor::visitLVal(LValAstNode &node) {
    // LVal -> Ident {'[' Exp ']'}
    // A. get the l_val's memory
    auto l_val_name   = node.ident()->getLiteral();
    auto l_val_memory = symbol_table_->searchTable(l_val_name);

    // B. check if this ident exist
    if (l_val_memory == nullptr) {
        semanticError("undefined variable\033[1m" + l_val_name + "\033[0m",
                      node.getLine());
        // we don't change the context, let the error happenes. i don't care
        return;
    }

    // C. if we are in an const expression, we need to check if the l_val is
    // const
    if (const_exp_flag_) {
        if (!l_val_memory->isConst()) {
            semanticError("non const variable \033[1m" + l_val_name +
                              "\033[0m in const exp",
                          node.getLine());
        }
    }

    // D. set the context type
    auto l_val_type = l_val_memory->getType();
    type_           = l_val_type;

    // E. check if it is an array
    auto exp_chain = node.exp_chain();
    if (exp_chain) {
        // ok, it is an array
        DEBUG_ASSERT(isArrayType(l_val_type));
        // though only in the const exp, we need to get the const value,
        // still remember to call the checker
        auto [val, dimension] =
            arrayIndexToValueMemoryChecker(l_val_memory, exp_chain);
        if (dimension == std::dynamic_pointer_cast<ArrayMemoryAPI>(l_val_memory)
                             ->getDimension()) {
            type_ = valArrayTypeToValType(type_);
        }
        if (const_exp_flag_) {
            if (val == nullptr) {
                DEBUG_ASSERT_NOT_REACH
            }
            const_exp_val_ = *val;
        }
        return;
    } else {
        // non-array type
        // A. this is easy, just set the context if const_exp_flag_ is set
        if (const_exp_flag_) {
            const_exp_val_ = *l_val_memory->getInitVal();
            return;
        }
    }
}

void SemanticAnalysisVisitor::visitNumber(NumberAstNode &node) {
    // Number -> IntConst
    auto [val, type] = literalToValue(node.token()->getLiteral());
    // always set the const_exp_val_ here, won't hurt
    const_exp_val_ = val;
    val_type_      = type;
}

void SemanticAnalysisVisitor::visitUnaryExp(UnaryExpAstNode &node) {
    // UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp
    // UnaryExp

    // A. test if this is a primary exp
    auto primary_exp = node.primary_exp();
    if (primary_exp != nullptr) {
        primary_exp->accept(*this);
        return;
    }

    // B. test if this is a func call
    // FIXME: unfinished

    // C. test if this is a unary op + unary exp
    auto unary_op = node.unary_op();
    if (unary_op != nullptr) {
        node.unary_exp()->accept(*this);
        unary_op->accept(*this);
        return;
    }

    DEBUG_ASSERT_NOT_REACH
}

void SemanticAnalysisVisitor::visitFuncRParams(FuncRParamsAstNode &node) {}

void SemanticAnalysisVisitor::visitEqExp(EqExpAstNode &node) {
    node.ExpBaseAstNode::accept(*this);
}

void SemanticAnalysisVisitor::visitE(EAstNode &node) { DEBUG_ASSERT_NOT_REACH }