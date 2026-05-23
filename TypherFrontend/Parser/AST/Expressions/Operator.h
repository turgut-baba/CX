#ifndef OPERATOR_H
#define OPERATOR_H

#include "AST/ASTNode.h"
#include "Tokens/TokenTypes.h"
#include "AST/Expression.h"

namespace AST {

	enum OperatorKind {
		ADD, SUB, MUL, DIV, MOD,
		ASN,
		EQS, LEQ, LES, NEQ, GEQ, GRT,
		DRF, ADO
	};

	class Operator : public Expression {
	public:
		Operator() 
			:Expression(AstNodeType::OPERATOR) {}
		 
		Operator(Lex::TokenOperator op);
		Operator(int de_ref_depth);

		void SetLHS(AST::Expression* lhs);
		void SetRHS(AST::Expression* rhs);

		AST::Expression* GetLHS();
		AST::Expression* GetRHS();

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
		int de_ref_depth_ = 0;
		OperatorKind OpType;
		Lex::TokenOperator operator_; // TODO: Put this in an enum.
		AST::Expression* lhs_;
		AST::Expression* rhs_;
	};
}

#endif