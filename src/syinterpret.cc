#include "syinterpret.h"

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "syparse.h"

using namespace interpreter;

// TODO:
// we can't yet handle the array passing
// [ ] FuncFParams should added to the symbol table
// [ ] FuncFParams, // FuncFParams -> FuncFParam { ',' FuncFParam }
// [ ] FuncFParam, // FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }]
// [ ] interpreter should handle the function redefinition error handling
// [+] CompUnit, // CompUnit -> [ CompUnit ] ( Decl | FuncDef )
// [+] Decl, // Decl -> ConstDecl | VarDecl
// [+] ConstDecl, // ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
// [+] BType, // BType -> 'int'
// [+] ConstInitVal, // ConstInitVal -> ConstExp | '{' [ ConstInitVal { ','
// ConstInitVal } ] '}'
// [+] VarDecl, // VarDecl -> BType VarDef { ',' VarDef } ';'
// [+] VarDef, // VarDef -> Ident { '[' ConstExp ']' } | Ident { '[' ConstExp
// ']' } '=' InitVal
// [+] InitVal, // InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
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
// [+] LVal, // LVal -> Ident {'[' Exp ']'}
// [+] PrimaryExp, // PrimaryExp -> '(' Exp ')' | LVal | Number
// [+] Number, // Number -> IntConst
// [+] UnaryExp, // UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' |
// UnaryOp UnaryExp
// [+] UnaryOp, // UnaryOp -> '+' | '-' | '!'
// [+] FuncRParams, // FuncRParams -> Exp { ',' Exp }
// [+] MulExp, // MulExp -> UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
// [+] AddExp, // AddExp -> MulExp | AddExp ('+' | '−') MulExp
// [+] RelExp, // RelExp -> AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
// [+] EqExp, // EqExp -> RelExp | EqExp ('==' | '!=') RelExp
// [+] LAndExp, // LAndExp -> EqExp | LAndExp '&&' EqExp
// [+] LOrExp, // LOrExp -> LAndExp | LOrExp '||' LAndExp
// [+] ConstExp, // ConstExp -> AddExp

void Interpreter::interpretWarning(std::string msg, int line) {
    fprintf(stderr,
            "\033[1m\033[35mWarning in executing\033[0m: line "
            "\033[1m%d\033[0m: %s\n",
            line, msg.c_str());
}

void Interpreter::interpretError(std::string msg, int line) {
    fprintf(
        stderr,
        "\033[1m\033[31mError in executing\033[0m: line \033[1m%d\033[0m: %s\n",
        line, msg.c_str());
    error_occured_ = 1;
    exit(-1);
}

Value Interpreter::expDispatcher(AstNodePtr exp) {
    switch (exp->getEbnfType()) {
        case SyEbnfType::ConstExp:
            return constExpHandler(exp);
        case SyEbnfType::Exp:
            return expHandler(exp);
        case SyEbnfType::AddExp:
        case SyEbnfType::MulExp:
        case SyEbnfType::RelExp:
        case SyEbnfType::EqExp:
        case SyEbnfType::LAndExp:
        case SyEbnfType::LOrExp:
            return subExpHandler(exp);
        case SyEbnfType::UnaryExp:
            return unaryExpHandler(exp);
        case SyEbnfType::PrimaryExp:
            return primaryExpHandler(exp);
        case SyEbnfType::LVal:
            return lValRightHandler(exp);
        case SyEbnfType::Number:
            return numberHandler(exp);
        default:
            // should not reach here, trigger a bug
            assert(0);
            return Value();
    }
}

Value Interpreter::numberHandler(AstNodePtr exp) {
    Value ret;
    // FIXME: stoi will throw exception if the string is out of int
    ret.i32 = std::stoi(exp->a_->getLiteral());
    return ret;
}

Value Interpreter::primaryExpHandler(AstNodePtr exp) {
    // PrimaryExp -> '(' Exp ')' | LVal | Number
    return expDispatcher(exp->a_);
}

