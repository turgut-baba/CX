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

	// TODO: move this to expression parser. Declaration should be a statement but declarator should be 
	// an expression.
	ArrayAlloc<AST::VariableDeclarator*> StatementParser::ParseDeclarators(AST::Identifier* ident) 
	{
		// This handles int a, b = 5;. The 'a, b = 5;' part.
		Lexer()->NextToken(); // Skip '='
		auto token = Lexer()->GetToken();

		ArrayAlloc<AST::VariableDeclarator*> declarators = Allocator()->ArrayAllocate<AST::VariableDeclarator*>();
		while (true) {
			auto expr = state_->expression_parser->parse_expression();
			AST::VariableDeclarator* decl = Allocator()->Allocate<AST::VariableDeclarator>(expr, ident);
			declarators.push_back(decl);

			if (IsStatementEnd())
			{
				Lexer()->NextToken();
				break;
			}

			if (token.IsTokenType<Lex::TokenPunctuator>(Lex::TokenPunctuator::COMMA)) {
				Lexer()->NextToken();
				ParseDeclarators(ident); // THIS CAUSES AN ERROR.  declarators REDIFNED ON EACH CALL.
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
				if (token.IsTokenType(Lex::TokenPunctuator::LEFT_PARENTHESES)) {
					return ParseFunction(Ident);
				} else if (token.IsTokenType(Lex::TokenPunctuator::SEMICOLON)) {
					auto declarator = Allocator()->Allocate<AST::VariableDeclarator>(Ident);
					auto declaration = Allocator()->Allocate<AST::VariableDeclaration>(declarator);
					Lexer()->NextToken();
					return declaration;
				}
				else {
					// throw error
					break;
				}
			}
			case Lex::TokenType::Operator:
			{
				if (token.IsTokenType(Lex::TokenOperator::ASSIGNMENT)) {
					auto declarators = ParseDeclarators(Ident); // THIS CAUSES AN ERROR. 
					return Allocator()->Allocate<AST::VariableDeclaration>(declarators);
				}
			}
		}

		return nullptr;
	}

	AST::Statement* StatementParser::ParseFunctionBody(AST::Function* functionDecl)
	{
		Lexer()->NextToken(); // Skip '{'
		while (!Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::RIGHT_CURLY_BRACE)) {
			auto statement = parse_statement();
			statement->SetParent(functionDecl);
		}
		Lexer()->NextToken(); // Skip '}'
		return functionDecl;
	}

	AST::Statement* StatementParser::ParseFunction(AST::Identifier* ident)
	{
		Lexer()->NextToken(); // Skip '('
		AST::Function* functionDecl = Allocator()->Allocate<AST::Function>(ident);
		while (!Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::RIGHT_PARENTHESES)) {
			// ParseParameter();
			if (!Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::COMMA)) {
				// Log error
			}
		}
		Lexer()->NextToken(); // Skip ')'

		if (!Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::SEMICOLON)) {
			// return;
		}

		return ParseFunctionBody(functionDecl);
	}

	AST::ReturnStatement* StatementParser::HandleReturnStatement()
	{
		// TODO: also add which function it belongs to.
		Lexer()->NextToken(); // Skip 'return'

		if (Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::SEMICOLON)) {
			return Allocator()->Allocate<AST::ReturnStatement>();
		}

		auto ReturnedExpr = state_->expression_parser->parse_expression();
		Lexer()->NextToken(); // Skip ';'
		return Allocator()->Allocate<AST::ReturnStatement>(ReturnedExpr);
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
		case Lex::TokenKeyword::RETURN:
			std::cout << "Handling return:" << std::endl;
			statement = HandleReturnStatement();
			std::cout << "Done." << std::endl;
			break;
		};

		return statement;
	}
}