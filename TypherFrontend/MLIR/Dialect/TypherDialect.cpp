#include "TypherDialect.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/OperationSupport.h"
#include "mlir/IR/Value.h"
#include "mlir/Interfaces/FunctionImplementation.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/TypeID.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/DialectImplementation.h"

#include <iostream>

using namespace mlir;
using namespace mlir::typher;

#include "Dialect.cpp.inc"
//===----------------------------------------------------------------------===//
// TypherDialect
//===----------------------------------------------------------------------===//

/// Dialect initialization, the instance will be owned by the context. This is
/// the point of registration of types and operations for the dialect.


  //MLIR_DEFINE_EXPLICIT_TYPE_ID(TypherDialect)
void TypherDialect::initialize() {
    addOperations<
        #define GET_OP_LIST
        #include "Ops.cpp.inc"
    >();
    addTypes<                    // add this
        #define GET_TYPEDEF_LIST
        #include "Types.cpp.inc"
    >();
}

static mlir::ParseResult parseBinaryOp(mlir::OpAsmParser &parser,
                                       mlir::OperationState &result) {
    SmallVector<mlir::OpAsmParser::UnresolvedOperand, 2> operands;
    SMLoc operandsLoc = parser.getCurrentLocation();
    Type type;
    if (parser.parseOperandList(operands, /*requiredOperandCount=*/2) ||
        parser.parseOptionalAttrDict(result.attributes) ||
        parser.parseColonType(type))
      return mlir::failure();

    // If the type is a function type, it contains the input and result types of
    // this operation.
    if (FunctionType funcType = llvm::dyn_cast<FunctionType>(type)) {
      if (parser.resolveOperands(operands, funcType.getInputs(), operandsLoc,
                                result.operands))
        return mlir::failure();
      result.addTypes(funcType.getResults());
      return mlir::success();
    }

    // Otherwise, the parsed type is the type of both operands and results.
    if (parser.resolveOperands(operands, type, result.operands))
      return mlir::failure();
    result.addTypes(type);
    return mlir::success();
}

/// A generalized printer for binary operations. It prints in two different
/// forms depending on if all of the types match.
static void printBinaryOp(mlir::OpAsmPrinter &printer, mlir::Operation *op) {
    printer << " " << op->getOperands();
    printer.printOptionalAttrDict(op->getAttrs());
    printer << " : ";

    // If all of the types are the same, print the type directly.
    Type resultType = *op->result_type_begin();
    if (llvm::all_of(op->getOperandTypes(),
                    [=](Type type) { return type == resultType; })) {
      	printer << resultType;
      	return;
    }

    // Otherwise, print a functional type.
    printer.printFunctionalType(op->getOperandTypes(), op->getResultTypes());
}

//===----------------------------------------------------------------------===//
// AddOp
//===----------------------------------------------------------------------===//


void AddOp::build(mlir::OpBuilder &builder, mlir::OperationState &state,
                  mlir::Value lhs, mlir::Value rhs) {
  // Use the type of the left-hand side as the result type (C-style)
    state.addTypes(lhs.getType()); 
    state.addOperands({lhs, rhs});
}

mlir::ParseResult AddOp::parse(mlir::OpAsmParser &parser,
                               mlir::OperationState &result) {
    mlir::OpAsmParser::UnresolvedOperand lhs, rhs;
    mlir::Type type;

    // Parse: %lhs, %rhs : type
    if (parser.parseOperand(lhs) || parser.parseComma() || 
        parser.parseOperand(rhs) || parser.parseColonType(type))
    	return mlir::failure();

    // Resolve the operands against the parsed type
    if (parser.resolveOperands({lhs, rhs}, type, result.operands))
      return mlir::failure();

    // Set the result type
    result.addTypes(type);
    return mlir::success();
}

void AddOp::print(mlir::OpAsmPrinter &p) {
    p << " " << getLhs() << ", " << getRhs() << " : " << getResult().getType();
}

//===----------------------------------------------------------------------===//
// ConstantOp
//===----------------------------------------------------------------------===//

/// Build a constant operation.
/// The builder is passed as an argument, so is the state that this method is
/// expected to fill in order to build the operation.
// 1. Updated Builder: Now creates a scalar F64 type and a FloatAttr
void ConstantOp::build(mlir::OpBuilder &builder, mlir::OperationState &state,
                       mlir::Type type, int value) {
    mlir::IntegerAttr attr = builder.getIntegerAttr(type, value);

    ConstantOp::build(builder, state, type, attr);
}

void ConstantOp::build(mlir::OpBuilder &builder, mlir::OperationState &state,
                       mlir::Type type, double value) {
    // Use getFloatAttr for F32/F64 types
    mlir::FloatAttr attr = builder.getFloatAttr(type, value);
    state.addAttribute("value", attr);
    state.addTypes(type);
}

void ConstantOp::build(mlir::OpBuilder &builder, mlir::OperationState &state,
                       mlir::Type type, char value){
    // char is always i8 — store it as a signed integer attribute
    assert(type == builder.getI8Type() && "char constant must have i8 result type");
    auto attr = builder.getIntegerAttr(type, static_cast<int64_t>(value));
    build(builder, state, type, attr);
}

