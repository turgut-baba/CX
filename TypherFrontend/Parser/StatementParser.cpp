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
		case Lex::TokenType::Identifier:
		{
			statement = HandleIdentifier();
			break;
		}
		default:
			UNREACHABLE("Unknown statement start.");
			return nullptr;
		};

		return statement;
	}

	AST::IfStatement* StatementParser::HandleIfKeyword()
	{
		// TODO: on subsequent if-else blocks each block gets nested parents. We want the first
		// if statement to be the parent of all.
		Lexer()->NextToken(); // Skip 'if';
		
		Lexer()->NextToken(); // Skip '('
		auto condition = state_->expression_parser->parse_expression();
		AST::IfStatement* if_statement = Allocator()->Allocate<AST::IfStatement>(condition, Allocator());
		Lexer()->NextToken(); // Skip ')';

		AST::Body* if_body = ParseBody(if_statement);
		if_statement->SetBody(if_body);

		if (Lexer()->GetToken().IsTokenType<Lex::TokenKeyword>(Lex::TokenKeyword::ELSE)) 
		{		
			Lexer()->NextToken(); // Skip 'else';
			if(Lexer()->GetToken().IsTokenType<Lex::TokenKeyword>(Lex::TokenKeyword::IF))
			{
				AST::IfStatement* elif_statement = HandleIfKeyword();
				if_statement->SetElif(elif_statement);
				elif_statement->SetParent(if_statement);
			} else {
				AST::Body* else_body = ParseBody(if_statement);
				if_statement->SetElse(else_body);
			}
		}

		return if_statement;
	}

	// TODO: move this to expression parser. Declaration should be a statement but declarator should be 
	// an expression.
	SlabVector<AST::VariableDeclarator*> StatementParser::ParseDeclarators(AST::Identifier* ident) 
	{
		// This handles int a, b = 5;. The 'a, b = 5;' part.
		Lexer()->NextToken(); // Skip '='
		auto token = Lexer()->GetToken();

		SlabVector<AST::VariableDeclarator*> declarators = Allocator()->ArrayAllocate<AST::VariableDeclarator*>();
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
			state_->diags.report<DiagLevel::Error>({}) 
				<< "Expected expression.";
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
					auto declaration = Allocator()->Allocate<AST::VariableDeclaration>(declarator, Allocator());
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

	AST::Body* StatementParser::ParseBody(AST::Statement* owner)
	{
		Lexer()->NextToken(); // Skip '{'
		AST::Body* body = Allocator()->Allocate<AST::Body>(Allocator());
		while (!Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::RIGHT_CURLY_BRACE)) {
			auto statement = parse_statement();
			statement->SetParent(owner);
			body->AddStatement(statement);
		}
		Lexer()->NextToken(); // Skip '}'
		return body;
	}

	AST::Statement* StatementParser::ParseFunction(AST::Identifier* ident)
	{
		Lexer()->NextToken(); // Skip '('
		AST::Function* functionDecl = Allocator()->Allocate<AST::Function>(ident, Allocator());
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

		AST::Body* function_body = ParseBody(functionDecl);
		functionDecl->SetBody(function_body);

		return functionDecl;
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
		switch (token.GetTokenType<Lex::TokenKeyword>())
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
			statement = HandleIfKeyword();
			break;
		case Lex::TokenKeyword::ELSE:
			state_->diags.report<DiagLevel::Error>({}) 
				<< "Expected parent if statement.";
			break;
		case Lex::TokenKeyword::RETURN:
			statement = HandleReturnStatement();
			break;
		};

		return statement;
	}

	AST::Statement* StatementParser::HandleIdentifier() 
	{
		auto expr = state_->expression_parser->parse_expression();

		AST::ExpressionStatement* expr_statement = 
			Allocator()->Allocate<AST::ExpressionStatement>(expr);

		Lexer()->NextToken(); // Skip ';'

		return expr_statement;
	}
}