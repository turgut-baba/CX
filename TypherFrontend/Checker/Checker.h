#ifndef CHECKER_H
#define CHECKER_H

#include "Parser.h"

namespace Checker {
	class Checker {
	public:
		Checker();
		void check_module(ArrayAlloc<AST::Statement*> ASTTree);

	};
}

#endif