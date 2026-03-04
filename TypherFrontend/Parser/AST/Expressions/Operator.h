#ifndef OPERATOR_H
#define OPERATOR_H

#include "AST/ASTNode.h"
#include "Tokens/TokenTypes.h"
#include "AST/Expression.h"

namespace AST {

	enum OperatorKind {
		ADD, SUB, MUL, DIV, MOD,
		ASN,
		EQS, LEQ, LES, NEQ, GEQ, GRT
	};

	class Operator : public Expression {
	public:
		Operator() 
			:Expression(AstNodeType::OPERATOR) {}
		 
		Operator(Lex::TokenOperator op);

		void SetLHS(AST::ASTNode* lhs);
		void SetRHS(AST::ASTNode* rhs);

		AST::ASTNode* GetLHS();
		AST::ASTNode* GetRHS();

		virtual ~Operator() = default;

		std::string String() override
		{
			return "Operator node";
		}
		
		OperatorKind OperatorType()
		{
			return OpType;
		}

		void Accept(NodeVisitor* v) override { v->Visit(this); }
	private:
		OperatorKind OpType;
		Lex::TokenOperator operator_; // TODO: Put this in an enum.
		AST::ASTNode* lhs_;
		AST::ASTNode* rhs_;
	};
}

#endif