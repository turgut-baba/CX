#ifndef MEMORY_OPERATION_H
#define MEMORY_OPERATION_H

#include "ASTNode.h"
#include "Tokens/TokenTypes.h"
#include "Expression.h"

namespace AST {
	class MemoryOperation : public Expression {
	public:
		MemoryOperation() 
			:Expression(AstNodeType::MEMORY_OPERATION) {}

        MemoryOperation(AST::Expression* expr, std::vector<AST::Expression*> indices, unsigned int array_depth = 0);
		MemoryOperation(AST::Expression* expr, unsigned int de_ref_depth = 0, unsigned int address_depth = 0);

		void SetExpression(AST::Expression* expr) 
        {
            expr_ = expr;
            expr->SetParent(this);
        }

		AST::Expression* GetExpression() 
        {
            return expr_;
        }

        std::vector<AST::Expression*> ArrayIndices() const { return array_indices_; }
        
		std::string String() override
		{
            return "MemoryOperation node";
		}

        unsigned int DeRefDepth() const { return de_ref_depth_; }
        unsigned int AddressDepth() const { return address_depth_; }
        unsigned int ArrayDepth() const { return array_depth_; }
        
		void Accept(NodeVisitor* v) override { v->Visit(this); }
        virtual ~MemoryOperation() = default;
	private:
		unsigned int de_ref_depth_ = 0;
        unsigned int address_depth_ = 0;
        unsigned int array_depth_ = 0;

        std::vector<AST::Expression*> array_indices_;

		AST::Expression* expr_;
	};
}

#endif