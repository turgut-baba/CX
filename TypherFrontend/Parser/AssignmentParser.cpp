#include "AssignmentParser.h"
#include "AST/statements/VariableDeclaration.h"
#include "ExpressionParser.h"

namespace Parser {

	using namespace Lex;

	AssignmentParser::AssignmentParser(ParserState* state)
	{
		state_ = state;
	}

	AST::VariableDeclarator* AssignmentParser::ParseDeclarator()
	{
		return nullptr;
	}

	AST::Statement* AssignmentParser::parse_assignment()
	{
		auto token = Lexer()->GetToken();
		Lexer()->NextToken();

		if (token.IsTokenType<TokenPunctuator>(TokenPunctuator::LEFT_PARENTHESES)) {
			//return ParseFunvtion();
		}

		AST::VariableDeclarator* declarators;
		while (true) {
			if (token.IsTokenType<TokenPunctuator>(TokenPunctuator::SEMICOLON))
				break;

			if (token.IsTokenType<TokenPunctuator>(TokenPunctuator::COMMA))
				continue;

			//declarators[i] = ParseDeclarator();
			break; // TODO: HANDLE OTHER CASES
		}
		return new AST::VariableDeclaration(declarators);
	}
	
};