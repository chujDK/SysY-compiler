#include "sysymbol_table.h"

#include <cassert>

IdentMemoryPtr SymbolTable::searchTable(AstNodePtr ident) {
	for (int i = current_scope_; i >= 0; i--) {
		if (symbol_table_[i].find(ident->getLiteral()) !=
		    symbol_table_[i].end()) {
			return symbol_table_[i][ident->getLiteral()];
		}
	}
	return nullptr;
}

IdentMemoryPtr SymbolTable::searchCurrentScope(AstNodePtr ident) {
	if (symbol_table_[current_scope_].find(ident->getLiteral()) !=
	    symbol_table_[current_scope_].end()) {
		return symbol_table_[current_scope_][ident->getLiteral()];
	}
	return nullptr;
}

IdentMemoryPtr IdentMemory::AllocMemoryForIdent(AstNodePtr ident,
                                                bool is_const) {
	std::shared_ptr<IdentMemoryAPI> mem(nullptr);
	switch (ident->ebnf_type_) {
		case SyEbnfType::TYPE_INT:
			mem.reset((IdentMemoryAPI*)new IdentMemory(sizeof(int), is_const));
			return mem;
		case SyEbnfType::TYPE_INT_ARRAY:
			mem.reset((ArrayMemoryAPI*)new ArrayMemory(
			    sizeof(int) * ident->u_.array_size_, is_const));
			return mem;
		default:
			// shouldn't reach here
			// cause a bug
			assert(1 != 1);
			return nullptr;
	}
}

inline IdentMemoryPtr SymbolTable::addSymbol(AstNodePtr ident, bool is_const) {
	// if this ident isn't exist in the current scope, delete won't hurt
	deleteSymbol(ident);
	IdentMemoryPtr mem = IdentMemory::AllocMemoryForIdent(ident, is_const);
	symbol_table_[current_scope_][ident->getLiteral()] = mem;
	return mem;
}

inline IdentMemoryPtr SymbolTable::addGlobalSymbol(AstNodePtr ident,
                                                   bool is_const) {
	// if this ident isn't exist in the current scope, delete won't hurt
	IdentMemoryPtr mem = IdentMemory::AllocMemoryForIdent(ident, is_const);
	symbol_table_[0][ident->getLiteral()] = mem;
	return mem;
}

// note that this function can still work when ident isn't exist in the current
// scope
inline void SymbolTable::deleteSymbol(AstNodePtr ident) {
	symbol_table_[current_scope_].erase(ident->getLiteral());
}

inline void SymbolTable::enterScope() {
	symbol_table_.push_back(std::map<std::string, IdentMemoryPtr>());
	current_scope_++;
}

inline void SymbolTable::exitScope() {
	symbol_table_.pop_back();
	current_scope_--;
}