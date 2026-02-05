#include "TypherDialect.h"

using namespace mlir;

// #include "Dialect/Dialect.cpp.inc"

//===----------------------------------------------------------------------===//
// ToyDialect
//===----------------------------------------------------------------------===//

/// Dialect initialization, the instance will be owned by the context. This is
/// the point of registration of types and operations for the dialect.
namespace MLIR {
  void TypherDialect::initialize() {
    addOperations<
    #define GET_OP_LIST
    #include "Ops.cpp.inc"
    >();
}

}