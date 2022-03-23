#include <cassert>
#include <cstdint>
#include "syinterpret.h"

void Interpreter::interpretWarning(std::string msg, int line) {
    fprintf(stderr, "\033[1m\033[35mWarning in executing\033[0m: line \033[1m%d\033[0m: %s\n", line, msg.c_str());
}

void Interpreter::interpretError(std::string msg, int line) {
    fprintf(stderr, "\033[1m\033[31mError in executing\033[0m: line \033[1m%d\033[0m: %s\n", line, msg.c_str());
    error_occured_ = 1;
}

Value Interpreter::addExpHandler(AstNodePtr exp) {
    Value ret;
    ret.i32 = 0x10;
    return ret;
}

inline Value Interpreter::constExpHandler(AstNodePtr const_exp) {
    return addExpHandler(const_exp->a_);
}

inline Value Interpreter::expHandler(AstNodePtr exp) {
    return addExpHandler(exp->a_);
}

void Interpreter::initValHandler(AstNodePtr init_val, AstNodePtr const_exp, 
char* mem_raw, int demention, int size_delta) {
    // we have pretty much confidence that all the const_exp is already computed
    assert(mem_raw != nullptr);

    if (demention == 0) {
        *((int*)mem_raw) = constExpHandler(init_val->a_).i32;
        return;
    }
    int val = const_exp->u_.const_val_;
    size_delta = size_delta / const_exp->u_.const_val_;
    auto init_val_list_current = init_val->a_;
    for (int i = 0; i < const_exp->u_.const_val_; i++) {
        initValHandler(init_val_list_current->a_, const_exp->d_, mem_raw, demention - 1, size_delta);
        init_val_list_current = init_val_list_current->d_;
        if (init_val == nullptr) {
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
            ident->u_.array_size_ = length;
            auto mem = (is_global ? symbol_table_->addGlobalSymbol(ident) : symbol_table_->addSymbol(ident));
            auto mem_raw = mem->getMem();

            // init_val: {{0}, {0}}
            // recursively add the init_val
            auto init_val = def->c_;
            auto const_exp = def->b_;
            if (init_val != nullptr) {
                initValHandler(init_val, const_exp, mem_raw, demension, arr_size);
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
    auto comp_unit = parser_->parse();
    if (comp_unit == nullptr) {
        return -1;
    }
    execOneCompUnit(comp_unit);
    return 0;
}