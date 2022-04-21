#ifndef _SYPARSE_H_
#define _SYPARSE_H_
#include <cassert>
#include <list>
#include <memory>
#include <string>

#include "sytype.h"
#include "utils.h"

struct AstNodeBase;
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

#ifdef DEBUG
#define DEBUG_ASSERT_NOT_REACH assert(false && "should not reach here");
#else
#define DEBUG_ASSERT_NOT_REACH
#endif

class AstNodeVisitor {
   public:
	virtual ~AstNodeVisitor() {}
};

struct AstNodeBase {
	// in the ast, the meaning is self-explained;
	// if the nodes are in a list, like FuncFParam to FuncFParams,
	// parent_ and d_ link up a double linked list
	AstNodePtr a_, b_, c_, d_;  // TODO: maybe delete this field in the future?
	// only to the EBnfType::TYPE_INT_ARRAY, array_size_ can be used
	// this also means that this complier can only support array size up to
	// 2^32-1 only to the EBnfType::ConstDef, array_size_ can be used
	// it's tempting to move the line_ into this union, but giveup
	// delete this field in the future
	union {
		unsigned int array_size_;
		unsigned int const_val_;
	} u_;
	unsigned int line_;

	virtual ~AstNodeBase() {}
	virtual std::string const& getLiteral() = 0;

	virtual enum SyAstType getAstType()            = 0;
	virtual enum SyEbnfType getEbnfType()          = 0;
	virtual void setAstType(enum SyAstType type)   = 0;
	virtual void setEbnfType(enum SyEbnfType type) = 0;

	virtual void Accept(AstNodeVisitor* visitor) = 0;
	// FIXME: to use the irGen virtual method, the compiler's componet should be
	// accessed globally, currently i don't know how to make it right
	virtual void irGen()                         = 0;
	virtual void checkSemantic()                 = 0;
	virtual AstNodePtr getAstParent()            = 0;
	virtual void setAstParent(AstNodePtr parent) = 0;

	AstNodeBase(int line) : line_(line) { u_.const_val_ = UINT32_MAX; }
};

class TokenAstNode : public AstNodeBase {
   private:
	std::string literal_;
	enum SyAstType ast_type_;

   public:
	TokenAstNode(enum SyAstType ast_type, int line, std::string&& literal)
	    : AstNodeBase(line), literal_(literal), ast_type_(ast_type) {}

	std::string const& getLiteral() { return literal_; }

	enum SyAstType getAstType() { return ast_type_; }
	enum SyEbnfType getEbnfType() { return SyEbnfType::END_OF_ENUM; }

	void setAstType(enum SyAstType type) { ast_type_ = type; }
	void setEbnfType(enum SyEbnfType type) { DEBUG_ASSERT_NOT_REACH }
	void setAstParent(AstNodePtr parent) { DEBUG_ASSERT_NOT_REACH }
	virtual void Accept(AstNodeVisitor* visitor) { DEBUG_ASSERT_NOT_REACH }
	virtual void irGen() { DEBUG_ASSERT_NOT_REACH }
	virtual void checkSemantic() { DEBUG_ASSERT_NOT_REACH }
	virtual AstNodePtr getAstParent() { return nullptr; }
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
	virtual std::string const& getLiteral() {
		static std::string null_string = "";
		return null_string;
	}

	enum SyAstType getAstType() { return SyAstType::END_OF_ENUM; }
	enum SyEbnfType getEbnfType() { return ebnf_type_; }
	void setAstType(enum SyAstType type) { DEBUG_ASSERT_NOT_REACH }
	void setEbnfType(enum SyEbnfType type) { ebnf_type_ = type; }

	virtual void Accept(AstNodeVisitor* visitor) { DEBUG_ASSERT_NOT_REACH }
	virtual void irGen() { DEBUG_ASSERT_NOT_REACH }
	virtual void checkSemantic() { DEBUG_ASSERT_NOT_REACH }
	virtual AstNodePtr getAstParent() { return nullptr; }
	virtual void setAstParent(AstNodePtr parent) { DEBUG_ASSERT_NOT_REACH }
};

class AstIterator {
   public:
	virtual ~AstIterator() {}
};

class CompUnitAstNode : public AstNode {
   private:
	class iterator : public AstIterator {};

