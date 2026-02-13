#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "AST/Identifier.h"
#include "AST/Statement.h"
#include "Memory/MemAlloc.h"
#include "Memory/BumpPtrAlloc.h"
#include "AST/statements/VariableDeclaration.h"
#include "ParserState.h"

#include <memory>

namespace Parser {

struct ParserState;

class Parser {
public:
	Parser() = default;

	Parser(std::string& file_buffer);

	void parse();

	SlabVector<AST::Statement*> AST()
	{
		return statements_;
	}

	void PrintAST(); //Debug
protected:
	Lex::Lexer* Lexer() const;

	template <AllocatorType Type = AllocatorType::SLAB>
	auto Allocator() -> typename AllocTypeMap<Type>::type*
	{
		if constexpr (Type == AllocatorType::DTOR) 
			return state_->allocator.dtorAlloc.get();
		if constexpr (Type == AllocatorType::SLAB) 
			return state_->allocator.slabAlloc.get();
		if constexpr (Type == AllocatorType::BUMP) 
			return state_->allocator.bumpAlloc.get();
	}

	bool IsStatementEnd();

	template<typename TokenType>
	Lex::Token ExpectToken(TokenType expected)
	{
		if (Lexer()->GetToken().IsTokenType(expected))
		{
			return Lexer()->GetToken();
		}

		// TODO: Log error
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
	SlabVector<AST::Statement*> statements_;

	std::shared_ptr<ParserState> state_; // TODO: write a proper allocator instead of raw ptr
	MemoryAllocator allocator_;
};

}

#endif