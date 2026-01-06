#ifndef CALL_EXPRESSION_H
#define CALL_EXPRESSION_H

#include "AST/ASTNode.h"
#include "AST/Expression.h"

namespace AST {
	class CallExpression : public Expression {
	public:
		CallExpression() = default;

		virtual ~CallExpression() = default;
	};
}

#endif