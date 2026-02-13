#include "VariableDeclaration.h"

namespace AST {
	VariableDeclaration::VariableDeclaration(SlabVector<VariableDeclarator*> declarators)
		:declarators_(declarators)
	{
		if (!declarators.empty()) {
			for (int i = 0; i < declarators.size(); i++) {
				declarators[i]->SetParent(this);
			}
		}
	}
}