#ifndef STATEMENT_H
#define STATEMENT_H

#include "ASTNode.h"
#include "AST/Body.h"

namespace AST {
	class Statement : public ASTNode {
	public:
		Body* GetBody() 
        {
			if(body_ == nullptr)
			{
				UNREACHABLE("Trying to acces body of a statement that has no body.");
			}
            return body_;
        }
		void Accept(NodeVisitor* v) override { v->Visit(this); }
	protected:
		virtual ~Statement() = default;

		std::string String() override
		{
			return "Statement node ";
		}

		Body* body_ = nullptr;
	};
}

#endif