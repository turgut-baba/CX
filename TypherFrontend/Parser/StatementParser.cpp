#include "StatementParser.h"
#include "ExpressionParser.h"
#include "DeclaratorParser.hpp"

#include "Helpers.h"

namespace Parser {

	StatementParser::StatementParser(std::shared_ptr<ParserState> state,
		DiagnosticEngine& diags, MemoryAllocator *allocator)
		: Parser(diags, allocator)
	{
		state_ = state;
		state_->expression_parser = Allocator()->Allocate<ExpressionParser>(state_, 
			diags_, allocator_);
		state_->declarator_parser = Allocator()->Allocate<DeclaratorParser>(state_, 
			diags_, allocator_);
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
			UNREACHABLE("Unknown statement start: " + token.Ident());
			return nullptr;
		};

		return statement;
	}

	AST::ForStatement* StatementParser::HandleForKeyword()
	{
		Lexer()->NextToken(); // Skip 'for';
		
		Lexer()->NextToken(); // Skip '('

		// TODO: Add a parameter here like 'IsVariableOnly' so it throws an error when it encounters 
		// a function declaration.
		auto init = state_->declarator_parser->VariableOrFunctionDecl();
		
		auto cond = state_->expression_parser->parse_expression();

		Lexer()->NextToken(); // Skip ';'

		auto iter = state_->expression_parser->parse_expression();

		Lexer()->NextToken(); // Skip ')'
		
		AST::ForStatement* for_statement = 
			Allocator()->Allocate<AST::ForStatement>(init, cond, iter, Allocator());

		AST::Body* for_body = ParseBody(for_statement);
		for_statement->SetBody(for_body);

		return for_statement;
	}

	AST::WhileStatement* StatementParser::HandleWhileKeyword()
	{
		Lexer()->NextToken(); // Skip 'while';
		
		Lexer()->NextToken(); // Skip '('
		auto condition = state_->expression_parser->parse_expression();
		AST::WhileStatement* while_statement = 
			Allocator()->Allocate<AST::WhileStatement>(condition, Allocator());

		Lexer()->NextToken(); // Skip ')';

		AST::Body* while_body = ParseBody(while_statement);
		while_statement->SetBody(while_body);

		return while_statement;
	}


	AST::IfStatement* StatementParser::HandleIfKeyword()
	{
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
			statement = state_->declarator_parser->VariableOrFunctionDecl();
			break;
		case Lex::TokenKeyword::IF:
			statement = HandleIfKeyword();
			break;
		case Lex::TokenKeyword::WHILE:
			statement = HandleWhileKeyword();
			break;
		case Lex::TokenKeyword::FOR:
			statement = HandleForKeyword();
			break;
		case Lex::TokenKeyword::ELSE:
			Diagnostic().report<DiagLevel::Error>({}) 
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