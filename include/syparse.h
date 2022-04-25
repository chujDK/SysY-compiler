#ifndef _SYPARSE_H_
#define _SYPARSE_H_
#include <cassert>
#include <list>
#include <memory>
#include <string>
#include <vector>

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
	using AstNode::AstNode;

	CompUnitAstNode(int line) : AstNode(SyEbnfType::CompUnit, line) {}
};

class DeclAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	DeclAstNode(int line) : AstNode(SyEbnfType::Decl, line) {}
};

class ConstDeclAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	ConstDeclAstNode(int line) : AstNode(SyEbnfType::ConstDecl, line) {}
};

class BTypeAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	BTypeAstNode(int line) : AstNode(SyEbnfType::BType, line) {}
};

class ConstDefAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	ConstDefAstNode(int line) : AstNode(SyEbnfType::ConstDef, line) {}
};

class ConstInitValAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	ConstInitValAstNode(int line) : AstNode(SyEbnfType::ConstInitVal, line) {}
};

class VarDeclAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	VarDeclAstNode(int line) : AstNode(SyEbnfType::VarDecl, line) {}
};

class VarDefAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	VarDefAstNode(int line) : AstNode(SyEbnfType::VarDef, line) {}
};

class InitValAstNode : public AstNode {
	// this part is very tricky.
   public:
	using AstNode::AstNode;

	InitValAstNode(int line) : AstNode(SyEbnfType::InitVal, line) {}
};

class FuncDefAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	FuncDefAstNode(int line) : AstNode(SyEbnfType::FuncDef, line) {}
};

class FuncTypeAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	FuncTypeAstNode(int line) : AstNode(SyEbnfType::FuncType, line) {}
};

class FuncFParamsAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	FuncFParamsAstNode(int line) : AstNode(SyEbnfType::FuncFParams, line) {}
};

class FuncFParamAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	FuncFParamAstNode(int line) : AstNode(SyEbnfType::FuncFParam, line) {}
};

class BlockAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	BlockAstNode(int line) : AstNode(SyEbnfType::Block, line) {}
};

class BlockItemAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	BlockItemAstNode(int line) : AstNode(SyEbnfType::BlockItem, line) {}
};

class StmtAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	StmtAstNode(int line) : AstNode(SyEbnfType::Stmt, line) {}
};

class ExpAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	ExpAstNode(int line) : AstNode(SyEbnfType::Exp, line) {}
};

class CondAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	CondAstNode(int line) : AstNode(SyEbnfType::Cond, line) {}
};

class LValAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	LValAstNode(int line) : AstNode(SyEbnfType::LVal, line) {}
};

class PrimaryExpAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	PrimaryExpAstNode(int line) : AstNode(SyEbnfType::PrimaryExp, line) {}
};

class NumberAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	NumberAstNode(int line) : AstNode(SyEbnfType::Number, line) {}
};

class UnaryExpAstNode : public AstNode {
   private:
	std::weak_ptr<AstNodeBase> parent_;

   public:
	using AstNode::AstNode;

	UnaryExpAstNode(int line) : AstNode(SyEbnfType::UnaryExp, line) {}
	void setAstParent(std::shared_ptr<AstNodeBase> parent) { parent_ = parent; }
	AstNodePtr getAstParent() { return parent_.lock(); }
};

class UnaryOpAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	UnaryOpAstNode(int line) : AstNode(SyEbnfType::UnaryOp, line) {}
};

class FuncRParamsAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	FuncRParamsAstNode(int line) : AstNode(SyEbnfType::FuncRParams, line) {}
};

class MulExpAstNode : public AstNode {
   private:
	std::weak_ptr<AstNodeBase> parent_;

   public:
	using AstNode::AstNode;

	MulExpAstNode(int line) : AstNode(SyEbnfType::MulExp, line) {}
	void setAstParent(std::shared_ptr<AstNodeBase> parent) { parent_ = parent; }
	AstNodePtr getAstParent() { return parent_.lock(); }
};

class AddExpAstNode : public AstNode {
   private:
	std::weak_ptr<AstNodeBase> parent_;

   public:
	using AstNode::AstNode;

	AddExpAstNode(int line) : AstNode(SyEbnfType::AddExp, line) {}
	void setAstParent(std::shared_ptr<AstNodeBase> parent) { parent_ = parent; }
	AstNodePtr getAstParent() { return parent_.lock(); }
};

class RelExpAstNode : public AstNode {
   private:
	std::weak_ptr<AstNodeBase> parent_;

   public:
	using AstNode::AstNode;

	RelExpAstNode(int line) : AstNode(SyEbnfType::RelExp, line) {}
	void setAstParent(std::shared_ptr<AstNodeBase> parent) { parent_ = parent; }
	AstNodePtr getAstParent() { return parent_.lock(); }
};

class EqExpAstNode : public AstNode {
   private:
	std::weak_ptr<AstNodeBase> parent_;

   public:
	using AstNode::AstNode;

	EqExpAstNode(int line) : AstNode(SyEbnfType::EqExp, line) {}
	void setAstParent(std::shared_ptr<AstNodeBase> parent) { parent_ = parent; }
	AstNodePtr getAstParent() { return parent_.lock(); }
};

class LAndExpAstNode : public AstNode {
   private:
	std::weak_ptr<AstNodeBase> parent_;

   public:
	using AstNode::AstNode;

	LAndExpAstNode(int line) : AstNode(SyEbnfType::LAndExp, line) {}
	void setAstParent(std::shared_ptr<AstNodeBase> parent) { parent_ = parent; }
	AstNodePtr getAstParent() { return parent_.lock(); }
};

class LOrExpAstNode : public AstNode {
   private:
	std::weak_ptr<AstNodeBase> parent_;

   public:
	using AstNode::AstNode;

	LOrExpAstNode(int line) : AstNode(SyEbnfType::LOrExp, line) {}
	void setAstParent(std::shared_ptr<AstNodeBase> parent) { parent_ = parent; }
	AstNodePtr getAstParent() { return parent_.lock(); }
};

class ConstExpAstNode : public AstNode {
   public:
	using AstNode::AstNode;

	ConstExpAstNode(int line) : AstNode(SyEbnfType::ConstExp, line) {}
};

class EAstNode : public AstNode {
   private:
	// this class has this field for the left recursive
	std::weak_ptr<AstNodeBase> parent_;

   public:
	using AstNode::AstNode;
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
