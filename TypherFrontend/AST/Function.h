#ifndef FUNCTION_H
#define FUNCTION_H

#include "ASTNode.h"
#include "Tokens/TokenTypes.h"
#include "Statement.h"
#include "Expressions/Identifier.h"
#include "statements/VariableDeclaration.h"

namespace AST {

	// This is a wrapper class to manage the parameters.
	// We can change it to a seperate class unrelated to VariableDeclaration.
	class Parameter {
	public:
		Parameter(AST::VariableDeclaration* decl)
			:decl_(decl) {}

		AstBuiltinTypes Type()
		{
			return decl_->Type();
		}

		std::string Name()
		{
			return decl_->Declarators()[0]->Name();
		}

		AST::VariableDeclarator* AsDeclarator() 
		{
			return decl_->Declarators()[0];
		}
	private:
		AST::VariableDeclaration* decl_;
	};
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
			:ident_(decl->Ident()), declaraton_(decl)
		{
			body_ = alloc->Allocate<Body>(alloc);
        	body_->SetOwner(this);
			node_type_ = AstNodeType::FUNCTION;
		}

		virtual std::string String() override
		{
			return "Function node ";
		}

		std::vector<Parameter> Params()
		{
			return param_list_;
		}

		std::string Name()
		{
			return ident_->Value();
		}

		AstBuiltinTypes ReturnType()
		{
			return ReturnType_;
		}

		void SetReturnType(AstBuiltinTypes return_type)
		{
			ReturnType_ = return_type;
		}

		AST::VariableDeclarator* Declarator()
		{
			return declaraton_;
		}

		void AddParameter(AST::VariableDeclaration* decl)
		{
			param_list_.push_back(decl);
		}

		void Accept(NodeVisitor* v) override { v->Visit(this); }
	private:
		AST::VariableDeclarator* declaraton_;
		// TODO: give better name to these variables
		AstBuiltinTypes ReturnType_;
		std::vector<Parameter> param_list_;
		AST::Identifier* ident_;
	};
}

#endif