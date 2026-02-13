#ifndef VARIABLE_DECLARATION_H
#define VARIABLE_DECLARATION_H

#include "VariableDeclarator.h"

namespace AST {
	class VariableDeclaration : public Statement {
	public:
		VariableDeclaration(SlabVector<VariableDeclarator*> decls);

		VariableDeclaration(VariableDeclarator* declarator)
			:declarators_()
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

		SlabVector<VariableDeclarator*> Declarators()
		{
			return declarators_;
		}

		void Accept(NodeVisitor* v) override { v->Visit(this); }
	private:
		SlabVector<VariableDeclarator*> declarators_;
		bool is_definition = true;
	};
}

#endif