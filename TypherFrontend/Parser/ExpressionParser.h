#ifndef EXPRESSION_PARSER_H
#define EXPRESSION_PARSER_H

#include "Parser.h"
#include "AST/Expression.h"
#include "AST/Expressions/CallExpression.h"
#include "AST/Expressions/Operator.h"

namespace Parser {
	class ExpressionParser : public Parser {
	public:
		ExpressionParser(std::shared_ptr<ParserState> state)
		{
			state_ = state;
		}

		AST::Expression* parse_expression();

	private:
		AST::CallExpression* ParseFunctionCall(AST::Identifier* ident);
		AST::ASTNode* CheckIdentifier();
	};
}

#endif