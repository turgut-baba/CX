#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "ASTNode.h"
#include "Tokens/TokenTypes.h"

namespace AST {
	class Expression: public ASTNode {
	public:
		Expression(AstNodeType node_type) 
			: ASTNode(node_type) {}

		virtual ~Expression() = default;

		virtual std::string String() override
		{
			return "Expression node ";
		}

		void Accept(NodeVisitor* v) override { v->Visit(this); }

		Lex::TokenKeyword type_; // TODO: turn this into a type class.
	};
}

#endif