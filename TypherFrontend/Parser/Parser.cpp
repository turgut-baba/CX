#include "Parser.h"
#include "Tokens/TokenTypes.h"
#include "AST/Statement.h"
#include "ParserState.h"
#include "StatementParser.h"

namespace Parser {

	Lex::Lexer* Parser::Lexer() const
	{
		return state_->lexer_.get();
	}

	Parser::Parser(std::string& file_buffer) {
		state_ = new ParserState(); // TODO: TURN THIS INTO CUSTOM ALLOCATOR
		state_->lexer_ = std::make_unique<Lex::Lexer>(file_buffer);
		state_->statement_parser = new StatementParser(state_); // TODO: TURN THIS INTO CUSTOM ALLOCATOR
	}

	void Parser::parse() 
	{
		while (Lexer()->GetToken().Type() != Lex::TokenType::EOS) {
			Lexer()->NextToken(); // TODO: Move this out of the while.
			AST::Statement* current_statement = state_->statement_parser->parse_statement();
			std::cout << "Token: " << Lexer()->GetToken().Ident() << std::endl;
		}
	}

	AST::Identifier* Parser::ExpectIdentifier()
	{
		Lexer()->NextToken();
		const auto token = Lexer()->GetToken();
		if (token.Type() == Lex::TokenType::Identifier)
		{
			AST::Identifier* ident = new AST::Identifier(token.Ident()); // TODO: TURN THIS INTO CUSTOM ALLOCATOR
			return ident;
		}

		// TODO: LOG ERROR
		return nullptr;
	}
}