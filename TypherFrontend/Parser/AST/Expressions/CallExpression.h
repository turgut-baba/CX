#ifndef CALL_EXPRESSION_H
#define CALL_EXPRESSION_H

#include "AST/ASTNode.h"
#include "AST/Expression.h"

namespace AST {
	class CallExpression : public Expression {
	public:
		CallExpression()
			: Expression(AstNodeType::CALL_EXPR) {}

		CallExpression(std::string callee, SlabVector<Expression*> args) 
			: Expression(AstNodeType::CALL_EXPR), Arguments_(args), callee_(callee) {}

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

		void Accept(NodeVisitor* v) override { v->Visit(this); }
	private:
		SlabVector<Expression*> Arguments_;
		std::string callee_;
	};
}

#endif