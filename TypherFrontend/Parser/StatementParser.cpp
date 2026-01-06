#include "StatementParser.h"
#include "AssignmentParser.h"

namespace Parser {
	StatementParser::StatementParser(ParserState* state)
	{
		state_ = state;
		state_->assignment_parser = new AssignmentParser(state_); // TODO: proper mem management
	}

	AST::Statement* StatementParser::parse_statement()
	{
		auto token = Lexer()->GetToken();
		switch (token.Type())
		{
		case Lex::TokenType::Keyword:
			HandleKeywords();
			break;
		case Lex::TokenType::Identifier:
			//HandleIdentifier();
			break;
		default:
			break;
			// TODO: handle unreachable
		};

		return nullptr;
	}

	AST::Statement* StatementParser::PottentialVariableOrFunctionDecl()
	{
		auto Ident = ExpectIdentifier();
		AST::Statement* statement = state_->assignment_parser->parse_assignment();
		
		return statement;
	}

	AST::Statement* StatementParser::HandleKeywords() 
	{
		auto token = Lexer()->GetToken();
		switch (token.GetKeywordType())
		{
		case Lex::TokenKeyword::BOOL:
		case Lex::TokenKeyword::CHAR:
		case Lex::TokenKeyword::SHORT:
		case Lex::TokenKeyword::INT:
		case Lex::TokenKeyword::FLOAT:
		case Lex::TokenKeyword::DOUBLE:
			PottentialVariableOrFunctionDecl();
			break;
		case Lex::TokenKeyword::IF:
			break;
		};

		return nullptr;
	}
}