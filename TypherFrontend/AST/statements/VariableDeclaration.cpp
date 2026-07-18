#include "VariableDeclaration.h"

namespace AST {
	VariableDeclaration::VariableDeclaration(SlabVector<VariableDeclarator*> declarators, AstBuiltinTypes type)
		:declarators_(declarators), type_(type)
	{
		if (!declarators.empty()) {
			for (int i = 0; i < declarators.size(); i++) {
				declarators[i]->SetParent(this);
			}
		}
	}

	VariableDeclaration::VariableDeclaration(VariableDeclarator* declarator, 
			SlabAllocator* alloc, AstBuiltinTypes type)
			: declarators_(alloc), type_(type)
		{
			declarator->SetParent(this);
			declarators_.push_back(declarator);
		}
}