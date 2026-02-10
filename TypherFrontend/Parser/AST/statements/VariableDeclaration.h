#ifndef VARIABLE_DECLARATION_H
#define VARIABLE_DECLARATION_H

#include "VariableDeclarator.h"

namespace AST {
	class VariableDeclaration : public Statement {
	public:
		VariableDeclaration(ArrayAlloc<VariableDeclarator*> decls);

		VariableDeclaration(VariableDeclarator* declarator) 
		{
			declarator->SetParent(this);
			declarators_.push_back(declarator);
		}

		VariableDeclaration() = default;
		virtual ~VariableDeclaration() = default;

		std::string String() override
		{
			return "DECLARATION node";
		}

		ArrayAlloc<VariableDeclarator*> Declarators()
		{
			return declarators_;
		}

		void Accept(NodeVisitor* v) override { v->Visit(this); }
	private:
		ArrayAlloc<VariableDeclarator*> declarators_;
		bool is_definition = true;
	};
}

#endif