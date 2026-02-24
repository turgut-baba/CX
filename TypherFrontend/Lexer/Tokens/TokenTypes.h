#ifndef TOKEN_TYPES_H
#define TOKEN_TYPES_H
#include <cstdint>

namespace Lex {
	enum class TokenType : uint32_t {
		EOS,
		Identifier,
		Keyword,
		Punctuator,
		Operator,
		Literal,
		Comment,
		Whitespace
	};

	enum class TokenKeyword : uint32_t {
		NOT_A_KEYWORD = 0, // If it's not a keyword
		IF,
		ELSE,
		FOR,
		WHILE,
		ENUM,
		CONST,
		VOID,
		BOOL,
		CHAR,
		SHORT,
		INT,
		LONG,
		FLOAT,
		DOUBLE,
		BREAK,
		CLASS,
		CONTINUE,
		SWITCH,
		CASE,
		DEFAULT,
		PACKAGE,
		INCLUDE,
		NAMESPACE,
		RETURN,
		STATIC,
		THIS,
		NULLPTR
	};

	enum class TokenPunctuator : uint32_t {
		NOT_A_PUNCTUATOR = 0, // If it's not a keyword
		COMMA,
		LEFT_CURLY_BRACE,
		RIGHT_CURLY_BRACE,
		LEFT_SQUARE_BRACKETS,
		RIGHT_SQUARE_BRACKETS,
		SEMICOLON,
		DOT,
		AT, // TODO: can be an operator later on
		LEFT_PARENTHESES,// )
		RIGHT_PARENTHESES, // (
		ARROW,
		SLASH,
		ELLIPSIS, // ...
		HASH, // #
		POINTER_ASTERISK, // *
		REFERANCE_AND, // &
		RIGHT_ANGLE_BRACKET, // >
		LEFT_ANGLE_BRACKET // <
	};

	enum class TokenOperator : uint32_t {
		NOT_AN_OPERATOR = 0, // If it's not a keyword
		ADD, // +
		SUBTRACT, // -
		MULTIPLY, // *
		DIVIDE, // '/'
		MODULUS, // %
		INCREMENT, // ++
		DECREMENT, // --
		ASSIGNMENT, // =
		EQUALS, // ==
		NOT_EQUALS, // !=
		GREATER, // >
		LESSER, // <
		GREAT_OR_EQUAL, // >=
		LESS_OR_EQUAL, // <=
		LOGICAL_AND, // &&
		LOGICAL_OR, // ||
		LOGICAL_NOT, // !
		ADD_COMPOUND_ASSIGNMENT, // +=
		SUB_COMPOUND_ASSIGNMENT, // -=
		MUL_COMPOUND_ASSIGNMENT, // *=
		DIV_COMPOUND_ASSIGNMENT, // /=
		BITWISE_AND, // &
		BITWISE_OR,  // |
		BITWISE_XOR, // ^ 
		BITWISE_NOT, // ~
		LEFT_SHIFT, // <<
		RIGHT_SHIFT, // >>
	};

	enum class TokenLiteral : char32_t {
		NOT_A_LITERAL = 0,
		STRING,
		CHARACTER,
		ESCAPE_SEQUENCE,
		BOOLEAN,
		DECIMAL, 
		HEX, // starts with 0x
		BINARY, // starts with 0b
		OCTAL, // starts with 0
		FLOATING_PONINT,
		SCIENTIFIC, // ex: 6.022e23
	};
}

#endif