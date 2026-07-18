#ifndef STATEMENT_H
#define STATEMENT_H

#include "ASTNode.h"
#include "Body.h"

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
		
		void SetBody(Body* body)
        {
            body_ = body;
        }

		std::string String() override
		{
			return "Statement node ";
		}

		void Accept(NodeVisitor* v) override { v->Visit(this); }
	protected:
		virtual ~Statement() = default;

		Body* body_ = nullptr;
	};
}

#endif