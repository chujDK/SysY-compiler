#ifndef _SYPARSE_H_
#define _SYPARSE_H_
#include <cassert>
#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "sytype.h"
#include "utils.h"

class AstNodeBase;
class TokenAstNode;
using TokenPtr   = std::shared_ptr<TokenAstNode>;
using AstNodePtr = std::shared_ptr<AstNodeBase>;
enum class SyAstType;
enum class SyEbnfType;

class AstNodePool {
   public:
    static TokenPtr get(SyAstType type, int line, std::string&& literal);
    static AstNodePtr get(SyEbnfType type, int line);
};

class InputStream {
   public:
    // this class is made to read the input "file"
    virtual char getChar()        = 0;  // get the current char
    virtual char peakChar()       = 0;  // get the current char
    virtual char peakNextChar()   = 0;  // get the next char
    virtual void ungetChar()      = 0;  // unget the current char
    virtual std::string getLine() = 0;  // get the current line
    virtual ~InputStream() {}
};

class AstNodeBase {
    // TODO: after refactor, change this to protected
   public:
    // in the ast, the meaning is self-explained;
    // if the nodes are in a list, like FuncFParam to FuncFParams,
    // parent_ and d_ link up a double linked list
    AstNodePtr a_, b_, c_, d_;  // TODO: maybe delete this field in the future?
                                // anyway, this field won't used in the visitors

    // only to the EBnfType::TYPE_INT_ARRAY, array_size_ can be used
    // this also means that this complier can only support array size up to
    // 2^32-1 only to the EBnfType::ConstDef, array_size_ can be used
    // it's tempting to move the line_ into this union, but giveup
    // delete this field in the future
    union {
        unsigned int array_size_;  // FIXME: DELETE ME(u_) WHEN WE DEPRECATE THE
                                   // INTERPRETER !
        unsigned int const_val_;
    } u_;
    unsigned int line_;

   public:
    // getter for the children. sometimes we want to treat two kinds of nodes as
    // the same in visiting, in that case, we need a more common getter
    virtual AstNodePtr getChildAt(int index) const;

    virtual ~AstNodeBase() {}
    virtual std::string const& getLiteral() const = 0;
    virtual unsigned int getLine() const { return line_; }

    virtual enum SyAstType getAstType()            = 0;
    virtual enum SyEbnfType getEbnfType()          = 0;
    virtual void setAstType(enum SyAstType type)   = 0;
    virtual void setEbnfType(enum SyEbnfType type) = 0;

    virtual AstNodePtr getAstParent()            = 0;
    virtual void setAstParent(AstNodePtr parent) = 0;

    AstNodeBase(int line) : line_(line) { u_.const_val_ = UINT32_MAX; }

    class AstNodeVisitor;
    virtual void accept(AstNodeVisitor& visitor) = 0;

    class AstNodeIterator;
    virtual AstNodeIterator begin();
    virtual AstNodeIterator end();

    // consider to add a comment setter
    friend class Parser;
};

// a simple iterator.
class AstNodeBase::AstNodeIterator {
   private:
    // note: here we use raw ptr to avoid the samrt pointer problem.
    // moreover, when iterator is used, the node itself can't be deleted.
    // after the iterator is used, the node do nothing to the node (means no
    // effects to the smart pointer's ref count).
    AstNodeBase* node_;

   public:
    bool operator!=(const AstNodeIterator& other) const {
        return node_ != other.node_;
    }

    AstNodeBase* operator*() { return node_; }
    AstNodeBase::AstNodeIterator operator++() {
        node_ = node_->d_.get();
        return *this;
    }
    AstNodeBase* operator->() { return node_; }

    AstNodeIterator(AstNodeBase* node) : node_(node) {}
};

class TokenAstNode : public AstNodeBase {
   private:
    std::string literal_;
    enum SyAstType ast_type_;

   public:
    TokenAstNode(enum SyAstType ast_type, int line, std::string&& literal)
        : AstNodeBase(line), literal_(literal), ast_type_(ast_type) {}

    std::string const& getLiteral() const override { return literal_; }

    enum SyAstType getAstType() override { return ast_type_; }
    enum SyEbnfType getEbnfType() override { return SyEbnfType::END_OF_ENUM; }

