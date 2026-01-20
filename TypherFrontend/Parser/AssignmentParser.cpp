#include "AssignmentParser.h"
#include "AST/statements/VariableDeclaration.h"
#include "ExpressionParser.h"


namespace Parser {

	using namespace Lex;

	AssignmentParser::AssignmentParser(std::shared_ptr<ParserState> state)
	{
		state_ = state;
	}

	AST::VariableDeclarator* AssignmentParser::ParseDeclarator()
	{
		//switch () { } // TODO: check for =, += or -=.
		auto exprs = state_->expression_parser->parse_expression();
		AST::VariableDeclarator* decl = Allocator()->Allocate<AST::VariableDeclarator>(exprs);
		return decl;
	}

	AST::Statement* AssignmentParser::parse_assignment()
	{
		auto token = Lexer()->GetToken();
		Lexer()->NextToken();

		if (token.IsTokenType<TokenPunctuator>(TokenPunctuator::LEFT_PARENTHESES)) {
			//return ParseFunvtion();
		}

		ArrayAlloc<AST::VariableDeclarator*> declarators = Allocator()->ArrayAllocate<AST::VariableDeclarator *>();
		while (true) {
			if (token.IsTokenType<TokenPunctuator>(TokenPunctuator::SEMICOLON))
				break;

			if (token.IsTokenType<TokenPunctuator>(TokenPunctuator::COMMA))
				continue;

			declarators.push_back(ParseDeclarator());
			break; // TODO: HANDLE OTHER CASES
		}
		return Allocator()->Allocate<AST::VariableDeclaration>(declarators);
	}
	
};