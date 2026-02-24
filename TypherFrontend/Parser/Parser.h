#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "AST/Expressions/Identifier.h"
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
	Parser() { 
		std::cout << "Def ctor" << std::endl;
	    state_ = std::make_shared<ParserState>() ;
		statements_ = SlabVector<AST::Statement*> (Allocator());
	};

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
		if (!Lexer()->GetToken().IsTokenType(expected))
		{
			state_->diags.report<DiagLevel::Error>({}) 
            	<< "Unexpected token, expected: " << (char)expected;
		}
		Lexer()->NextToken();
	}
protected:
	AST::Identifier* ExpectIdentifier();
	SlabVector<AST::Statement*> statements_;

	std::shared_ptr<ParserState> state_;
	MemoryAllocator allocator_;
};

}

#endif