   public:
	CompUnitAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	CompUnitAstNode(int line) : AstNode(SyEbnfType::CompUnit, line) {}
};

class DeclAstNode : public AstNode {
   public:
	DeclAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	DeclAstNode(int line) : AstNode(SyEbnfType::Decl, line) {}
};

class ConstDeclAstNode : public AstNode {
   public:
	ConstDeclAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	ConstDeclAstNode(int line) : AstNode(SyEbnfType::ConstDecl, line) {}
};

class BTypeAstNode : public AstNode {
   public:
	BTypeAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	BTypeAstNode(int line) : AstNode(SyEbnfType::BType, line) {}
};

class ConstDefAstNode : public AstNode {
   public:
	ConstDefAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	ConstDefAstNode(int line) : AstNode(SyEbnfType::ConstDef, line) {}
};

class ConstInitValAstNode : public AstNode {
   public:
	ConstInitValAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	ConstInitValAstNode(int line) : AstNode(SyEbnfType::ConstInitVal, line) {}
};

class VarDeclAstNode : public AstNode {
   public:
	VarDeclAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	VarDeclAstNode(int line) : AstNode(SyEbnfType::VarDecl, line) {}
};

class VarDefAstNode : public AstNode {
   public:
	VarDefAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	VarDefAstNode(int line) : AstNode(SyEbnfType::VarDef, line) {}
};

class InitValAstNode : public AstNode {
   public:
	InitValAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	InitValAstNode(int line) : AstNode(SyEbnfType::InitVal, line) {}
};

class FuncDefAstNode : public AstNode {
   public:
	FuncDefAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	FuncDefAstNode(int line) : AstNode(SyEbnfType::FuncDef, line) {}
};

class FuncTypeAstNode : public AstNode {
   public:
	FuncTypeAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	FuncTypeAstNode(int line) : AstNode(SyEbnfType::FuncType, line) {}
};

class FuncFParamsAstNode : public AstNode {
   public:
	FuncFParamsAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	FuncFParamsAstNode(int line) : AstNode(SyEbnfType::FuncFParams, line) {}
};

class FuncFParamAstNode : public AstNode {
   public:
	FuncFParamAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	FuncFParamAstNode(int line) : AstNode(SyEbnfType::FuncFParam, line) {}
};

class BlockAstNode : public AstNode {
   public:
	BlockAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	BlockAstNode(int line) : AstNode(SyEbnfType::Block, line) {}
};

class BlockItemAstNode : public AstNode {
   public:
	BlockItemAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	BlockItemAstNode(int line) : AstNode(SyEbnfType::BlockItem, line) {}
};

class StmtAstNode : public AstNode {
   public:
	StmtAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	StmtAstNode(int line) : AstNode(SyEbnfType::Stmt, line) {}
};

class ExpAstNode : public AstNode {
   public:
	ExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	ExpAstNode(int line) : AstNode(SyEbnfType::Exp, line) {}
};

class CondAstNode : public AstNode {
   public:
	CondAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	CondAstNode(int line) : AstNode(SyEbnfType::Cond, line) {}
};

class LValAstNode : public AstNode {
   public:
	LValAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	LValAstNode(int line) : AstNode(SyEbnfType::LVal, line) {}
};

class PrimaryExpAstNode : public AstNode {
   public:
	PrimaryExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	PrimaryExpAstNode(int line) : AstNode(SyEbnfType::PrimaryExp, line) {}
};

class NumberAstNode : public AstNode {
   public:
	NumberAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	NumberAstNode(int line) : AstNode(SyEbnfType::Number, line) {}
};

class UnaryExpAstNode : public AstNode {
   private:
	std::weak_ptr<AstNodeBase> parent_;

   public:
	UnaryExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	UnaryExpAstNode(int line) : AstNode(SyEbnfType::UnaryExp, line) {}
	void setAstParent(std::shared_ptr<AstNodeBase> parent) { parent_ = parent; }
	AstNodePtr getAstParent() { return parent_.lock(); }
};

class UnaryOpAstNode : public AstNode {
   public:
	UnaryOpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	UnaryOpAstNode(int line) : AstNode(SyEbnfType::UnaryOp, line) {}
};