    void setAstType(enum SyAstType type) override { ast_type_ = type; }
    void setEbnfType(enum SyEbnfType type) override { DEBUG_ASSERT_NOT_REACH }
    void setAstParent(AstNodePtr parent) override { DEBUG_ASSERT_NOT_REACH }
    virtual AstNodePtr getAstParent() override { return nullptr; }

    void accept(AstNodeVisitor& visitor) override;
};

class AstNode : public AstNodeBase {
   private:
    // TODO: there is no need to use the ebnf_type_ to record the type, we can
    // use the virtual function getEbnfType() to return the correct type.
    // so this field can be removed. however, for simplicity, keep it is also
    // pretty fair.
    SyEbnfType ebnf_type_;

   public:
    AstNode(enum SyEbnfType ebnf_type, int line)
        : AstNodeBase(line), ebnf_type_(ebnf_type) {}

    virtual ~AstNode() {}
    virtual std::string const& getLiteral() const override {
        static std::string null_string = "";
        return null_string;
    }

    enum SyAstType getAstType() override { return SyAstType::END_OF_ENUM; }
    enum SyEbnfType getEbnfType() override { return ebnf_type_; }
    void setAstType(enum SyAstType type) override { DEBUG_ASSERT_NOT_REACH }
    void setEbnfType(enum SyEbnfType type) override { ebnf_type_ = type; }

    virtual AstNodePtr getAstParent() override { return nullptr; }
    virtual void setAstParent(AstNodePtr parent) override {
        DEBUG_ASSERT_NOT_REACH
    }

    virtual void accept(AstNodeVisitor& visitor) override {
        DEBUG_ASSERT_NOT_REACH
    }
};

class AstIterator {
   public:
    virtual ~AstIterator() {}
};

class CompUnitAstNode : public AstNode {
   private:
    class iterator : public AstIterator {};

   public:
    using AstNode::AstNode;

    CompUnitAstNode(int line) : AstNode(SyEbnfType::CompUnit, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr decl() {
        if (a_->getEbnfType() == SyEbnfType::Decl) {
            return a_;
        } else {
            return nullptr;
        }
    }
    AstNodePtr func_def() {
        if (a_->getEbnfType() == SyEbnfType::FuncDef) {
            return a_;
        } else {
            return nullptr;
        }
    }
    AstNodePtr getChild() { return a_; }
};

class DeclAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    DeclAstNode(int line) : AstNode(SyEbnfType::Decl, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr const_decl() {
        if (a_->getEbnfType() == SyEbnfType::ConstDef) {
            return a_;
        } else {
            return nullptr;
        }
    }

    AstNodePtr var_decl() {
        if (a_->getEbnfType() == SyEbnfType::VarDecl) {
            return a_;
        } else {
            return nullptr;
        }
    }

    AstNodePtr const_decl_or_var_decl() { return a_; }
};

class ConstDeclAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    ConstDeclAstNode(int line) : AstNode(SyEbnfType::ConstDecl, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr b_type() { return a_; }
    AstNodePtr const_def() { return b_; }
};

class BTypeAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    BTypeAstNode(int line) : AstNode(SyEbnfType::BType, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr type() { return a_; }
};

class ConstDefAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    ConstDefAstNode(int line) : AstNode(SyEbnfType::ConstDef, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr ident() { return a_; }
    AstNodePtr const_exp() { return b_; }
    AstNodePtr const_init_val() { return c_; }
};

class ConstInitValAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    ConstInitValAstNode(int line) : AstNode(SyEbnfType::ConstInitVal, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodeBase::AstNodeIterator begin() override {
        if (a_->getEbnfType() != SyEbnfType::ConstExp) {
            return AstNodeIterator(this);
        } else {
            return AstNodeIterator(nullptr);
        }
    }

    AstNodePtr const_exp() {
        if (a_->getEbnfType() == SyEbnfType::ConstExp) {
            return a_;
        } else {
            return nullptr;
        }
    }
};

class VarDeclAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    VarDeclAstNode(int line) : AstNode(SyEbnfType::VarDecl, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr b_type() { return a_; }
    AstNodePtr var_def() { return b_; }
};

class VarDefAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    VarDefAstNode(int line) : AstNode(SyEbnfType::VarDef, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr ident() { return a_; }
    // TODO: for the simplicity, maybe add a type of const_exp_list
    AstNodePtr const_exp_list() { return b_; }
    AstNodePtr init_val() { return c_; }
};

class InitValAstNode : public AstNode {
    // this part is very tricky.
   public:
    using AstNode::AstNode;

