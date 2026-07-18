#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "ASTNode.h"
#include "Tokens/TokenTypes.h"
#include "Memory/SlabAlloc.h"

namespace AST {
	
	class Expression: public ASTNode {
	public:
		Expression(AstNodeType node_type) 
			: ASTNode(node_type){}

		virtual ~Expression() = default;

		virtual std::string String() override
		{
			return "Expression node ";
		}

		bool Type() // TODO: TEMP
		{
			return type_;
		}

		void SetType(bool type) // TODO: TEMP
		{
			type_ = type;
		}

		void Accept(NodeVisitor* v) override { v->Visit(this); }

	private:
		bool type_; // TODO: turn this into a type class.
	};
}

#endif