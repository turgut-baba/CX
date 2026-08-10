#ifndef INTEGER_LITERAL_H
#define INTEGER_LITERAL_H

#include "Literal.h"

union NumericTypes {
	long double floating;
	long long decimal;
};

namespace AST {
	class IntegerLiteral: public Literal {
	public:
		IntegerLiteral(double value) {
			value_.floating = value;
			isFloating = true;
		}

		IntegerLiteral(int value) {
			value_.decimal = value;
			isFloating = false;
		}
		
		template <typename RetType>
		RetType Value()
		{
			if (isFloating) {
				return value_.floating;
			} else {
				return value_.decimal;
			}
		}

		std::string String() override
		{
			if (isFloating) {
				return ("Literal Node: " + std::to_string(value_.floating));
			} else {
				return ("Literal Node: " + std::to_string(value_.decimal));
			}
		}

		bool IsFloating()
		{
			return isFloating;
		}

		virtual bool IsIntegerLiteral() override
		{
			return true;
		}

		void Accept(NodeVisitor* v) override { v->Visit(this); }
	private:
		bool isFloating;
		NumericTypes value_; // TODO: Change this to support different types
	};
}

#endif