    InitValAstNode(int line) : AstNode(SyEbnfType::InitVal, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodeBase::AstNodeIterator begin() override {
        if (a_->getEbnfType() != SyEbnfType::Exp) {
            return AstNodeIterator(this);
        } else {
            return AstNodeIterator(nullptr);
        }
    }

    AstNodePtr exp() {
        if (a_->getEbnfType() == SyEbnfType::Exp) {
            return a_;
        } else {
            return nullptr;
        }
    }
};

class FuncDefAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    FuncDefAstNode(int line) : AstNode(SyEbnfType::FuncDef, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr func_type() { return a_; }
    AstNodePtr ident() { return b_; }
    AstNodePtr func_f_params() { return c_; }
    AstNodePtr block() { return d_; }
};

class FuncTypeAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    FuncTypeAstNode(int line) : AstNode(SyEbnfType::FuncType, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr type() { return a_; }
};

class FuncFParamsAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    FuncFParamsAstNode(int line) : AstNode(SyEbnfType::FuncFParams, line) {}
    void accept(AstNodeVisitor& visitor) override;
};

class FuncFParamAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    FuncFParamAstNode(int line) : AstNode(SyEbnfType::FuncFParam, line) {}
    void accept(AstNodeVisitor& visitor) override;
};

class BlockAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    BlockAstNode(int line) : AstNode(SyEbnfType::Block, line) {}
    void accept(AstNodeVisitor& visitor) override;
};

class BlockItemAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    BlockItemAstNode(int line) : AstNode(SyEbnfType::BlockItem, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr decl_or_stmt() {
        DEBUG_ASSERT(a_->getEbnfType() == SyEbnfType::Decl ||
                     a_->getEbnfType() == SyEbnfType::Stmt);
        return a_;
    }
};

class StmtAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    StmtAstNode(int line) : AstNode(SyEbnfType::Stmt, line) {}
    void accept(AstNodeVisitor& visitor) override;
};

class ExpAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    ExpAstNode(int line) : AstNode(SyEbnfType::Exp, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr add_exp() { return a_; }
};

class CondAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    CondAstNode(int line) : AstNode(SyEbnfType::Cond, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr l_or_exp() {
        DEBUG_ASSERT(a_->getEbnfType() == SyEbnfType::LOrExp);
        return a_;
    }
};

class LValAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    LValAstNode(int line) : AstNode(SyEbnfType::LVal, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr ident() {
        DEBUG_ASSERT(a_->getAstType() == SyAstType::IDENT);
        return a_;
    }
    AstNodePtr exp_chain() { return b_; }
};

class PrimaryExpAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    PrimaryExpAstNode(int line) : AstNode(SyEbnfType::PrimaryExp, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr number() {
        if (a_->getEbnfType() == SyEbnfType::Number) {
            return a_;
        } else {
            return nullptr;
        }
    }

    AstNodePtr l_val() {
        if (a_->getEbnfType() == SyEbnfType::LVal) {
            return a_;
        } else {
            return nullptr;
        }
    }

    AstNodePtr exp() {
        if (a_->getEbnfType() == SyEbnfType::Exp) {
            return a_;
        } else {
            return nullptr;
        }
    }
};

class NumberAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    NumberAstNode(int line) : AstNode(SyEbnfType::Number, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr token() { return a_; }
};

class UnaryExpAstNode : public AstNode {
   private:
    std::weak_ptr<AstNodeBase> parent_;

   public:
    using AstNode::AstNode;

    UnaryExpAstNode(int line) : AstNode(SyEbnfType::UnaryExp, line) {}
    void accept(AstNodeVisitor& visitor) override;
    void setAstParent(std::shared_ptr<AstNodeBase> parent) override {
        parent_ = parent;
    }
    AstNodePtr getAstParent() override { return parent_.lock(); }

