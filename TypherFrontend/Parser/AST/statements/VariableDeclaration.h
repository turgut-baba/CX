#ifndef VARIABLE_DECLARATION_H
#define VARIABLE_DECLARATION_H

#include "AST/Expressions/VariableDeclarator.h"
#include "ParserState.h"

namespace AST {
	class VariableDeclaration : public Statement {
	public:
		VariableDeclaration(SlabVector<VariableDeclarator*> decls, AstBuiltinTypes type);

		VariableDeclaration(VariableDeclarator* declarator, 
			SlabAllocator* alloc, AstBuiltinTypes type);

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

		AstBuiltinTypes Type()
		{
			return type_;
		}

		void Accept(NodeVisitor* v) override { v->Visit(this); }
	private:
		AstBuiltinTypes type_; // TODO: this can be any type.
		SlabVector<VariableDeclarator*> declarators_;
		bool is_definition = true;
	};
}

#endif