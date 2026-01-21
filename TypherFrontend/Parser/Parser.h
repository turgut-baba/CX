#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "AST/Identifier.h"
#include "AST/Statement.h"
#include "Memory/MemAlloc.h"
#include "AST/statements/VariableDeclaration.h"

namespace Parser {

struct ParserState;

class Parser {
public:
	Parser() = default;

	Parser(std::string& file_buffer);

	void parse();

	Lex::Lexer* Lexer() const;

	MemoryAllocator* Allocator() const;

	~Parser() 
	{
		
	}
	
	bool IsStatementEnd();

	void PrintAST(); //Debug
protected:
	AST::Identifier* ExpectIdentifier();
	ArrayAlloc<AST::Statement*> statements_;

	std::shared_ptr<ParserState> state_; // TODO: write a proper allocator instead of raw ptr
	MemoryAllocator allocator_;
};

}

#endif