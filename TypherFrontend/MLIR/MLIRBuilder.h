#ifndef MLIR_BUILDER_H
#define MLIR_BUILDER_H

#include "MLIRGen.h"

#include <iostream>

namespace MLIR {
	class Builder {
	public:
		Builder() {}

		void BuildModule(SlabVector<AST::Statement*>& ASTTree);
	private:
		Generator gen_;
	};
}

#endif
