#include "sysemantic.h"

void SemanticAnalysisVisitor::visitCompUnit(CompUnitAstNode &node) {
    node.getChild()->accept(*this);
}

void SemanticAnalysisVisitor::visitDecl(DeclAstNode &node) {
    // to the Decl, we need add it to the symbol table
    //
}

void SemanticAnalysisVisitor::visitAddExp(AddExpAstNode &node) {}

void SemanticAnalysisVisitor::visitBlock(BlockAstNode &node) {}

void SemanticAnalysisVisitor::visitBType(BTypeAstNode &node) {}

void SemanticAnalysisVisitor::visitBlockItem(BlockItemAstNode &node) {}

void SemanticAnalysisVisitor::visitCond(CondAstNode &node) {}

void SemanticAnalysisVisitor::visitConstDecl(ConstDeclAstNode &node) {}

void SemanticAnalysisVisitor::visitConstDef(ConstDefAstNode &node) {}

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

void SemanticAnalysisVisitor::visitVarDecl(VarDeclAstNode &node) {}

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