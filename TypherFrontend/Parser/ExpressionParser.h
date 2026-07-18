#ifndef EXPRESSION_PARSER_H
#define EXPRESSION_PARSER_H

#include "Parser.h"
#include "Expression.h"
#include "Expressions/CallExpression.h"
#include "Expressions/Operator.h"
#include "Literals/IntegerLiteral.h"
#include "Literals/StringLiteral.h"
#include "Literals/Literal.h"
#include "ParserState.h"

namespace Parser {
	class ExpressionParser : public Parser {
	public:
		ExpressionParser(std::shared_ptr<ParserState> state,
			DiagnosticEngine& diags, MemoryAllocator *allocator)
			: Parser(diags, allocator)
		{
			state_ = state;
		}

		AST::Expression* parse_expression();
		AST::Expression* parse_assignment();
		private:
		AST::Expression* CheckDeRefAndAddressOf();
		AST::Expression* CheckIdentifier();
		AST::Literal* CheckLiteral();
		AST::CallExpression* ParseFunctionCall(AST::Identifier* ident);
		AST::ASTNode* ParsePrimaryExpression();
		AST::Expression* ParseOperator(AST::Expression* lhs);
		AST::Expression* ParseSingleExpression();
	};
}

#endif

/*
int a = 1, b = 2;
int* p = &a;
bool flag = true;

 1. Primary expressions
a;
42;
true;
nullptr;
this;
(a);

2. Postfix expressions 
a++;
a--;
a(b);
p->~int();
typeid(a);
typeid(int);

3. Unary expressions
++a;
--a;
+a;
-a;
!flag;
~a;
*p;
&a;
sizeof(a);
sizeof(int);
alignof(int);
noexcept(a);

 4. New / delete expressions 
int* x = new int(5);
delete x;
int* arr = new int[10];
delete[] arr;

 5. Cast expressions 
(int)a;                       // C-style
static_cast<int>(b);
reinterpret_cast<long>(p);
const_cast<int&>(a);

 6. Multiplicative 
a* b;
a / b;
a% b;

 7. Additive 
a + b;
a - b;

 8. Shift 
a << b;
a >> b;

 9. Relational 
a < b;
a <= b;
a > b;
a >= b;

 10. Equality 
a == b;
a != b;

 11. Bitwise 
a& b;
a^ b;
a | b;

 12. Logical 
a&& b;
a || b;

 13. Conditional (ternary) 
a > b ? a : b;

 14. Assignment 
a = b;
a += b;
a -= b;
a *= b;
a /= b;
a %= b;
a <<= b;
a >>= b;
a &= b;
a ^= b;
a |= b;

 15. Comma 
a = 1, b = 2;

 16. Lambda expressions 
auto lam = [](int x) { return x + 1; };
lam(5);

 17. Requires expression (C++20) 
	requires { a + b; };

 18. Fold expression (C++17) 
auto sum = [](auto... xs) { return (xs + ...); };
sum(1, 2, 3);

 19. Co_await expression (C++20 coroutines) 
// co_await some_awaitable;


*/