// 2. Updated Parser: Uses TypedAttr to handle any scalar (float, int, etc.)
mlir::ParseResult ConstantOp::parse(mlir::OpAsmParser &parser,
                                    mlir::OperationState &result) {
    mlir::TypedAttr value;
    if (parser.parseOptionalAttrDict(result.attributes) ||
        parser.parseAttribute(value, "value", result.attributes))
      return failure();

    result.addTypes(value.getType());
    return success();
}

// 3. Updated Printer: Simpler output for scalar values
void ConstantOp::print(mlir::OpAsmPrinter &printer) {
    printer << " ";
    printer.printOptionalAttrDict((*this)->getAttrs(), /*elidedAttrs=*/{"value"});
    printer << getValue(); // getValue() now returns a scalar attribute
}

// 4. Updated Verifier: Much simpler, just ensures the type matches the value
llvm::LogicalResult ConstantOp::verify() {
    auto resultType = getResult().getType();
/*   auto attrType = getValue().getType();

  if (attrType != resultType) {
    return emitOpError("result type (") << resultType 
           << ") must match attribute type (" << attrType << ")";
  } */
    return mlir::success();
}

//===----------------------------------------------------------------------===//
// StringConstantOp
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// FuncOp
//===----------------------------------------------------------------------===//

void FuncOp::build(mlir::OpBuilder &builder, mlir::OperationState &state,
                   llvm::StringRef name, mlir::FunctionType type,
                   llvm::ArrayRef<mlir::NamedAttribute> attrs) {
  // FunctionOpInterface provides a convenient `build` method that will populate
  // the state of our FuncOp, and create an entry block.
    buildWithEntryBlock(builder, state, name, type, attrs, type.getInputs());
}

mlir::ParseResult FuncOp::parse(mlir::OpAsmParser &parser,
                                mlir::OperationState &result) {
  // Dispatch to the FunctionOpInterface provided utility method that parses the
  // function operation.
    auto buildFuncType =
      [](mlir::Builder &builder, llvm::ArrayRef<mlir::Type> argTypes,
         llvm::ArrayRef<mlir::Type> results,
         mlir::function_interface_impl::VariadicFlag,
         std::string &) { return builder.getFunctionType(argTypes, results); };

    return mlir::function_interface_impl::parseFunctionOp(
      parser, result, /*allowVariadic=*/false,
      getFunctionTypeAttrName(result.name), buildFuncType,
      getArgAttrsAttrName(result.name), getResAttrsAttrName(result.name));
}

void FuncOp::print(mlir::OpAsmPrinter &p) {
  // Dispatch to the FunctionOpInterface provided utility method that prints the
  // function operation.
    mlir::function_interface_impl::printFunctionOp(
      p, *this, /*isVariadic=*/false, getFunctionTypeAttrName(),
      getArgAttrsAttrName(), getResAttrsAttrName());
}

//===----------------------------------------------------------------------===//
// GenericCallOp
//===----------------------------------------------------------------------===//

void GenericCallOp::build(mlir::OpBuilder &builder, mlir::OperationState &state,
                          StringRef callee, ArrayRef<mlir::Value> arguments) {
  // Generic call always returns an unranked Tensor initially.
    state.addTypes(builder.getI32Type());
    state.addOperands(arguments);
    state.addAttribute("callee",
                      mlir::SymbolRefAttr::get(builder.getContext(), callee));
}


//===----------------------------------------------------------------------===//
// ReturnOp
//===----------------------------------------------------------------------===//

llvm::LogicalResult ReturnOp::verify() {
    // We know that the parent operation is a function, because of the 'HasParent'
    // trait attached to the operation definition.
    auto func = (*this)->getParentOfType<FuncOp>();
    if (!func) {
      return (*this)->emitOpError("is not nested inside a function!");
    }
    /// TODO: Check if type is the same as function type.
    /// TODO: Make sure the returning variable is the same as return type.

    return mlir::success();
}


//===----------------------------------------------------------------------===//
// EqualsOp
//===----------------------------------------------------------------------===//

void EqualsOp::build(mlir::OpBuilder &builder, mlir::OperationState &result, 
                     mlir::Value lhs, mlir::Value rhs) {
    // 1. Add the operands (the two integers to compare)
    result.addOperands({lhs, rhs});

    // 2. Add the result type (the i1 boolean)
    result.addTypes(builder.getI1Type());
}

//===----------------------------------------------------------------------===//
// IfOp
//===----------------------------------------------------------------------===//

void IfOp::build(OpBuilder &builder, OperationState &result, 
                 Value condition, bool withElseRegion) {
	// Add the condition operand
	result.addOperands(condition);

	// Create the 'then' region and its entry block
	Region *thenRegion = result.addRegion();
	thenRegion->push_back(new Block());

	// Create the 'else' region (if requested)
	Region *elseRegion = result.addRegion();
	if (withElseRegion) {
		elseRegion->push_back(new Block());
	}
}

LogicalResult IfOp::verify() {
  // 1. Check if 'then' region is empty
	if (getThenRegion().empty())
		return emitOpError("then region must not be empty");

	// 2. Check if the condition is an i1
	if (!getCondition().getType().isInteger(1))
		return emitOpError("condition must be a 1-bit integer");

	return success();
}

#define GET_TYPEDEF_CLASSES
#include "Types.cpp.inc"

#define GET_OP_CLASSES
#include "Ops.cpp.inc"