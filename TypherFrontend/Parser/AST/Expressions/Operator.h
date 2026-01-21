#ifndef OPERATOR_H
#define OPERATOR_H

#include "AST/ASTNode.h"
#include "Tokens/TokenTypes.h"
#include "AST/Expression.h"

namespace AST {
	class Operator : public Expression {
	public:
		Operator() = default;
		 
		Operator(Lex::TokenOperator op);

		void SetLHS(AST::ASTNode* lhs);
		void SetRHS(AST::ASTNode* rhs);

		virtual ~Operator() = default;

		std::string String() override
		{
			return "Operator node";
		}
	private:
		Lex::TokenOperator operator_;
		AST::ASTNode* lhs_;
		AST::ASTNode* rhs_;
	};
}

#endif