Value Interpreter::unaryExpHandler(AstNodePtr exp) {
    // UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
    // currently only retunr the val of PrimaryExp
    if (exp->a_->getEbnfType() == SyEbnfType::PrimaryExp) {
        return primaryExpHandler(exp);
    }
    if (exp->a_->getAstType() == SyAstType::IDENT) {
        auto function = func_table_->getFunc(exp->a_->getLiteral());
        auto args     = exp->b_;
        if (function) {
            return function->exec(args, this);
        } else {
            interpretError(
                std::string("function") + std::string(" \"\033[1;31m") +
                    exp->a_->getLiteral() + std::string("\033[0m\" not found"),
                exp->a_->line_);
        }
    }
    if (exp->a_->getEbnfType() == SyEbnfType::UnaryOp) {
        Value ret = expDispatcher(exp->b_);
        switch (exp->a_->a_->getAstType()) {
            case SyAstType::ALU_ADD:
                return ret;
            case SyAstType::ALU_SUB:
                ret.i32 = -ret.i32;
                return ret;
            case SyAstType::LOGIC_NOT:
                ret.i32 = !(ret.i32);
                return ret;
            default:
                // shouldn't reach here
                assert(0);
                break;
        }
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
Value Interpreter::subExpHandler(AstNodePtr exp) {
    Value ret;
    auto exp_a = exp->a_;
    if (exp->b_ == nullptr) {
#ifdef DEBUG
        assert(exp->c_ == nullptr);
#endif
        return expDispatcher(exp_a);
    }
    auto exp_c  = exp->c_;
    Value val_a = expDispatcher(exp_a);
    Value val_c = expDispatcher(exp_c);
    switch (exp->b_->getAstType()) {
        case SyAstType::ALU_ADD:
            ret.i32 = val_a.i32 + val_c.i32;
            break;
        case SyAstType::ALU_SUB:
            ret.i32 = val_a.i32 - val_c.i32;
            break;
        case SyAstType::ALU_DIV:
            ret.i32 = val_a.i32 / val_c.i32;
            break;
        case SyAstType::ALU_MUL:
            ret.i32 = val_a.i32 * val_c.i32;
            break;
        case SyAstType::ALU_MOD:
            ret.i32 = val_a.i32 % val_c.i32;
            break;
        case SyAstType::EQ:
            ret.i32 = val_a.i32 == val_c.i32;
            break;
        case SyAstType::NEQ:
            ret.i32 = val_a.i32 != val_c.i32;
            break;
        case SyAstType::LNE:
            ret.i32 = val_a.i32 < val_c.i32;
            break;
        case SyAstType::LE:
            ret.i32 = val_a.i32 <= val_c.i32;
            break;
        case SyAstType::GNE:
            ret.i32 = val_a.i32 > val_c.i32;
            break;
        case SyAstType::GE:
            ret.i32 = val_a.i32 >= val_c.i32;
            break;
        case SyAstType::LOGIC_AND:
            ret.i32 = val_a.i32 && val_c.i32;
            break;
        case SyAstType::LOGIC_OR:
            ret.i32 = val_a.i32 || val_c.i32;
            break;
        default:
            // shouldn't reach here trigger a bug
            assert(1 != 1);
            break;
    }
    return ret;
}

inline Value Interpreter::constExpHandler(AstNodePtr const_exp) {
    if (const_exp->u_.const_val_ == 0xFFFFFFFF) {
        const_exp->u_.const_val_ = expDispatcher(const_exp->a_).i32;
    }
    Value val;
    val.i32 = const_exp->u_.const_val_;
    return val;
}

inline Value Interpreter::expHandler(AstNodePtr exp) {
    return expDispatcher(exp->a_);
}

// need test
// this is currently quiet plain, we can use template to make it more
// generic to handle all types
// just support exact init_val,
// this is not support
// `int arr1[0x10][0x20][0x30] = {{{0x10, 0x11, 0x12}}, {{0x20, 0x21, 0x22}},
// {{0x30, 0x31, 0x32}}}`
void Interpreter::initValHandler(AstNodePtr init_val, AstNodePtr const_exp,
                                 char* mem_raw, int dimension, int size_delta) {
    // we have pretty much confidence that all the const_exp is already computed
    assert(mem_raw != nullptr);

    if (dimension == 0) {
#ifdef DEBUG
        if (init_val->a_->getEbnfType() != SyEbnfType::ConstExp &&
            init_val->a_->getEbnfType() != SyEbnfType::Exp) {
            assert(init_val->a_->getEbnfType() == SyEbnfType::ConstExp ||
                   init_val->a_->getEbnfType() == SyEbnfType::Exp);
        }
#endif
        *((int*)mem_raw) = constExpHandler(init_val->a_).i32;
        return;
    }
    size_delta                 = size_delta / const_exp->u_.const_val_;
    auto init_val_list_current = init_val->a_;
    for (int i = 0; i < const_exp->u_.const_val_; i++) {
        initValHandler(init_val_list_current->a_, const_exp->d_, mem_raw,
                       dimension - 1, size_delta);
        init_val_list_current = init_val_list_current->d_;
        if (init_val_list_current == nullptr) {
            return;
        }
        mem_raw += size_delta;
    }
}

AstNodePtr Interpreter::initValValidater(AstNodePtr init_val,
                                         AstNodePtr const_exp, int dimension) {
    // make the init_list valid
    if (dimension == 0) {
        if (init_val->a_->getEbnfType() == SyEbnfType::InitVal) {
            interpretError("excess elements in scalar initializer",
                           init_val->a_->line_);
            return nullptr;
        }
        return init_val;
    }
    int init_val_dimension = 0;
    auto init_val_walker   = init_val;
    while (init_val_walker->a_->getEbnfType() != SyEbnfType::ConstExp &&
           init_val_walker->a_->getEbnfType() != SyEbnfType::Exp) {
        init_val_walker = init_val_walker->a_;
        init_val_dimension++;
    }
    for (int i = init_val_dimension; i < 2 * dimension; i++) {
        auto fake_parent = std::make_shared<AstNode>(SyEbnfType::InitVal, 0);
        fake_parent->a_  = init_val;
        init_val         = fake_parent;
    }
    auto init_val_list_current = init_val->a_;
    for (int i = 0; i < const_exp->u_.const_val_; i++) {
        init_val_list_current->a_ = initValValidater(
            init_val_list_current->a_, const_exp->d_, dimension - 1);
        init_val_list_current = init_val_list_current->d_;
        if (init_val_list_current == nullptr) {
            return init_val;
        }
    }
    return init_val;
}

// TODO: 根据 symbol table 的实现方式，这里无法检出 init_val 里面是否
// example: int a = a + 1;
void Interpreter::declHandler(AstNodePtr decl, bool is_global) {
    // Decl -> ConstDecl | VarDecl
    // ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
    // VarDecl   ->         BType VarDef   { ',' VarDef   } ';'
    // VarDef   -> Ident { '[' ConstExp ']' }
    //          -> Ident { '[' ConstExp ']' } '=' InitVal
    // ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
    // ConstExp -> AddExp

    auto is_const = decl->a_->getEbnfType() == SyEbnfType::ConstDecl;
    for (auto def = decl->a_->b_; def != nullptr; def = def->d_) {
        auto ident = def->a_;
        if (def->b_ != nullptr) {
            // array
            int dimension = 0;
            int length;
            std::vector<unsigned int> dim_vec;
            uint64_t arr_size =
                1;  // arr_size is the number of elements in the array
            for (auto const_exp = def->b_; const_exp != nullptr;
                 const_exp      = const_exp->d_) {
                // example: int a[10][0x10] = {{0}, {0}}, b;
                // def: a[10][0x10] = {{0}, {0}}
                // ident: a
                // init_val: {{0}, {0}}
                // const_exp: 10, 0x10

                // b won't enter this loop
                if (const_exp->u_.const_val_ == 0xFFFFFFFF) {
                    // it's ok if the val of const_exp is really 0xFFFFFFFF
                    // we just recompute it
                    length                   = constExpHandler(const_exp).i32;
                    const_exp->u_.const_val_ = length;
                }
                arr_size *= length;
                if (arr_size > UINT32_MAX) {
                    interpretError("array size overflow", const_exp->line_);
                    return;
                }
                dim_vec.push_back(length);
                dimension++;
            }
            // example: int a[10][0x10] = {{0}, {0}}, b;
            // arr_size: 10 * 0x10
            // add this ident to symbol_table_
            ident->setAstType(SyAstType::VAL_TYPE_INT_ARRAY);
            ident->u_.array_size_ = arr_size;
            auto mem =
                (is_global ? symbol_table_->addGlobalSymbol(ident, is_const)
                           : symbol_table_->addSymbol(ident, is_const));
            ArrayMemoryPtr array_mem =
                std::static_pointer_cast<ArrayMemoryAPI>(mem);
            array_mem->setDimension(dimension);
            for (int i = 0; i < dimension; i++) {
                array_mem->setSizeForDimension(i, dim_vec[i]);
            }
            auto mem_raw = mem->getMem();
            if (is_global) {
                memset(mem_raw, 0, arr_size * sizeof(int));
            }

            // init_val: {{0}, {0}}
            // recursively add the init_val
            auto init_val  = def->c_;
            auto const_exp = def->b_;
            if (init_val != nullptr) {
                if (init_val->a_ == nullptr) {
                    // TODO: check here!
                    // the init_val is "{}"
                    // just set it all to 0
                    memset(mem_raw, 0, arr_size * sizeof(int));
                } else if (init_val->a_->getEbnfType() !=
                           init_val->getEbnfType()) {
                    // init_val is not a list
                    // throw an warning
                    interpretWarning(
                        "init_val for array is not a list, skipping the init",
                        init_val->line_);
                } else {
                    if (!is_global) {
                        // to a partail init, always init to 0
                        // global is already init to 0
                        memset(mem_raw, 0, arr_size * sizeof(int));
                    }
                    init_val = initValValidater(init_val, const_exp, dimension);
                    initValHandler(init_val, const_exp, mem_raw, dimension,
                                   arr_size * sizeof(int));
                }
            }
            if (is_global) {
                // to a global variable, we can release all the tokens of it's
                // init_val
                def->c_ = nullptr;
            }
        } else {
            // example: int a, b;
            // def: a
            // ident: a
            // init_val: nullptr
            // const_exp: nullptr
            ident->setAstType(SyAstType::VAL_TYPE_INT);
            auto mem =
                (is_global ? symbol_table_->addGlobalSymbol(ident, is_const)
                           : symbol_table_->addSymbol(ident, is_const));
            auto mem_raw  = mem->getMem();
            auto init_val = def->c_;
            if (is_global) {
                memset(mem_raw, 0, sizeof(int));
            }
            if (init_val != nullptr) {
                if (init_val->a_->getEbnfType() != SyEbnfType::ConstExp &&
                    init_val->a_->getEbnfType() != SyEbnfType::Exp) {
                    interpretWarning(
                        "init_val for non-array is a list, skipping the init",
                        init_val->line_);
                } else {
                    *((int*)mem_raw) = expDispatcher(init_val->a_->a_).i32;
                }
            }
        }
    }
}

SYFunctionPtr FunctionTable::addFunc(AstNodePtr func_ast) {
    auto sy_function = std::make_shared<SYFunction>(func_ast);
    func_table_[func_ast->b_->getLiteral()] = sy_function;
    return sy_function;
}

SYFunctionPtr FunctionTable::addFunc(SYFunctionPtr function, std::string name) {
    func_table_[name] = function;
    return function;
}

SYFunctionPtr FunctionTable::getFunc(std::string func_name) {
    return func_table_[func_name];
}

AstNodePtr SYFunction::getFuncAst() {
    called_times_++;
    return func_;
}

SYFunction::SYFunction(AstNodePtr func)
    : func_exec_mem_(nullptr), jited_(false), called_times_(0) {
    func_ = func;
    addToLLVMSymbolTable();
}

SYFunction::SYFunction(void* func, bool no_fail,
                       Value (*exec_call_back)(char*, AstNodePtr,
                                               InterpreterAPI*))
    : func_(nullptr),
      func_exec_mem_(reinterpret_cast<char*>(func)),
      exec_call_back_(exec_call_back),
      jited_(true),
      no_fail_(no_fail),
      called_times_(0){};

Value SYFunction::exec(AstNodePtr args, InterpreterAPI* interpreter) {
    if (jited_) {
        // call the jited function
        return exec_call_back_(func_exec_mem_, args, interpreter);
    } else {
        return interpreter->execFunction(func_, args);
    }
    return Value();
}

void Interpreter::addFunction(SYFunctionPtr function, std::string name) {
    func_table_->addFunc(function, name);
}

int Interpreter::execOneCompUnit(AstNodePtr comp_unit) {
    // here we do a simple dispatch
    if (comp_unit->a_->getEbnfType() == SyEbnfType::Decl) {
        declHandler(comp_unit->a_, 1);
    } else if (comp_unit->a_->getEbnfType() == SyEbnfType::FuncDef) {
        func_table_->addFunc(comp_unit->a_);
        if (comp_unit->a_->b_->getLiteral() == "main") {
// main function
// call it
#ifdef COMPILER
            auto function = func_table_->getFunc("main");
            function->compile();
            exit(0);
#endif
            symbol_table_->enterScope();
            auto val = execFunction(comp_unit->a_, nullptr);
            symbol_table_->exitScope();
            exit(val.i32);
        }
    } else {
        // shouldn't reach here
        // cause an error
        assert(1 != 1);
    }
    return 0;
}

std::tuple<StmtState, Value> Interpreter::blockHandler(AstNodePtr block) {
    auto block_item = block->a_;
    for (; block_item != nullptr; block_item = block_item->d_) {
        if (block_item->a_->getEbnfType() == SyEbnfType::Decl) {
            auto decl = block_item->a_;
            declHandler(decl, 0);
        }
        if (block_item->a_->getEbnfType() == SyEbnfType::Stmt) {
            auto stmt           = block_item->a_;
            auto [state, value] = stmtHandler(stmt);
            if (state == StmtState::RETURN || state == StmtState::BREAK) {
                return std::make_tuple(state, value);
            }
            if (state == StmtState::CONTINUE) {
                return std::make_tuple(state, value);
            }
        }
    }
    return std::make_tuple(StmtState::END_OF_ENUM, Value());
}

void Interpreter::variableIdentTyper(AstNodePtr ident, SyEbnfType type_enum) {
    ident->setEbnfType(type_enum);
}
void Interpreter::variableIdentTyper(AstNodePtr ident, AstNodePtr type) {
    switch (type->getAstType()) {
        case SyAstType::TYPE_INT:
            ident->setAstType(SyAstType::VAL_TYPE_INT);
            break;
        default:
            // shouldn't reach here
            // cause an error
            assert(0);
            break;
    }
}

Value Interpreter::execFunction(AstNodePtr func_ast, AstNodePtr args) {
    // FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
    // first add the args to the symbol table
    symbol_table_->enterScope();
    AstNodePtr arg_exp = nullptr;
    if (args != nullptr) {
        arg_exp = args->a_;
    }
    for (auto func_f_param = func_ast->c_; func_f_param != nullptr;
         func_f_param      = func_f_param->d_) {
        if (arg_exp == nullptr) {
            // throw an error
            interpretError("too few arguments when calling function \"\033[1m" +
                               func_ast->b_->getLiteral() +
                               std::string("\033[0m\""),
                           args->line_);
            return Value();
        }
        auto type  = func_f_param->a_->a_;
        auto ident = func_f_param->b_;
        variableIdentTyper(ident, type);
        auto exps = func_f_param->a_->c_;

        // type checking is very lightly now
        if (exps == nullptr) {
            // non array
            auto mem     = symbol_table_->addSymbol(ident, false);
            auto mem_raw = mem->getMem();
            if (ident->getAstType() == SyAstType::VAL_TYPE_INT) {
                int init_val = expDispatcher(arg_exp->a_).i32;
                memcpy(mem_raw, &init_val, sizeof(int));
            } else {
                // shouldn't reach here
                assert(1 != 1);
            }
        } else {
            // TODO: currently not support multi-dimentional array
        }
        arg_exp = arg_exp->d_;
    }

    auto [state, value] = blockHandler(func_ast->d_);
    if (state == StmtState::BREAK) {
        interpretWarning(
            std::string("\033[1;31mbreak signal\033[0m unhandled until "
                        "function \"\033[1m") +
                func_ast->b_->getLiteral() +
                std::string(
                    "\033[0m\" is end. It is ignored.\n\033[1mhint\033[0m: "
                    "break statement not within loop"),
            func_ast->line_);
    }
    if (state == StmtState::CONTINUE) {
        interpretWarning(
            std::string("\033[1;31mcontiue signal\033[0m unhandled until "
                        "function \"\033[1m") +
                func_ast->b_->getLiteral() +
                std::string(
                    "\033[0m\" is end. It is ignored.\n\033[1mhint\033[0m: "
                    "continue statement not within loop"),
            func_ast->line_);
    }
    symbol_table_->exitScope();
    return value;
}

Value Interpreter::lValRightHandler(AstNodePtr exp) {
    auto [mem, type] = lValLeftHandler(exp);
    Value ret;
    switch (type) {
        case SyAstType::VAL_TYPE_CONST_INT_DEPRECATE:
        case SyAstType::VAL_TYPE_INT:
            ret.i32 = *(reinterpret_cast<int*>(mem));
            break;
        default:
            break;
    }
    return ret;
}

std::tuple<char*, SyAstType> Interpreter::lValLeftHandler(AstNodePtr l_val) {
    // LVal -> Ident {'[' Exp ']'}
    auto mem = symbol_table_->searchTable(l_val->a_);
    if (mem == nullptr) {
        interpretError(std::string("undefined variable \"\033[1;31m") +
                           l_val->a_->getLiteral() + std::string("\033[0m\""),
                       l_val->a_->line_);
    }
    if (l_val->b_ == nullptr) {
        // this is a non-array ident
        if (mem->isConst()) {
            return std::make_tuple(mem->getMem(),
                                   SyAstType::VAL_TYPE_CONST_INT_DEPRECATE);
        } else {
            return std::make_tuple(mem->getMem(), SyAstType::VAL_TYPE_INT);
        }
    } else {
        ArrayMemoryPtr array = std::static_pointer_cast<ArrayMemoryAPI>(mem);
        // auto array_size = array->getArraySize(); FIXME: where to use?
        auto exp             = l_val->b_;
        auto array_dimension = array->getDimension();
        auto mem_raw         = array->getMem();
        uint32_t mem_delta   = array->getSize();
        for (int i = 0; i < array_dimension; i++) {
            int32_t index                = expDispatcher(exp).i32;
            uint32_t size_this_demension = array->getSizeForDimension(i);
            mem_delta /= size_this_demension;
            if (index < 0 || index >= size_this_demension) {
                // throw an error
                interpretError("array index out of bound", exp->line_);
            } else {
                // get the next array's mem start address
                mem_raw += index * mem_delta;
            }
            if (exp == nullptr) {
                // throw an error
                interpretError("pointer can't be a left value", exp->line_);
            }
            exp = exp->d_;
        }
        if (mem->isConst()) {
            return std::make_tuple(mem_raw,
                                   SyAstType::VAL_TYPE_CONST_INT_DEPRECATE);
        } else {
            return std::make_tuple(mem_raw, SyAstType::VAL_TYPE_INT);
        }
    }
    return std::make_tuple(nullptr, SyAstType::END_OF_ENUM);
}

std::tuple<StmtState, Value> Interpreter::stmtHandler(AstNodePtr stmt) {
    // Stmt -> LVal '=' Exp ';' | [Exp] ';' | Block
    //| 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
    //| 'while' '(' Cond ')' Stmt
    //| 'break' ';' | 'continue' ';'
    //| 'return' [Exp] ';'
    bool cond;
    std::tuple<StmtState, Value> ret(StmtState::END_OF_ENUM, Value());
    std::tuple<StmtState, Value> callee_ret(StmtState::END_OF_ENUM, Value());
    if (stmt->a_ == nullptr) {
        // null statement
        return ret;
    }
    switch (stmt->a_->getAstType()) {
        case SyAstType::STM_IF:
            // if (cond) stmt
            cond = expDispatcher(stmt->b_->a_).i32;
            symbol_table_->enterScope();
            if (cond) {
                ret = stmtHandler(stmt->c_);
            } else {
                if (stmt->d_ != nullptr) {
                    ret = stmtHandler(stmt->d_);
                }
            }
            symbol_table_->exitScope();
            return ret;
        case SyAstType::STM_WHILE:
            cond = expDispatcher(stmt->b_->a_).i32;
            symbol_table_->enterScope();
            while (cond) {
                auto [state, val] = stmtHandler(stmt->c_);
                if (state == StmtState::BREAK) {
                    return ret;
                }
                if (state == StmtState::RETURN) {
                    return callee_ret;
                }
                if (state == StmtState::CONTINUE) {
                    // no problem, we go on!
                    // do nothing
                }
                cond = expDispatcher(stmt->b_->a_).i32;
            }
            symbol_table_->exitScope();
            return ret;
        case SyAstType::STM_BREAK:
            std::get<0>(ret) = StmtState::BREAK;
            return ret;
        case SyAstType::STM_CONTINUE:
            std::get<0>(ret) = StmtState::CONTINUE;
            return ret;
        case SyAstType::STM_RETURN:
            // TODO: check if the return val type is correct
            if (stmt->b_ != nullptr) {
                std::get<1>(ret) = expDispatcher(stmt->b_);
            }
            std::get<0>(ret) = StmtState::RETURN;
            return ret;
        default:
            break;
    }
    // LVal '=' Exp ';' | [Exp] ';' | Block
    if (stmt->a_->getEbnfType() == SyEbnfType::LVal) {
        // LVal '=' Exp ';'
        auto l_val       = stmt->a_;
        auto [mem, type] = lValLeftHandler(l_val);
        auto exp         = stmt->b_;
        auto exp_val     = expDispatcher(exp);
        if (type == SyAstType::VAL_TYPE_INT) {
            *(reinterpret_cast<int*>(mem)) = exp_val.i32;
        } else if (type == SyAstType::VAL_TYPE_CONST_INT_DEPRECATE) {
            interpretError("can't assign to a const variable", exp->line_);
        }
    } else if (stmt->a_->getEbnfType() == SyEbnfType::Exp) {
        // Exp ';'
        expDispatcher(stmt->a_);
    } else {
#ifdef DEBUG
        assert(stmt->a_->getEbnfType() == SyEbnfType::Block);
#endif
        callee_ret = blockHandler(stmt->a_);
        return callee_ret;
    }
    return callee_ret;
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

// FIXME:
// alright, the init val handle is really tricky, i just skip it right now

// helper function to transform a InitValAstNode to a Value return a std::tuple,
// the first is wheather it can transform to a Value, true for yes, false for
// no, second for the Value it self
static std::tuple<bool, Value> toNumber(AstNodePtr init_val,
                                        Interpreter* interpreter) {
    if (init_val->getEbnfType() == SyEbnfType::Exp) {
        auto exp_val = interpreter->expDispatcher(init_val);
        return std::make_tuple(true, exp_val);
    } else {
        return std::make_tuple(false, Value());
    }
}

void Interpreter::setInitValMemoryLayout(
    int dimension, std::vector<unsigned int> size_for_dimension, Value* mem_raw,
    AstNodePtr init_val) {
    AstNodePtr current_init_val = init_val->a_;
    for (int i = 0; i < size_for_dimension[dimension]; i++) {
        if (current_init_val == nullptr) {
            return;
        }
        auto [state, value] = toNumber(current_init_val, this);
        if (state == false) {
            // this means current_init_val is also a list, so we jump to the
            // next dimension
            // TODO
        } else {
            mem_raw[i] = value;
        }
    }
}
