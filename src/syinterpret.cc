#include <cassert>
#include <cstdint>
#include <string>
#include <cstring>
#include "syinterpret.h"

void Interpreter::interpretWarning(std::string msg, int line) {
    fprintf(stderr, "\033[1m\033[35mWarning in executing\033[0m: line \033[1m%d\033[0m: %s\n", line, msg.c_str());
}

void Interpreter::interpretError(std::string msg, int line) {
    fprintf(stderr, "\033[1m\033[31mError in executing\033[0m: line \033[1m%d\033[0m: %s\n", line, msg.c_str());
    error_occured_ = 1;
}

Value Interpreter::expDispatcher(AstNodePtr exp) {
    switch (exp->ebnf_type_) {
        case SyEbnfType::ConstExp:
            return constExpHandler(exp);
        case SyEbnfType::AddExp:
            return addExpHandler(exp);
        case SyEbnfType::MulExp:
            return mulExpHandler(exp);
        case SyEbnfType::RelExp:
            return relExpHandler(exp);
        case SyEbnfType::EqExp:
            return eqExpHandler(exp);
        case SyEbnfType::LAndExp:
            return lAndExpHandler(exp);
        case SyEbnfType::LOrExp:
            return lOrExpHandler(exp);
        case SyEbnfType::UnaryExp:
            return unaryExpHandler(exp);
        case SyEbnfType::Exp:
            return expHandler(exp);
        case SyEbnfType::PrimaryExp:
            return primaryExpHandler(exp);
        case SyEbnfType::LVal:
            return lValHandler(exp);
        case SyEbnfType::Number:
            return numberHandler(exp);
        default:
            // should not reach here, trigger a bug
            assert(0);
            return Value();
    }
}

Value Interpreter::lValHandler(AstNodePtr exp) {
    return Value();
}

Value Interpreter::numberHandler(AstNodePtr exp) {
    Value ret;
    ret.i32 = std::stoi(exp->a_->literal_);
    return ret;
}

Value Interpreter::primaryExpHandler(AstNodePtr exp) {
    // PrimaryExp -> '(' Exp ')' | LVal | Number
    return expDispatcher(exp->a_);
}

Value Interpreter::unaryExpHandler(AstNodePtr exp) {
    // UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
    // TODO: FINISH THIS
    // currently only retunr the val of PrimaryExp
    if (exp->a_->ebnf_type_ == SyEbnfType::PrimaryExp) {
        return primaryExpHandler(exp);
    }
    return Value();
}

// adjust:
// ast:      AddExp
//          /  |  \
//     MulExp '+' AddExp(L) type: AddExp
//               /   |   \
//           MulExp '+' MulExp type: END_OF_ENUM => MulExp
//
// after adjust, all AddExpL is a AddExp

Value Interpreter::addExpHandler(AstNodePtr exp) {
    // AddExp -> MulExp | AddExp ('+' | '-') MulExp 
    // add_exp->a_->ebnf_type_ == MulExp;
    // add_exp->b_->ast_type_ == ALU_ADD | ALU_SUB
    // add_exp->c_->ebnf_type_ == MulExp | AddExp;
    #ifdef DEBUG
    assert(exp->a_->ebnf_type_ == SyEbnfType::MulExp);
    if (exp->b_ != nullptr) {
        assert(exp->b_->ast_type_ == SyAstType::ALU_ADD || 
            exp->b_->ast_type_ == SyAstType::ALU_SUB);
        assert(exp->c_->ebnf_type_ == SyEbnfType::MulExp ||
            exp->c_->ebnf_type_ == SyEbnfType::AddExp);
    }
    #endif
    Value ret;
    auto exp_a = exp->a_;
    if (exp->b_ == nullptr) {
        #ifdef DEBUG
        assert(exp->c_ == nullptr);
        #endif
        return expDispatcher(exp_a);
    }
    auto exp_c = exp->c_;
    Value val_a = expDispatcher(exp_a);
    Value val_c = expDispatcher(exp_c);
    switch (exp->b_->ast_type_) {
        case SyAstType::ALU_ADD:
            ret.i32 = val_a.i32 + val_c.i32;
            break;
        case SyAstType::ALU_SUB:
            ret.i32 = val_a.i32 - val_c.i32;
            break;
        default:
            // shouldn't reach here trigger a bug
            assert(1!=1);
            break;
    }
    return ret;
}

