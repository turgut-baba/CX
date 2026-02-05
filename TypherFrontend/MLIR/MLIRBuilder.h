#ifndef MLIR_BUILDER_H
#define MLIR_BUILDER_H

#include "Parser.h" // TODO: change this to checker/analyzer.
#include "Types/MLIRIntegerType.h"

#include "mlir/IR/BuiltinDialect.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

#include "mlir/IR/AsmState.h"
#include "Dialect/TypherDialect.h"

#include <iostream>

namespace MLIR {
	class Builder {
	public:
		Builder() {}

		void BuildModule();

		IntegerType GetI32Type()
		{
			return IntegerType(32);
		}
	};
}

#endif