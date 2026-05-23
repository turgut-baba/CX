#ifndef VARIABLE_DECLARATOR_H
#define VARIABLE_DECLARATOR_H

#include "AST/Statement.h"
#include "AST/Expression.h"
#include "AST/Expressions/Identifier.h"
#include "AST/Visitor.h"

namespace AST {
	
	enum DeclaratorKind { Pointer, Array, Reference };

	class VariableDeclarator : public Expression {
	public:
		VariableDeclarator()
			: Expression(AstNodeType::VARIABLE_DECLARATOR) {}

		VariableDeclarator(Expression* expr, Identifier* ident, std::vector<DeclaratorKind> modifiers);

		VariableDeclarator(Identifier* ident, std::vector<DeclaratorKind> modifiers);

		virtual ~VariableDeclarator() = default;

		std::string Name()
		{
			return ident_->Value();
		}

		Identifier* Ident()
		{
			return ident_;
		}
		
		Expression* Expr() const
		{
			return expression;
		}
		
		std::string String() override
		{
			return "DECLARATOR node";
		}

		int GetPointerDepth();
		
		void Accept(NodeVisitor* v) override { v->Visit(this); }
	private:
		std::vector<DeclaratorKind> modifiers_;

		int arraySize = 0;

		Identifier* ident_;
		Expression* expression;
	};
}

#endif