Value Interpreter::relExpHandler(AstNodePtr exp) {
    // RelExp -> AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
    #ifdef DEBUG
    assert(exp->a_->ebnf_type_ == SyEbnfType::AddExp);
    if (exp->b_ != nullptr) {
        assert(exp->b_->ast_type_ == SyAstType::LE ||
            exp->b_->ast_type_ == SyAstType::LNE ||
            exp->b_->ast_type_ == SyAstType::GE ||
            exp->b_->ast_type_ == SyAstType::GNE);
        assert(exp->c_->ebnf_type_ == SyEbnfType::RelExp ||
            exp->c_->ebnf_type_ == SyEbnfType::AddExp);
    }
    #endif
    Value ret;
    auto exp_a = exp->a_;
    if (exp->b_ == nullptr) {
        #ifdef DEBUG
        assert(exp->c_ == nullptr);
        #endif
        return expDispatcher(exp_a);
    }
    auto exp_c = exp->c_;
    Value val_a = expDispatcher(exp_a);
    Value val_c = expDispatcher(exp_c);
    switch (exp->b_->ast_type_) {
        case SyAstType::LE:
            ret.i32 = val_a.i32 <= val_c.i32;
            break;
        case SyAstType::GE:
            ret.i32 = val_a.i32 >= val_c.i32;
            break;
        case SyAstType::GNE:
            ret.i32 = val_a.i32 > val_c.i32;
            break;
        case SyAstType::LNE:
            ret.i32 = val_a.i32 < val_c.i32;
            break;
        default:
            // shouldn't reach here trigger a bug
            assert(1!=1);
            break;
    }
    return ret;
}

Value Interpreter::mulExpHandler(AstNodePtr exp) {
    // MulExp -> UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
    #ifdef DEBUG
    assert(exp->a_->ebnf_type_ == SyEbnfType::UnaryExp);
    if (exp->b_ != nullptr) {
        assert(exp->b_->ast_type_ == SyAstType::ALU_MUL ||
            exp->b_->ast_type_ == SyAstType::ALU_DIV ||
            exp->b_->ast_type_ == SyAstType::ALU_DIV);
        assert(exp->c_->ebnf_type_ == SyEbnfType::UnaryExp ||
            exp->c_->ebnf_type_ == SyEbnfType::MulExp);
    }
    #endif
    Value ret;
    auto exp_a = exp->a_;
    if (exp->b_ == nullptr) {
        #ifdef DEBUG
        assert(exp->c_ == nullptr);
        #endif
        return expDispatcher(exp_a);
    }
    auto exp_c = exp->c_;
    Value val_a = expDispatcher(exp_a);
    Value val_c = expDispatcher(exp_c);
    switch (exp->b_->ast_type_) {
        case SyAstType::ALU_MUL:
            ret.i32 = val_a.i32 * val_c.i32;
            break;
        case SyAstType::ALU_DIV:
            ret.i32 = val_a.i32 / val_c.i32;
            break;
        case SyAstType::ALU_MOD:
            ret.i32 = val_a.i32 % val_c.i32;
            break;
        default:
            // shouldn't reach here trigger a bug
            assert(1!=1);
            break;
    }
    return ret;

}

Value Interpreter::eqExpHandler(AstNodePtr exp) {
    // EqExp -> RelExp | EqExp ('==' | '!=') RelExp
    #ifdef DEBUG
    assert(exp->a_->ebnf_type_ == SyEbnfType::RelExp);
    if (exp->b_ != nullptr) {
        assert(exp->b_->ast_type_ == SyAstType::EQ ||
            exp->b_->ast_type_ == SyAstType::NEQ);
        assert(exp->c_->ebnf_type_ == SyEbnfType::RelExp ||
            exp->c_->ebnf_type_ == SyEbnfType::EqExp);
    }
    #endif
    Value ret;
    auto exp_a = exp->a_;
    if (exp->b_ == nullptr) {
        #ifdef DEBUG
        assert(exp->c_ == nullptr);
        #endif
        return expDispatcher(exp_a);
    }
    auto exp_c = exp->c_;
    Value val_a = expDispatcher(exp_a);
    Value val_c = expDispatcher(exp_c);
    switch (exp->b_->ast_type_) {
        case SyAstType::EQ:
            ret.i32 = val_a.i32 == val_c.i32;
            break;
        case SyAstType::NEQ:
            ret.i32 = val_a.i32 != val_c.i32;
            break;
        default:
            // shouldn't reach here trigger a bug
            assert(1!=1);
            break;
    }
    return ret;
}

