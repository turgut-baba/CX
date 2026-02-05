#ifndef TYPHER_DIALECT_H
#define TYPHER_DIALECT_H

#include "mlir/IR/BuiltinDialect.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

//#include "Dialect/Dialect.h.inc"

/// Include the auto-generated header file containing the declarations of the
/// toy operations.
#define GET_OP_CLASSES
#include "Ops.h.inc"

namespace MLIR{
    class TypherDialect : public mlir::Dialect {
    public:
        explicit TypherDialect(mlir::MLIRContext *ctx);

        /// Provide a utility accessor to the dialect namespace.
        static llvm::StringRef getDialectNamespace() { return "typher"; }

        /// An initializer called from the constructor of ToyDialect that is used to
        /// register attributes, operations, types, and more within the Toy dialect.
        void initialize();
    };
}

#endif