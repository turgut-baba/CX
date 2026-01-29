#include "Parser.h"
#include "Tokens/TokenTypes.h"
#include "AST/Statement.h"
#include "ParserState.h"
#include "StatementParser.h"

namespace Parser {

	Parser::Parser(std::string& file_buffer) {
		state_ = std::make_shared<ParserState>(); // TODO: TURN THIS INTO CUSTOM ALLOCATOR
		state_->lexer_ = std::make_unique<Lex::Lexer>(file_buffer);
		state_->statement_parser = Allocator()->Allocate<StatementParser>(state_); // TODO: TURN THIS INTO CUSTOM ALLOCATOR
	}

	void Parser::parse()
	{
		Lexer()->NextToken(); // TODO: Move this out of the while.
		while (Lexer()->GetToken().Type() != Lex::TokenType::EOS) {
			AST::Statement* current_statement = state_->statement_parser->parse_statement();
			statements_.push_back(current_statement);
		}
	}

	Lex::Lexer* Parser::Lexer() const
	{
		return state_->lexer_.get();
	}

	MemoryAllocator* Parser::Allocator() const
	{
		return &state_->allocator;
	}

	bool Parser::IsStatementEnd()
	{
		const auto token = Lexer()->GetToken();
		if (token.Type() == Lex::TokenType::Punctuator &&
			Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::SEMICOLON))
		{
			return true;
		}

		return false;
	}

	AST::Identifier* Parser::ExpectIdentifier()
	{
		Lexer()->NextToken();
		const auto token = Lexer()->GetToken();
		if (token.Type() == Lex::TokenType::Identifier)
		{
			AST::Identifier* ident = Allocator()->Allocate<AST::Identifier>(token.Ident()); // TODO: TURN THIS INTO CUSTOM ALLOCATOR
			return ident;
		}

		// TODO: LOG ERROR
		return nullptr;
	}

	void print_statement(AST::ASTNode* node, int depth)
	{
		for (int i = 0; i < depth; i++) {
			std::cout << "--";
		}
		std::cout << node->String() << std::endl;
		for (auto& child : node->Chlidren())
		{
			if (child != nullptr) print_statement(child, depth + 1);
		}
	}

	void Parser::PrintAST()
	{
		for (auto& node : statements_.vec_)
		{
			if (node != nullptr) {
				print_statement(node, 0);
				std::cout << std::endl;
			}
		}
	}
}