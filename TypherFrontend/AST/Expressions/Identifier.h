#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <iostream>
#include "ASTNode.h"
#include "Expression.h"

namespace AST {
	class Identifier: public Expression {
	public:
		Identifier(std::string value) 
			: Expression(AstNodeType::IDENTIFIER), value_(value) { }

		std::string Value() const
		{
			return value_;
		}

		std::string String() override
		{
			return ("Ident node: " + value_);
		}

		void Accept(NodeVisitor* v) override { v->Visit(this); }
	private:
		std::string value_;
	};
}

#endif