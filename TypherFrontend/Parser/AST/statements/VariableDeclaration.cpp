#include "VariableDeclaration.h"

namespace AST {
	VariableDeclaration::VariableDeclaration(VariableDeclarator* declarators) 
	{
		for (int i = 0; i < sizeof(declarators); i++) {
			declarators[i].SetParent(this);
		}
	}
}