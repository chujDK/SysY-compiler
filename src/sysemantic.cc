#include "sysemantic.h"

#include <cstdint>
#include <memory>
#include <vector>

#include "syparse.h"
#include "utils.h"

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

static SyAstType valTypeToValArrayType(SyAstType type) {
    switch (type) {
        case SyAstType::VAL_TYPE_INT:
            return SyAstType::VAL_TYPE_INT_ARRAY;
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

    // B. test if the ident is an array. instead of using the type, we directly
    // use the ConstExp chain
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

        // B.d check the init val
        auto init_val = def->getChildAt(2);
        if (init_val != nullptr) {
            // here we need give some context to the init val check, which means
            // the `array_mem_'
            array_mem_ = array_mem;
            init_val->accept(*this);
            // after checking, remember to clear the context
            array_mem_ = nullptr;
        }
    } else {
        // this is the non-array case
        // B.a add to the symbol table
        ident_mem_ = symbol_table_->addSymbol(ident_name, type, 0, is_const);
        // B.d check the init val
        auto init_val = def->getChildAt(2);
        if (init_val != nullptr) {
            // here we need give some context to the init val check, which means
            // the `ident_mem_'
            init_val->accept(*this);
            // after checking, remember to clear the context
            ident_mem_ = nullptr;
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

void SemanticAnalysisVisitor::visitConstDef(ConstDefAstNode &node) {}

void SemanticAnalysisVisitor::visitAddExp(AddExpAstNode &node) {}

void SemanticAnalysisVisitor::visitBlock(BlockAstNode &node) {}

void SemanticAnalysisVisitor::visitBType(BTypeAstNode &node) {
    type_ = bTypeToValType(node.type()->getAstType());
}

void SemanticAnalysisVisitor::visitBlockItem(BlockItemAstNode &node) {}

void SemanticAnalysisVisitor::visitCond(CondAstNode &node) {}

void SemanticAnalysisVisitor::visitVarDef(VarDefAstNode &node) {}

void SemanticAnalysisVisitor::visitUnaryOp(UnaryOpAstNode &node) {}

void SemanticAnalysisVisitor::visitRelExp(RelExpAstNode &node) {}

void SemanticAnalysisVisitor::visitMulExp(MulExpAstNode &node) {}

void SemanticAnalysisVisitor::visitConstInitVal(ConstInitValAstNode &node) {}

void SemanticAnalysisVisitor::visitConstExp(ConstExpAstNode &node) {}

void SemanticAnalysisVisitor::visitFuncType(FuncTypeAstNode &node) {}

void SemanticAnalysisVisitor::visitFuncDef(FuncDefAstNode &node) {}

void SemanticAnalysisVisitor::visitStmt(StmtAstNode &node) {}

void SemanticAnalysisVisitor::visitPrimaryExp(PrimaryExpAstNode &node) {}

void SemanticAnalysisVisitor::visitLAndExp(LAndExpAstNode &node) {}

void SemanticAnalysisVisitor::visitToken(TokenAstNode &node) {}

void SemanticAnalysisVisitor::visitLOrExp(LOrExpAstNode &node) {}

void SemanticAnalysisVisitor::visitInitVal(InitValAstNode &node) {}

void SemanticAnalysisVisitor::visitFuncFParams(FuncFParamsAstNode &node) {}

void SemanticAnalysisVisitor::visitFuncFParam(FuncFParamAstNode &node) {}

void SemanticAnalysisVisitor::visitExp(ExpAstNode &node) {}

void SemanticAnalysisVisitor::visitLVal(LValAstNode &node) {}

void SemanticAnalysisVisitor::visitNumber(NumberAstNode &node) {}

void SemanticAnalysisVisitor::visitUnaryExp(UnaryExpAstNode &node) {}

void SemanticAnalysisVisitor::visitFuncRParams(FuncRParamsAstNode &node) {}

void SemanticAnalysisVisitor::visitEqExp(EqExpAstNode &node) {}

void SemanticAnalysisVisitor::visitE(EAstNode &node) {}