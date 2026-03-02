#ifndef MLIR_BUILDER_H
#define MLIR_BUILDER_H

#include "MLIRGen.h"
#include "MLIREmitter.h"

#include <iostream>

namespace MLIR {
	class Builder {
	public:
		Builder() {}

		void BuildModule(SlabVector<AST::Statement*>& AST_tree);
	private:
		Generator gen_;
		Emitter emit_;
	};
}

#endif