Value Interpreter::lAndExpHandler(AstNodePtr exp) {
    // LAndExp -> EqExp | LAndExp '&&' EqExp
    #ifdef DEBUG
    assert(exp->a_->ebnf_type_ == SyEbnfType::EqExp);
    if (exp->b_ != nullptr) {
        assert(exp->b_->ast_type_ == SyAstType::LOGIC_AND);
        assert(exp->c_->ebnf_type_ == SyEbnfType::EqExp ||
            exp->c_->ebnf_type_ == SyEbnfType::LAndExp);
    }
    #endif
    Value ret;
    auto exp_a = exp->a_;
    if (exp->b_ == nullptr) {
        #ifdef DEBUG
        assert(exp->c_ == nullptr);
        #endif
        return expDispatcher(exp_a);
    }
    auto exp_c = exp->c_;
    Value val_a = expDispatcher(exp_a);
    Value val_c = expDispatcher(exp_c);
    switch (exp->b_->ast_type_) {
        case SyAstType::LOGIC_AND:
            ret.i32 = val_a.i32 && val_c.i32;
            break;
        default:
            // shouldn't reach here trigger a bug
            assert(1!=1);
            break;
    }
    return ret;

}

Value Interpreter::lOrExpHandler(AstNodePtr exp) {
    // LOrExp -> LAndExp | LOrExp '||' LAndExp
    #ifdef DEBUG
    assert(exp->a_->ebnf_type_ == SyEbnfType::LAndExp);
    if (exp->b_ != nullptr) {
        assert(exp->b_->ast_type_ == SyAstType::LOGIC_OR);
        assert(exp->c_->ebnf_type_ == SyEbnfType::LOrExp ||
            exp->c_->ebnf_type_ == SyEbnfType::LAndExp);
    }
    #endif
    Value ret;
    auto exp_a = exp->a_;
    if (exp->b_ == nullptr) {
        #ifdef DEBUG
        assert(exp->c_ == nullptr);
        #endif
        return expDispatcher(exp_a);
    }
    auto exp_c = exp->c_;
    Value val_a = expDispatcher(exp_a);
    Value val_c = expDispatcher(exp_c);
    switch (exp->b_->ast_type_) {
        case SyAstType::LOGIC_AND:
            ret.i32 = val_a.i32 || val_c.i32;
            break;
        default:
            // shouldn't reach here trigger a bug
            assert(1!=1);
            break;
    }
    return ret;
}

inline Value Interpreter::constExpHandler(AstNodePtr const_exp) {
    if (const_exp->u_.const_val_ == 0xFFFFFFFF) {
        const_exp->u_.const_val_ = addExpHandler(const_exp->a_).i32;
    }
    Value val;
    val.i32 = const_exp->u_.const_val_;
    return val;
}

inline Value Interpreter::expHandler(AstNodePtr exp) {
    return addExpHandler(exp->a_);
}