class FuncRParamsAstNode : public AstNode {
   public:
	FuncRParamsAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	FuncRParamsAstNode(int line) : AstNode(SyEbnfType::FuncRParams, line) {}
};

class MulExpAstNode : public AstNode {
   private:
	std::weak_ptr<AstNodeBase> parent_;

   public:
	MulExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	MulExpAstNode(int line) : AstNode(SyEbnfType::MulExp, line) {}
	void setAstParent(std::shared_ptr<AstNodeBase> parent) { parent_ = parent; }
	AstNodePtr getAstParent() { return parent_.lock(); }
};

class AddExpAstNode : public AstNode {
   private:
	std::weak_ptr<AstNodeBase> parent_;

   public:
	AddExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	AddExpAstNode(int line) : AstNode(SyEbnfType::AddExp, line) {}
	void setAstParent(std::shared_ptr<AstNodeBase> parent) { parent_ = parent; }
	AstNodePtr getAstParent() { return parent_.lock(); }
};

class RelExpAstNode : public AstNode {
   private:
	std::weak_ptr<AstNodeBase> parent_;

   public:
	RelExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	RelExpAstNode(int line) : AstNode(SyEbnfType::RelExp, line) {}
	void setAstParent(std::shared_ptr<AstNodeBase> parent) { parent_ = parent; }
	AstNodePtr getAstParent() { return parent_.lock(); }
};

class EqExpAstNode : public AstNode {
   private:
	std::weak_ptr<AstNodeBase> parent_;

   public:
	EqExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	EqExpAstNode(int line) : AstNode(SyEbnfType::EqExp, line) {}
	void setAstParent(std::shared_ptr<AstNodeBase> parent) { parent_ = parent; }
	AstNodePtr getAstParent() { return parent_.lock(); }
};

class LAndExpAstNode : public AstNode {
   private:
	std::weak_ptr<AstNodeBase> parent_;

   public:
	LAndExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	LAndExpAstNode(int line) : AstNode(SyEbnfType::LAndExp, line) {}
	void setAstParent(std::shared_ptr<AstNodeBase> parent) { parent_ = parent; }
	AstNodePtr getAstParent() { return parent_.lock(); }
};

class LOrExpAstNode : public AstNode {
   private:
	std::weak_ptr<AstNodeBase> parent_;

   public:
	LOrExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	LOrExpAstNode(int line) : AstNode(SyEbnfType::LOrExp, line) {}
	void setAstParent(std::shared_ptr<AstNodeBase> parent) { parent_ = parent; }
	AstNodePtr getAstParent() { return parent_.lock(); }
};

class ConstExpAstNode : public AstNode {
   public:
	ConstExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}

	ConstExpAstNode(int line) : AstNode(SyEbnfType::ConstExp, line) {}
};

class EAstNode : public AstNode {
   private:
	// this class has this field for the left recursive
	std::weak_ptr<AstNodeBase> parent_;

   public:
	EAstNode(enum SyEbnfType ebnf_type, int line) : AstNode(ebnf_type, line) {}

	EAstNode(int line) : AstNode(SyEbnfType::E, line) {}
	void setAstParent(std::shared_ptr<AstNodeBase> parent) { parent_ = parent; }
	AstNodePtr getAstParent() { return parent_.lock(); }
};

using TokenPtrIter = typename std::list<TokenPtr>::iterator;
class Lexer {
   private:
	InputStream* input_stream_;
	int line_;
	bool error_occured_;
	std::list<TokenPtr> token_stream_;
	// to the std::list, the iterator won't be invalid, we will take advantage
	// of that
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
	    : line_(1), input_stream_(input_stream), token_stream_() {
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

using LexerIterator = typename Lexer::iterator;

class Parser : ParserAPI {
   private:
	LexerIterator token_iter_;
	Lexer* lexer_;
	bool error_occured_;
	bool end_parse_;

	void parseError(std::string msg, int line);
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
	AstNodePtr LVal();  // no error handling TODO: check if it really needn't
	                    // error handling
	AstNodePtr PrimaryExp();   // no error handling
	AstNodePtr Number();       // no error handling
	AstNodePtr UnaryExp();     // no error handling
	AstNodePtr UnaryOp();      // no error handling
	AstNodePtr FuncRParams();  // error handling: DONE
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
