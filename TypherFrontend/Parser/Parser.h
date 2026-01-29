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

	ArrayAlloc<AST::Statement*> AST()
	{
		return statements_;
	}

	void PrintAST(); //Debug
protected:
	Lex::Lexer* Lexer() const;

	MemoryAllocator* Allocator() const;

	bool IsStatementEnd();

	template<typename TokenType>
	AST::ASTNode* ExpectToken(TokenType expected)
	{
		if (Lexer()->GetToken().IsTokenType(expected))
		{
			return Lexer()->GetToken();
		}

		// TODO: Log error
		return nullptr;
	}

	template<typename TokenType>
	void SkipToken(TokenType expected)
	{
		Lexer()->NextToken();
		if (!Lexer()->GetToken().IsTokenType(expected))
		{
			// TODO: Log error
		}
	}
protected:
	AST::Identifier* ExpectIdentifier();
	ArrayAlloc<AST::Statement*> statements_;

	std::shared_ptr<ParserState> state_; // TODO: write a proper allocator instead of raw ptr
	MemoryAllocator allocator_;
};

}

#endif