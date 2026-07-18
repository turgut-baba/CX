#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "Expressions/Identifier.h"
#include "Statement.h"
#include "Memory/MemAlloc.h"
#include "Memory/BumpPtrAlloc.h"
#include "statements/VariableDeclaration.h"
#include "ParserState.h"

#include <memory>

namespace Parser {

struct ParserState;

class Parser {
public:
	Parser(std::string& file_buffer, 
		DiagnosticEngine& diags, MemoryAllocator *allocator);
	
	void parse();

	SlabVector<AST::Statement*> AST()
	{
		return AST_tree;
	}

	void PrintAST(); //Debug
protected:

	Parser(DiagnosticEngine& diags, MemoryAllocator *allocator)
		:diags_(diags), allocator_(allocator)
	{
	    state_ = std::make_shared<ParserState>() ;
		AST_tree = SlabVector<AST::Statement*>(Allocator());
	}

	
	template <AllocatorType Type = AllocatorType::SLAB>
	auto Allocator() -> typename AllocTypeMap<Type>::type*
	{
		if constexpr (Type == AllocatorType::DTOR)
		return allocator_->dtorAlloc.get();
		if constexpr (Type == AllocatorType::SLAB)
			return allocator_->slabAlloc.get();
			if constexpr (Type == AllocatorType::BUMP)
			return allocator_->bumpAlloc.get();
		}
		
	DiagnosticEngine& Diagnostic() const
	{
		return diags_;
	}
	
	
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
			diags_.report<DiagLevel::Error>({}) 
			<< "Unexpected token, expected: " << (char)expected;
		}
		Lexer()->NextToken();
	}
	
	Lex::Lexer* Lexer() const;
	bool IsStatementEnd();

protected:
	AstBuiltinTypes ParseType();
	AST::Identifier* ExpectIdentifier();
	SlabVector<AST::Statement*> AST_tree;

	std::shared_ptr<ParserState> state_;

	MemoryAllocator* allocator_;
	DiagnosticEngine& diags_;
};

}

#endif