    AstNodePtr primary_exp() {
        if (a_->getEbnfType() == SyEbnfType::PrimaryExp) {
            return a_;
        } else {
            return nullptr;
        }
    }

    AstNodePtr ident() {
        if (a_->getAstType() == SyAstType::IDENT) {
            return a_;
        } else {
            return nullptr;
        }
    }

    AstNodePtr func_r_params() {
        if (ident() != nullptr) {
            DEBUG_ASSERT(b_ != nullptr &&
                         b_->getEbnfType() == SyEbnfType::FuncRParams);
            return b_;
        } else {
            return nullptr;
        }
    }

    AstNodePtr unary_op() {
        if (a_->getEbnfType() == SyEbnfType::UnaryOp) {
            return a_;
        } else {
            return nullptr;
        }
    }

    AstNodePtr unary_exp() {
        if (b_ != nullptr) {
            DEBUG_ASSERT(unary_op() != nullptr);
            if (b_->getEbnfType() == SyEbnfType::UnaryExp) {
                return b_;
            } else {
                return nullptr;
            }
        } else {
            DEBUG_ASSERT(unary_op() == nullptr);
            return nullptr;
        }
    }
};

class UnaryOpAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    UnaryOpAstNode(int line) : AstNode(SyEbnfType::UnaryOp, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr op() { return a_; }
};

class FuncRParamsAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    FuncRParamsAstNode(int line) : AstNode(SyEbnfType::FuncRParams, line) {}
    void accept(AstNodeVisitor& visitor) override;
};

class ExpBaseAstNode : public AstNode {
   public:
    using AstNode::AstNode;

    ExpBaseAstNode(int line) : AstNode(SyEbnfType::Exp, line) {}
    void accept(AstNodeVisitor& visitor) override;

    AstNodePtr lhs() { return a_; }
    AstNodePtr op() { return b_; }
    AstNodePtr rhs() { return c_; }
};

class MulExpAstNode : public ExpBaseAstNode {
   private:
    std::weak_ptr<AstNodeBase> parent_;

   public:
    using ExpBaseAstNode::ExpBaseAstNode;

    MulExpAstNode(int line) : ExpBaseAstNode(SyEbnfType::MulExp, line) {}
    void accept(AstNodeVisitor& visitor) override;
    void setAstParent(std::shared_ptr<AstNodeBase> parent) override {
        parent_ = parent;
    }
    AstNodePtr getAstParent() override { return parent_.lock(); }
};

class AddExpAstNode : public ExpBaseAstNode {
   private:
    std::weak_ptr<AstNodeBase> parent_;

   public:
    using ExpBaseAstNode::ExpBaseAstNode;

    AddExpAstNode(int line) : ExpBaseAstNode(SyEbnfType::AddExp, line) {}
    void accept(AstNodeVisitor& visitor) override;
    void setAstParent(std::shared_ptr<AstNodeBase> parent) override {
        parent_ = parent;
    }
    AstNodePtr getAstParent() override { return parent_.lock(); }

    AstNodePtr lhs() { return a_; }
    AstNodePtr rhs() { return c_; }
    AstNodePtr op() { return b_; }
};

class RelExpAstNode : public ExpBaseAstNode {
   private:
    std::weak_ptr<AstNodeBase> parent_;

   public:
    using ExpBaseAstNode::ExpBaseAstNode;

    RelExpAstNode(int line) : ExpBaseAstNode(SyEbnfType::RelExp, line) {}
    void accept(AstNodeVisitor& visitor) override;
    void setAstParent(std::shared_ptr<AstNodeBase> parent) override {
        parent_ = parent;
    }
    AstNodePtr getAstParent() override { return parent_.lock(); }
};

class EqExpAstNode : public ExpBaseAstNode {
   private:
    std::weak_ptr<AstNodeBase> parent_;

   public:
    using ExpBaseAstNode::ExpBaseAstNode;

    EqExpAstNode(int line) : ExpBaseAstNode(SyEbnfType::EqExp, line) {}
    void accept(AstNodeVisitor& visitor) override;
    void setAstParent(std::shared_ptr<AstNodeBase> parent) override {
        parent_ = parent;
    }
    AstNodePtr getAstParent() override { return parent_.lock(); }
};

