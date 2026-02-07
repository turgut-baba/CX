#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <iostream>
#include "AST/ASTNode.h"

namespace AST {
	class Identifier: public ASTNode {
	public:
		Identifier(std::string value) : value_(value) { }

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