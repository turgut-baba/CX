#ifndef CALL_EXPRESSION_H
#define CALL_EXPRESSION_H

#include "AST/ASTNode.h"
#include "AST/Expression.h"

namespace AST {
	class CallExpression : public Expression {
	public:
		CallExpression() = default;

		CallExpression(std::string callee, SlabVector<Expression*> args) 
			: Arguments_(args), callee_(callee) {}

		virtual ~CallExpression() = default;

		std::string Callee()
		{
			return callee_;
		}

		SlabVector<Expression*> Args()
		{
			return Arguments_;
		}

		std::string String()
		{
			return "Call Expression";
		}
	private:
		SlabVector<Expression*> Arguments_;
		std::string callee_;
	};
}

#endif