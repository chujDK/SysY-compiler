#ifndef _SYPARSE_H_
#define _SYPARSE_H_
#include <cassert>
#include <list>
#include <memory>
#include <string>

#include "sytype.h"
#include "utils.h"

struct AstNode;
class TokenAstNode;
using TokenPtr   = std::shared_ptr<TokenAstNode>;
using AstNodePtr = std::shared_ptr<AstNode>;
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

struct AstNode {
	enum SyEbnfType ebnf_type_;  // delete this field in the future
	unsigned int line_;
	// only to the EBnfType::TYPE_INT_ARRAY, array_size_ can be used
	// this also means that this complier can only support array size up to
	// 2^32-1 only to the EBnfType::ConstDef, array_size_ can be used
	// it's tempting to move the line_ into this union, but giveup
	// delete this field in the future
	union {
		unsigned int array_size_;
		unsigned int const_val_;
	} u_;
	// in the ast, the meaning is self-explained;
	// if the nodes are in a list, like FuncFParam to FuncFParams,
	// parent_ and d_ link up a double linked list
	// be careful when using the d_, make sure it's not in a list
	std::weak_ptr<AstNode> parent_;  // delete this field in the future
	AstNodePtr a_, b_, c_, d_;       // delete this field in the future
	AstNode(int line)
	    : ebnf_type_(SyEbnfType::END_OF_ENUM),
	      line_(line),
	      a_(nullptr),
	      b_(nullptr),
	      c_(nullptr),
	      d_(nullptr) {
		u_.const_val_ = 0xFFFFFFFF;
	}

	AstNode(enum SyEbnfType ebnf_type, int line)
	    : ebnf_type_(ebnf_type),
	      line_(line),
	      a_(nullptr),
	      b_(nullptr),
	      c_(nullptr),
	      d_(nullptr) {
		u_.const_val_ = 0xFFFFFFFF;
	}

	virtual ~AstNode() {}
	virtual std::string const& getLiteral() {
		DEBUG_ASSERT_NOT_REACH return "";
	}

	virtual SyAstType getAstType() { return SyAstType::END_OF_ENUM; }

	virtual void Accept(AstNodeVisitor* visitor) { DEBUG_ASSERT_NOT_REACH }
	virtual void irGen() { DEBUG_ASSERT_NOT_REACH }
	virtual void checkSemantic() { DEBUG_ASSERT_NOT_REACH }
};

class TokenAstNode : public AstNode {
   private:
	std::string literal_;
	enum SyAstType ast_type_;

   public:
	TokenAstNode(enum SyAstType ast_type, int line, std::string&& literal)
	    : AstNode(line), literal_(literal), ast_type_(ast_type) {}

	std::string const& getLiteral() { return literal_; }

	SyAstType getAstType() { return ast_type_; }
};

class CompUnitAstNode : public AstNode {
   public:
	CompUnitAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class DeclAstNode : public AstNode {
   public:
	DeclAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class ConstDeclAstNode : public AstNode {
   public:
	ConstDeclAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class BTypeAstNode : public AstNode {
   public:
	BTypeAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class ConstDefAstNode : public AstNode {
   public:
	ConstDefAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class ConstInitValAstNode : public AstNode {
   public:
	ConstInitValAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class VarDeclAstNode : public AstNode {
   public:
	VarDeclAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class VarDefAstNode : public AstNode {
   public:
	VarDefAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class InitValAstNode : public AstNode {
   public:
	InitValAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class FuncDefAstNode : public AstNode {
   public:
	FuncDefAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class FuncTypeAstNode : public AstNode {
   public:
	FuncTypeAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class FuncFParamsAstNode : public AstNode {
   public:
	FuncFParamsAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class FuncFParamAstNode : public AstNode {
   public:
	FuncFParamAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class BlockAstNode : public AstNode {
   public:
	BlockAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class BlockItemAstNode : public AstNode {
   public:
	BlockItemAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class StmtAstNode : public AstNode {
   public:
	StmtAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class ExpAstNode : public AstNode {
   public:
	ExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class CondAstNode : public AstNode {
   public:
	CondAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class LValAstNode : public AstNode {
   public:
	LValAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class PrimaryExpAstNode : public AstNode {
   public:
	PrimaryExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class NumberAstNode : public AstNode {
   public:
	NumberAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class UnaryExpAstNode : public AstNode {
   public:
	UnaryExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class UnaryOpAstNode : public AstNode {
   public:
	UnaryOpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class FuncRParamsAstNode : public AstNode {
   public:
	FuncRParamsAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class MulExpAstNode : public AstNode {
   public:
	MulExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class AddExpAstNode : public AstNode {
   public:
	AddExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class RelExpAstNode : public AstNode {
   public:
	RelExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class EqExpAstNode : public AstNode {
   public:
	EqExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class LAndExpAstNode : public AstNode {
   public:
	LAndExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class LOrExpAstNode : public AstNode {
   public:
	LOrExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class ConstExpAstNode : public AstNode {
   public:
	ConstExpAstNode(enum SyEbnfType ebnf_type, int line)
	    : AstNode(ebnf_type, line) {}
};

class EAstNode : public AstNode {
   public:
	EAstNode(enum SyEbnfType ebnf_type, int line) : AstNode(ebnf_type, line) {}
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
