#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <iostream>
#include "ASTNode.h"
#include "Expression.h"

namespace AST {

	enum ExpressionModifiers { DeRef, ArrayAccess, AddressOf, MemberAccess };

	class Identifier: public Expression {
	public:
		Identifier(std::string value) 
			: Expression(AstNodeType::IDENTIFIER), value_(value) { }

		Identifier(std::string value, const std::vector<ExpressionModifiers>& modifiers)
			: Expression(AstNodeType::IDENTIFIER), value_(value), modifiers_(modifiers) { }

		std::string Value() const
		{
			return value_;
		}

		void AddArrayAccessExpression(Expression* expr)
		{
			array_access_expressions_.push_back(expr);
		}

		std::string String() override
		{
			return ("Ident node: " + value_);
		}

		const std::vector<ExpressionModifiers>& Modifiers() 
		{
			return modifiers_;
		}

		Expression* Result() override 
		{
			return this;
		}

		void Accept(NodeVisitor* v) override { v->Visit(this); }
	private:
		std::string value_;
		SlabVector<Expression*> array_access_expressions_;
		std::vector<ExpressionModifiers> modifiers_;
	};
}

#endif