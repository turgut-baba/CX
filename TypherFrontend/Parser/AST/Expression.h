#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "ASTNode.h"
#include "Tokens/TokenTypes.h"

namespace AST {
	class Expression: public ASTNode {
	public:
		virtual ~Expression() = default;

		virtual std::string String() override
		{
			return "Expression node ";
		}

		Lex::TokenKeyword type_; // TODO: turn this into a type class.
	};
}

#endif