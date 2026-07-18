#include "Parser.h"
#include "Tokens/TokenTypes.h"
#include "Statement.h"
#include "StatementParser.h"
#include "Helpers.h"

namespace Parser {

	Parser::Parser(std::string& file_buffer, 
		DiagnosticEngine& diags, MemoryAllocator *allocator) 
		:diags_(diags), allocator_(allocator)
	 {
		state_ = std::make_shared<ParserState>() ;
		AST_tree = SlabVector<AST::Statement*> (Allocator());
		state_->lexer_ = std::make_unique<Lex::Lexer>(file_buffer);
		state_->statement_parser = Allocator()->Allocate<StatementParser>(state_, 
			diags_, allocator_);
	}

	void Parser::parse()
	{
		Lexer()->NextToken(); // TODO: Move this out of the while.
		while (Lexer()->GetToken().Type() != Lex::TokenType::EOS) {
			AST::Statement* current_statement = state_->statement_parser->parse_statement();
			AST_tree.push_back(current_statement);
		}
	}

	Lex::Lexer* Parser::Lexer() const
	{
		return state_->lexer_.get();
	}

	AstBuiltinTypes Parser::ParseType() 
	{
		// TODO: Turn this into a type class.
		Lex::TokenKeyword token_style_type = Lexer()->GetToken().GetTokenType<Lex::TokenKeyword>(); 
		return TokenTypeToBuiltinAstType(token_style_type);
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
			AST::Identifier* ident = Allocator()->Allocate<AST::Identifier>(token.Ident());
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
		for (auto& node : AST_tree)
		{
			if (node != nullptr) {
				print_statement(node, 0);
				std::cout << std::endl;
			}
		}
	}
}