class LAndExpAstNode : public ExpBaseAstNode {
   private:
    std::weak_ptr<AstNodeBase> parent_;

   public:
    using ExpBaseAstNode::ExpBaseAstNode;

    LAndExpAstNode(int line) : ExpBaseAstNode(SyEbnfType::LAndExp, line) {}
    void accept(AstNodeVisitor& visitor) override;
    void setAstParent(std::shared_ptr<AstNodeBase> parent) override {
        parent_ = parent;
    }
    AstNodePtr getAstParent() override { return parent_.lock(); }
};

class LOrExpAstNode : public ExpBaseAstNode {
   private:
    std::weak_ptr<AstNodeBase> parent_;

   public:
    using ExpBaseAstNode::ExpBaseAstNode;

    LOrExpAstNode(int line) : ExpBaseAstNode(SyEbnfType::LOrExp, line) {}
    void accept(AstNodeVisitor& visitor) override;
    void setAstParent(std::shared_ptr<AstNodeBase> parent) override {
        parent_ = parent;
    }
    AstNodePtr getAstParent() override { return parent_.lock(); }
};

class ConstExpAstNode : public AstNode {
   private:
    Value const_val_;
    bool const_val_computed_;

   public:
    ConstExpAstNode(enum SyEbnfType ebnf_type, int line)
        : AstNode(ebnf_type, line) {
        const_val_computed_ = false;
    }

    ConstExpAstNode(int line) : AstNode(SyEbnfType::ConstExp, line) {}
    void accept(AstNodeVisitor& visitor) override;

    // [true: computed; false: haven't computed, const_val_]
    std::tuple<bool, Value> const_val() {
        // if the const_val_ is not computed, then it is equal to max value
        // if the const_val_ itself is equal to max value,  it won't hurt to
        // compute it again
        return std::make_tuple(const_val_computed_, const_val_);
    }

    void setConstVal(Value const_val) {
        DEBUG_ASSERT(const_val_computed_ == 0);
        const_val_computed_ = true;
        const_val_          = const_val;
    }

    AstNodePtr add_exp() { return a_; }
};

class EAstNode : public AstNode {
   private:
    // this class has this field for the left recursive
    std::weak_ptr<AstNodeBase> parent_;

   public:
    using AstNode::AstNode;
    EAstNode(int line) : AstNode(SyEbnfType::E, line) {}
    void accept(AstNodeVisitor& visitor) override;
    void setAstParent(std::shared_ptr<AstNodeBase> parent) override {
        parent_ = parent;
    }
    AstNodePtr getAstParent() override { return parent_.lock(); }
};

class AstNodeBase::AstNodeVisitor {
   public:
#define DEF_VISIT_FUNC(type) \
    virtual void visit##type(type##AstNode& node){DEBUG_ASSERT_NOT_REACH};
    SY_EBNF_TYPE_LIST(DEF_VISIT_FUNC)
#undef DEF_VISIT_FUNC
    virtual ~AstNodeVisitor() {}
};

using TokenPtrIter = typename std::list<TokenPtr>::iterator;
class Lexer {
   private:
    InputStream* input_stream_;
    int line_;
    bool error_occured_;
    std::list<TokenPtr> token_stream_;
    // to the std::list, the iterator won't be invalid, we will take
    // advantage of that
    TokenPtrIter current_token_;

    void lexError(std::string msg);
    void lexWarning(std::string msg);
    std::string getString();
    std::string getNumber();
    TokenPtr getIdent();
    TokenPtr getNextTokenInternal();
    TokenPtrIter getNextToken();
    TokenPtrIter getNextToken(TokenPtrIter token);
    TokenPtrIter getPrevToken(TokenPtrIter token);

   public:
    class iterator {
       private:
        TokenPtrIter iter_;
        Lexer* this_lexer_;

