#ifndef LITERAL_H
#define LITERAL_H

#include "ASTNode.h"

namespace AST {
	class Literal: public Expression {
	public:
		Literal() : Expression(AstNodeType::LITERAL) { }

		virtual bool IsIntegerLiteral() = 0;
	};
}

#endif