#ifndef CALL_EXPRESSION_H
#define CALL_EXPRESSION_H

#include "AST/ASTNode.h"
#include "AST/Expression.h"

namespace AST {
	class CallExpression : public Expression {
	public:
		CallExpression() = default;

		CallExpression(std::string callee, ArrayAlloc<Expression*> args) 
			: Arguments_(args) {}

		virtual ~CallExpression() = default;

		std::string Callee()
		{
			return callee_;
		}

		ArrayAlloc<Expression*> Args()
		{
			return Arguments_;
		}

		std::string String()
		{
			return "Call Expression";
		}
	private:
		ArrayAlloc<Expression*> Arguments_;
		std::string callee_;
	};
}

#endif