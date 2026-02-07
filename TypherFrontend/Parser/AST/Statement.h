#ifndef STATEMENT_H
#define STATEMENT_H

#include "ASTNode.h"

namespace AST {
	class Statement : public ASTNode {
	public:
		void Accept(NodeVisitor* v) override { v->Visit(this); }
	protected:
		virtual ~Statement() = default;

		std::string String() override
		{
			return "Statement node ";
		}
	};
}

#endif