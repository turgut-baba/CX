#ifndef FUNCTION_H
#define FUNCTION_H

#include "ASTNode.h"
#include "Tokens/TokenTypes.h"
#include "AST/Statement.h"
#include "AST/Identifier.h"
#include "AST/statements/VariableDeclaration.h"

namespace AST {
	class Function: public Statement {
	public:
		virtual ~Function() = default;

		Function() 
		{
			node_type_ = NodeType::FUNCTION;
		}

		Function(AST::Identifier* ident)
			:ident_(ident)
		{
			node_type_ = NodeType::FUNCTION;
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
		Lex::TokenKeyword ReturnType_; // TODO: turn this into a type class.
		SlabVector<VariableDeclarator*> param_list_;
		SlabVector<Statement*> statement_list_;
		AST::Identifier* ident_;
	};
}

#endif