       public:
        iterator() {}
        iterator(TokenPtrIter iter, Lexer* this_lexer)
            : iter_(iter), this_lexer_(this_lexer) {}
        TokenPtr operator*() { return *iter_; }
        TokenPtr operator->() { return *iter_; }
        iterator& operator++() {
            iter_ = this_lexer_->getNextToken(iter_);
            return *this;
        }
        iterator& operator--() {
            iter_ = this_lexer_->getPrevToken(iter_);
            return *this;
        }
        bool operator==(const iterator& rhs) { return iter_ == rhs.iter_; }
        bool operator!=(const iterator& rhs) { return iter_ != rhs.iter_; }
    };
    friend class iterator;

    iterator begin() {
        if (current_token_ == token_stream_.end()) {
            current_token_ = getNextToken();
        }
        iterator iter(current_token_, this);
        return iter;
    }

    Lexer(InputStream* input_stream)
        : input_stream_(input_stream), line_(1), token_stream_() {
        current_token_ = token_stream_.begin();
    }
    ~Lexer() { delete input_stream_; }
};

class ParserAPI {
   public:
    virtual AstNodePtr parse() = 0;
    virtual bool end()         = 0;
    virtual bool error()       = 0;
    virtual ~ParserAPI(){};
};

class Parser : ParserAPI {
   private:
    Lexer::iterator token_iter_;
    Lexer* lexer_;
    bool error_occured_;
    bool end_parse_;

    void parseError(std::string msg, int line);

    // they are protected instead of private is mainly for testing
   protected:
    AstNodePtr BType();         // no error handling
    AstNodePtr CompUnit();      // no error handling
    AstNodePtr Decl();          // no error handling
    AstNodePtr FuncDef();       // error handling: DONE
    AstNodePtr FuncType();      // no error handling
    AstNodePtr ConstDecl();     // error handling: DONE
    AstNodePtr ConstDef();      // error handling: DONE
    AstNodePtr ConstInitVal();  // error handling: DONE
    AstNodePtr VarDecl();       // error handling: DONE
    AstNodePtr VarDef();        // error handling: DONE
    AstNodePtr InitVal();       // error handling: DONE
    AstNodePtr FuncFParams();   // error handling: DONE
    AstNodePtr FuncFParam();    // no error handling
    AstNodePtr Block();         // error handling: DONE
    AstNodePtr BlockItem();     // error handling: DONE
    AstNodePtr Stmt();          // error handling: DONE
    AstNodePtr Exp();           // no error handling
    AstNodePtr Cond();          // no errror handling
    AstNodePtr LVal();          // no error handling TODO: check if it really
                                // needn't error handling
    AstNodePtr PrimaryExp();    // no error handling
    AstNodePtr Number();        // no error handling
    AstNodePtr UnaryExp();      // no error handling
    AstNodePtr UnaryOp();       // no error handling
    AstNodePtr FuncRParams();   // error handling: DONE
    // following function with the L suffix means left recursion elimination
    AstNodePtr MulExp();    // no error handling
    AstNodePtr MulExpL();   // won't error
    AstNodePtr AddExp();    // no error handling
    AstNodePtr AddExpL();   // won't error
    AstNodePtr RelExp();    // no error handling
    AstNodePtr RelExpL();   // won't error
    AstNodePtr EqExp();     // no error handling
    AstNodePtr EqExpL();    // won't error
    AstNodePtr LAndExp();   // no error handling
    AstNodePtr LAndExpL();  // won't error
    AstNodePtr LOrExp();    // no error handling
    AstNodePtr LOrExpL();   // won't error
    AstNodePtr ConstExp();  // no error handling
    AstNodePtr Ident();     // no error handling

    // helper for the *L suffix node elemination
    AstNodePtr copyInvaildAstNodeToValidHelper(AstNodePtr node,
                                               SyEbnfType type);
    void adjustExpLAst(AstNodePtr node);
    AstNodePtr adjustExpAst(AstNodePtr root);
    AstNodePtr adjustExpAstRightBindToLeftBind(AstNodePtr node);

   public:
    AstNodePtr parse();
    bool end() { return end_parse_; }
    bool error() { return error_occured_; }
    Parser(InputStream* InputStream) {
        lexer_         = new Lexer(InputStream);
        token_iter_    = lexer_->begin();
        error_occured_ = false;
        end_parse_     = false;
    }
    ~Parser() { delete lexer_; }
};

#endif