// need test
// this is currently quiet plain, we can use template to make it more
// generic to handle all types
// TODO: currently just support exact init_val, 
// this is not support 
// `int arr1[0x10][0x20][0x30] = {{{0x10, 0x11, 0x12}}, {{0x20, 0x21, 0x22}}, {{0x30, 0x31, 0x32}}}`
void Interpreter::initValHandler(AstNodePtr init_val, AstNodePtr const_exp, 
char* mem_raw, int demention, int size_delta) {
    // we have pretty much confidence that all the const_exp is already computed
    assert(mem_raw != nullptr);

    if (demention == 0) {
        #ifdef DEBUG
        if (init_val->a_->ebnf_type_ != SyEbnfType::ConstExp &&
            init_val->a_->ebnf_type_ != SyEbnfType::Exp) {
            assert(init_val->a_->ebnf_type_ == SyEbnfType::ConstExp ||
                init_val->a_->ebnf_type_ == SyEbnfType::Exp);
        }
        #endif
        *((int*)mem_raw) = constExpHandler(init_val->a_).i32;
        return;
    }
    size_delta = size_delta / const_exp->u_.const_val_;
    auto init_val_list_current = init_val->a_;
    for (int i = 0; i < const_exp->u_.const_val_; i++) {
        initValHandler(init_val_list_current->a_, const_exp->d_, mem_raw, demention - 1, size_delta);
        init_val_list_current = init_val_list_current->d_;
        if (init_val_list_current == nullptr) {
            return;
        }
        mem_raw += size_delta;
    }
}

void Interpreter::declHandler(AstNodePtr decl, bool is_global) {
    // Decl -> ConstDecl | VarDecl
    // ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
    // VarDecl   ->         BType VarDef   { ',' VarDef   } ';'
    // VarDef   -> Ident { '[' ConstExp ']' } 
    //          -> Ident { '[' ConstExp ']' } '=' InitVal 
    // ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal 
    // ConstExp -> AddExp

    for (auto def = decl->a_->b_; def != nullptr; def = def->d_) {
        auto ident = def->a_;
        if (def->b_ != nullptr) {
            int demension = 0;
            int length;
            uint64_t arr_size = 1; // arr_size is the number of elements in the array
            for (auto const_exp = def->b_; const_exp != nullptr; const_exp = const_exp->d_) {
                // example: int a[10][0x10] = {{0}, {0}}, b;
                // def: a[10][0x10] = {{0}, {0}}
                // ident: a
                // init_val: {{0}, {0}}
                // const_exp: 10, 0x10

                // b won't enter this loop
                demension++;
                if (const_exp->u_.const_val_ == 0xFFFFFFFF) {
                    // it's ok if the val of const_exp is really 0xFFFFFFFF
                    // we just recompute it
                    length = constExpHandler(const_exp).i32;
                    const_exp->u_.const_val_ = length;
                }
                arr_size *= length;
                if (arr_size > UINT32_MAX) {
                    interpretError("array size overflow", const_exp->line_);
                    return;
                }
            }
            // example: int a[10][0x10] = {{0}, {0}}, b;
            // arr_size: 10 * 0x10
            // add this ident to symbol_table_
            ident->ebnf_type_ = SyEbnfType::TYPE_INT_ARRAY;
            ident->u_.array_size_ = arr_size;
            auto mem = (is_global ? symbol_table_->addGlobalSymbol(ident) : symbol_table_->addSymbol(ident));
            auto mem_raw = mem->getMem();
            if (is_global) {
                memset(mem_raw, 0, arr_size * sizeof(int));
            }

            // init_val: {{0}, {0}}
            // recursively add the init_val
            auto init_val = def->c_;
            auto const_exp = def->b_;
            if (init_val != nullptr) {
                initValHandler(init_val, const_exp, mem_raw, demension, arr_size * sizeof(int));
            }
            if (is_global) {
                // to a global variable, we can release all the tokens of it's init_val
                def->c_ = nullptr;
            }
        }
        else {
            // example: int a, b;
            // def: a
            // ident: a
            // init_val: nullptr
            // const_exp: nullptr

        }
    }
}

int Interpreter::execOneCompUnit(AstNodePtr comp_unit) {
    // here we do a simple dispatch
    if (comp_unit->a_->ebnf_type_ == SyEbnfType::Decl) {
        declHandler(comp_unit->a_, 1);
    }
    else if (comp_unit->a_->ebnf_type_ == SyEbnfType::FuncDef) {
        // TODO: add a function_table_
    }
    else {
        // shouldn't reach here
        // cause an error
        assert(1!=1);
    }
    return 0;
}

int Interpreter::exec() {
    while (1) {
        auto comp_unit = parser_->parse();
        if (parser_->end()) {
            break;
        }
        if (parser_->error()) {
            return -1;
        }
        if (comp_unit == nullptr) {
            return -1;
        }
        execOneCompUnit(comp_unit);
    }
    return 0;
}