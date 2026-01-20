#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "AST/Expressions/Identifier.h"
#include "AST/Statement.h"
#include "Memory/MemAlloc.h"

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
protected:
	AST::Identifier* ExpectIdentifier();

	std::shared_ptr<ParserState> state_; // TODO: write a proper allocator instead of raw ptr
	MemoryAllocator allocator_;
};

}

#endif