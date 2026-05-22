#ifndef STRING_LITERAL_H
#define STRING_LITERAL_H

#include "Literal.h"

union StringType {
	char chr;
	char* str;
};

namespace AST {
	class StringLiteral: public Literal {
	public:
		StringLiteral(char value) {
			value_.chr = value;
			isChar = true;
		}

		StringLiteral(char* value) {
			value_.str = value;
			isChar = false;
		}
		
		template <typename RetType>
		RetType Value()
		{
			if constexpr (std::is_same_v<RetType, char>) {
				return value_.chr;
			} else if constexpr (std::is_same_v<RetType, std::string_view>) {
				return value_.str;
			} else {
				static_assert(false, "Value<T>: unsupported type T");
			}
		}

		std::string String() override
		{
			if (isChar) {
				return ("Literal Node: " + std::to_string(value_.chr));
			} else {
				return ("Literal Node: " + *value_.str);
			}
		}

		bool IsChar()
		{
			return isChar;
		}

		void Accept(NodeVisitor* v) override { v->Visit(this); }
	private:
		bool isChar;
		StringType value_; // TODO: Change this to support different types
	};
}

#endif