#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "AST/Expressions/Identifier.h"
#include "AST/Statement.h"

namespace Parser {

struct ParserState;

class Parser {
public:
	Parser() = default;

	Parser(std::string& file_buffer);

	void parse();

	Lex::Lexer* Lexer() const;

	~Parser() 
	{
		delete state_;
	}
protected:
	AST::Identifier* ExpectIdentifier();

	ParserState* state_; // TODO: write a proper allocator instead of raw ptr
};

}

#endif