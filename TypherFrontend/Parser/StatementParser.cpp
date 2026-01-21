#include "StatementParser.h"
#include "ExpressionParser.h"

namespace Parser {
	StatementParser::StatementParser(std::shared_ptr<ParserState> state)
	{
		state_ = state;
		state_->expression_parser = Allocator()->Allocate<ExpressionParser>(state_);
	}

	AST::Statement* StatementParser::parse_statement()
	{
		auto token = Lexer()->GetToken();
		AST::Statement* statement;
		switch (token.Type())
		{
		case Lex::TokenType::Keyword:
		{
			statement = HandleKeywords();
			break;
		}
		/*
		case Lex::TokenType::Identifier:
			//HandleIdentifier();
			//break;
		*/
		default:
			return nullptr;
			
			// TODO: handle unreachable
		};

		return statement;
	}

	ArrayAlloc<AST::VariableDeclarator*> StatementParser::ParseDeclarators(AST::Identifier* ident) 
	{
		// This handles int a, b = 5;. The 'a, b = 5;' part.
		Lexer()->NextToken(); // Skip '='
		auto token = Lexer()->GetToken();

		ArrayAlloc<AST::VariableDeclarator*> declarators = Allocator()->ArrayAllocate<AST::VariableDeclarator*>();
		while (true) {
			if (IsStatementEnd())
				break;

			auto expr = state_->expression_parser->parse_expression();
			AST::VariableDeclarator* decl = Allocator()->Allocate<AST::VariableDeclarator>(expr, ident);
			declarators.push_back(decl);

			if (token.IsTokenType<Lex::TokenPunctuator>(Lex::TokenPunctuator::COMMA)) {
				Lexer()->NextToken();
				PottentialVariableOrFunctionDecl(); // THIS CAUSES AN ERROR.  declarators REDIFNED ON EACH CALL.
				continue;
			}

			break; // TODO: HANDLE OTHER CASES
		}

		if (declarators.empty())
		{
			// this means int a = ; throw error.
		}

		return declarators;
	}

	AST::Statement* StatementParser::PottentialVariableOrFunctionDecl()
	{
		// AST::Statement* statement = state_->assignment_parser->parse_assignment();

		Lex::TokenKeyword type = Lexer()->GetToken().GetTokenType<Lex::TokenKeyword>(); // Turn this into a type class.
		auto Ident = ExpectIdentifier();

		Lexer()->NextToken();
		auto token = Lexer()->GetToken();

		switch (token.Type()) {
			case Lex::TokenType::Punctuator:
			{
				if (token.IsTokenType(Lex::TokenPunctuator::SEMICOLON)) {
					auto declarator = Allocator()->Allocate<AST::VariableDeclarator>(Ident);
					return Allocator()->Allocate<AST::VariableDeclaration>(declarator);
				}
				else {
					// throw error
					break;
				}
			}
			case Lex::TokenType::Operator:
			{
				if (token.IsTokenType(Lex::TokenOperator::LEFT_PAREN)) {
					// Parse func
				}
				else if (token.IsTokenType(Lex::TokenOperator::ASSIGNMENT)) {
					auto declarators = ParseDeclarators(Ident); // THIS CAUSES AN ERROR. 
					return Allocator()->Allocate<AST::VariableDeclaration>(declarators);
				}
			}
		}

		return nullptr;
	}

	AST::Statement* StatementParser::HandleKeywords() 
	{
		auto token = Lexer()->GetToken();
		AST::Statement* statement;
		switch (token.GetKeywordType())
		{
		case Lex::TokenKeyword::BOOL:
		case Lex::TokenKeyword::CHAR:
		case Lex::TokenKeyword::SHORT:
		case Lex::TokenKeyword::INT:
		case Lex::TokenKeyword::FLOAT:
		case Lex::TokenKeyword::DOUBLE:
			statement = PottentialVariableOrFunctionDecl();
			break;
		case Lex::TokenKeyword::IF:
			break;
		};

		return statement;
	}
}