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
void ConstantOp::print(mlir::OpAsmPrinter &printer)
{
    printer << " ";
    printer.printOptionalAttrDict((*this)->getAttrs(), /*elidedAttrs=*/{"value"});
    printer << getValue(); // getValue() now returns a scalar attribute
}

// 4. Updated Verifier: Much simpler, just ensures the type matches the value
llvm::LogicalResult ConstantOp::verify()
{
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
// ForOp
//===----------------------------------------------------------------------===//

mlir::ParseResult ForOp::parse(mlir::OpAsmParser &parser,mlir::OperationState &result)
{
    auto &builder = parser.getBuilder();
    mlir::OpAsmParser::UnresolvedOperand ivArg;
    mlir::OpAsmParser::UnresolvedOperand lbOperand, ubOperand, stepOperand;
    mlir::Type type;

    // 1. Parse the loop header syntax: %i = %lb to %ub step %step
    if (parser.parseOperand(ivArg) || 
        parser.parseOperand(lbOperand) || 
        parser.parseKeyword("to") ||
        parser.parseOperand(ubOperand) || 
        parser.parseKeyword("step") ||
        parser.parseOperand(stepOperand)) {
        return mlir::failure();
    }

    // 2. Parse the trailing type specification (e.g., : i32)
    if (parser.parseColonType(type)) {
        return mlir::failure();
    }

    // 3. Resolve the three bounds operands into the operation state using that type
    if (parser.resolveOperand(lbOperand, type, result.operands) ||
        parser.resolveOperand(ubOperand, type, result.operands) ||
        parser.resolveOperand(stepOperand, type, result.operands)) {
        return mlir::failure();
    }

    // 4. Parse optional structural attributes if present (using standard attr-dict format)
    if (parser.parseOptionalAttrDict(result.attributes)) {
        return mlir::failure();
    }

    // 5. Add the region to the operation state and prep the induction variable block argument
    mlir::Region *bodyRegion = result.addRegion();
    // 6. Parse the curly-braced block region body
    if (parser.parseRegion(*bodyRegion, /*arguments=*/{}, /*argTypes=*/{})) {
        return mlir::failure();
    }

    return mlir::success();
}

void mlir::typher::ForOp::print(mlir::OpAsmPrinter &p) {
    // 1. Extract the induction variable from the entry block arguments
    mlir::Region &bodyRegion = getRegion();
    if (!bodyRegion.empty() && bodyRegion.front().getNumArguments() > 0) {
        // Prints the variable name (e.g., %i)
        p << " " << bodyRegion.front().getArgument(0) << " = ";
    } else {
        p << " <invalid_loop_variable> = ";
    }
    
    // 2. Print the operands: lower bound, upper bound, step
    p << getLowerBound() << " to " << getUpperBound() << " step " << getStep();
    
    // 3. Print the trailing type specification (using the lower bound's type)
    p << " : " << getLowerBound().getType();
    
    // 4. Print any structural attributes attached to the operation
    p.printOptionalAttrDict(getOperation()->getAttrs(), /*elidedAttrs=*/{});
    
    // 5. Print the curly-braced block region body
    // We pass `printEntryBlockArgs=false` because we already printed `%i` inline at the top
    p << " ";
    p.printRegion(bodyRegion, /*printEntryBlockArgs=*/false, /*printBlockTerminators=*/true);
}
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
                          mlir::Type returnType, StringRef callee, 
                          ArrayRef<mlir::Value> arguments) {
    // Instead of hardcoding builder.getI32Type(), use the passed-in type
    state.addTypes(returnType);
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