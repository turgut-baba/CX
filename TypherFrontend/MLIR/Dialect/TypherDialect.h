#ifndef TYPHER_DIALECT_H
#define TYPHER_DIALECT_H

#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/IR/Dialect.h"
#include "mlir/Interfaces/DataLayoutInterfaces.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Interfaces/CallInterfaces.h"
#include "mlir/Interfaces/FunctionInterfaces.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"
#include "llvm/ADT/TypeSwitch.h"
#include "mlir/Support/InterfaceSupport.h" 
#include "Dialect.h.inc"

#define GET_TYPEDEF_CLASSES
#include "Types.h.inc"

#define GET_OP_CLASSES
#include "Ops.h.inc"

#endif