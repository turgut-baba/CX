#ifndef FUNCTION_H
#define FUNCTION_H

#include "ASTNode.h"
#include "Tokens/TokenTypes.h"
#include "AST/Statement.h"
#include "AST/Expressions/Identifier.h"
#include "AST/statements/VariableDeclaration.h"

namespace AST {
	class Function: public Statement {
	public:
		virtual ~Function() = default;

		Function(SlabAllocator* alloc) 
		{
			body_ = alloc->Allocate<Body>(alloc);
        	body_->SetOwner(this);
			node_type_ = AstNodeType::FUNCTION;
		}

		Function(AST::VariableDeclarator* decl, SlabAllocator* alloc)
			:ident_(decl->Ident())
		{
			body_ = alloc->Allocate<Body>(alloc);
        	body_->SetOwner(this);
			node_type_ = AstNodeType::FUNCTION;
		}

		virtual std::string String() override
		{
			return "Function node ";
		}

		SlabVector<VariableDeclarator*> Params()
		{
			return param_list_;
		}

		std::string Name()
		{
			return ident_->Value();
		}

		void Accept(NodeVisitor* v) override { v->Visit(this); }
	private:
		// TODO: give better name to these variables
		Lex::TokenKeyword ReturnType_; // TODO: turn this into a type class.
		SlabVector<VariableDeclarator*> param_list_;
		AST::Identifier* ident_;
	};
}

#endif