#ifndef MLIR_BUILDER_H
#define MLIR_BUILDER_H

#include "Parser.h" // TODO: change this to checker/analyzer.
#include "Types/MLIRIntegerType.h"

namespace MLIR {
	class Builder {
	public:
		Builder();

		void BuildModule();

		IntegerType GetI32Type()
		{
			return IntegerType(32);
		}
	};
}

#endif