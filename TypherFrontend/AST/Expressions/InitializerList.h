#ifndef INITIALIZER_LIST_H
#define INITIALIZER_LIST_H

#include "ASTNode.h"
#include "Tokens/TokenTypes.h"
#include "Expression.h"

namespace AST {
	class InitializerList : public Expression {
	public:
		InitializerList() 
			:Expression(AstNodeType::INITIALIZER_LIST) {}

        InitializerList(SlabVector<Expression*> elements);

        void AddMember(AST::Expression* member) 
        {
            elements_.push_back(member);
        }

        SlabVector<Expression*> GetElements() const 
        {
            return elements_;
        }

        void Accept(NodeVisitor* v) override { v->Visit(this); }
        virtual ~InitializerList() = default;
	private:
		SlabVector<Expression*> elements_;
	};
}

#endif