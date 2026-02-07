#ifndef INTEGER_LITERAL_H
#define INTEGER_LITERAL_H

#include "Literal.h"

namespace AST {
	class IntegerLiteral: public Literal {
	public:
		IntegerLiteral(int value) : value_(value) {  }
		
		int Value()
		{
			return value_;
		}

		std::string String() override
		{
			return ("Literal Node: " + std::to_string(value_));
		}

		void Accept(NodeVisitor* v) override { v->Visit(this); }
	private:
		int value_; // TODO: Change this to support different types
	};
}

#endif