
#ifndef TYPHER_OPS
#define TYPHER_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/FunctionInterfaces.td"
include "mlir/IR/SymbolInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

// Provide a definition of the 'toy' dialect in the ODS framework so that we
// can define our operations.
def Typher_Dialect : Dialect {
  let name = "typher";
  let cppNamespace = "::mlir::typher";
  let summary = "A dialect for Typher langauge that uses C-language constructs.";
}

def C_Type : AnyTypeOf<[I1, I8, I16, I32, I64, F32, F64]>;

// Base class for toy dialect operations. This operation inherits from the base
// `Op` class in OpBase.td, and provides:
//   * The parent dialect of the operation.
//   * The mnemonic for the operation, or the name without the dialect prefix.
//   * A list of traits for the operation.
class Typher_Op<string mnemonic, list<Trait> traits = []> :
    Op<Typher_Dialect, mnemonic, traits>;

//===----------------------------------------------------------------------===//
// Typher Operations
//===----------------------------------------------------------------------===//

def CastOp : Typher_Op<"cast"> {
    let summary = "Cast between C types";
    let arguments = (ins C_Type:$input);
    let results = (outs C_Type:$output);
}

//===----------------------------------------------------------------------===//
// ConstantOp
//===----------------------------------------------------------------------===//

// We define a toy operation by inheriting from our base 'Toy_Op' class above.
// Here we provide the mnemonic and a list of traits for the operation. The
// constant operation is marked as 'Pure' as it is a pure operation
// and may be removed if dead.
def ConstantOp : Typher_Op<"constant", [Pure]> {
  let summary = "scalar constant operation";
  let description = [{
    The "constant" operation creates a scalar SSA value from a literal attribute.
    This mimics C literals like '42' or '3.14'.
    
    Example:
    ```mlir
      %0 = typher.constant 1.0 : f64
      %1 = typher.constant 42 : i32
    ```
  }];

  // 1. Changed from AnyType to AnyAttr (Attributes store the actual data)
  let arguments = (ins AnyAttr:$value);

  // 2. Changed result to be a scalar integer or float
  let results = (outs C_Type:$result);

  let hasCustomAssemblyFormat = 1;

  let builders = [
    OpBuilder<(ins "mlir::Type":$type, "int":$value)>,
    OpBuilder<(ins "mlir::Type":$type, "double":$value)>
  ];

  let hasVerifier = 1;
}


//===----------------------------------------------------------------------===//
// AddOp
//===----------------------------------------------------------------------===//

def AddOp : Typher_Op<"add"> {
  let summary = "scalar addition operation";
  let description = [{
    The "add" operation performs addition between two scalar values.
    It supports standard C types including integers and floating-point values.
    Both operands and the result must share the same type.
  }];

  // Define a constraint for C-like scalar types
  let arguments = (ins 
    AnyTypeOf<[AnyInteger, AnyFloat]>:$lhs, 
    AnyTypeOf<[AnyInteger, AnyFloat]>:$rhs
  );

  let results = (outs 
    AnyTypeOf<[AnyInteger, AnyFloat]>:$result
  );

  let hasCustomAssemblyFormat = 1;

  let builders = [
    OpBuilder<(ins "Value":$lhs, "Value":$rhs)>
  ];
}


//===----------------------------------------------------------------------===//
// AssignOp
//===----------------------------------------------------------------------===//

def AssignOp : Typher_Op<"assign"> {
    let summary = "C-style assignment (store) operation";
    let description = [{
        Assigns a value to a memory location.
        Example: `mylang.assign %val, %ptr : i32, memref<i32>`
    }];

    // Arguments: The value to store and the destination address
    let arguments = (ins 
        AnyType:$value, 
        Arg<AnyMemRef, "the address to write to", [MemWrite]>:$addr
    );
    // C assignments usually don't return a value in the IR, 
    // though the C expression 'a = 5' technically returns 5.
    let results = (outs);

    // Custom assembly: mylang.assign %5, %addr : i32, memref<i32>
    let assemblyFormat = "$value `,` $addr attr-dict `:` type($value) `,` type($addr)";
}


//===----------------------------------------------------------------------===//
// AllocaOp
//===----------------------------------------------------------------------===//


def AllocaOp : Typher_Op<"alloca"> {
    let summary = "Stack allocation for a local variable";
    let description = [{
        Allocates memory on the stack for a single scalar value.
        Example: `%ptr = mylang.alloca : memref<i32>`
    }];

    // No input arguments needed for a simple scalar allocation
    let arguments = (ins);

    // Returns a memory reference (the address of the variable)
    let results = (outs AnyMemRef:$addr);

    // Custom assembly: mylang.alloca : memref<i32>
    let assemblyFormat = "attr-dict `:` type($addr)";
}

//===----------------------------------------------------------------------===//
// LoadOp
//===----------------------------------------------------------------------===//


def LoadOp : Typher_Op<"load"> {
    let summary = "Read a value from memory (dereference)";
    let description = [{
        Loads a scalar value from a memory location (memref).
        This is used to convert an L-value (address) into an R-value (data).
        Example: `%val = typher.load %ptr : memref<i32> -> i32`
    }];

    let arguments = (ins Arg<AnyMemRef, "the address to read from", [MemRead]>:$addr);
    let results = (outs AnyType:$value);

    let assemblyFormat = "$addr attr-dict `:` type($addr) `->` type($value)";
}

//===----------------------------------------------------------------------===//
// FuncOp
//===----------------------------------------------------------------------===//

def FuncOp : Typher_Op<"func", [
    FunctionOpInterface, IsolatedFromAbove
  ]> {
  let summary = "user defined function operation";
  let description = [{
    The "toy.func" operation represents a user defined function. These are
    callable SSA-region operations that contain toy computations.

    Example:

    ```mlir
    toy.func @main() {
      %0 = toy.constant dense<5.500000e+00> : tensor<f64>
      %1 = toy.reshape(%0 : tensor<f64>) to tensor<2x2xf64>
      toy.print %1 : tensor<2x2xf64>
      toy.return
    }
    ```
  }];

  let arguments = (ins
    SymbolNameAttr:$sym_name,
    TypeAttrOf<FunctionType>:$function_type,
    OptionalAttr<DictArrayAttr>:$arg_attrs,
    OptionalAttr<DictArrayAttr>:$res_attrs
  );
  let regions = (region AnyRegion:$body);

  let builders = [OpBuilder<(ins
    "StringRef":$name, "FunctionType":$type,
    CArg<"ArrayRef<NamedAttribute>", "{}">:$attrs)
  >];

  let extraClassDeclaration = [{
    //===------------------------------------------------------------------===//
    // FunctionOpInterface Methods
    //===------------------------------------------------------------------===//

    /// Returns the argument types of this function.
    ArrayRef<Type> getArgumentTypes() { return getFunctionType().getInputs(); }

    /// Returns the result types of this function.
    ArrayRef<Type> getResultTypes() { return getFunctionType().getResults(); }

    Region *getCallableRegion() { return &getBody(); }
  }];

  let hasCustomAssemblyFormat = 1;
  let skipDefaultBuilders = 1;
}

//===----------------------------------------------------------------------===//
// GenericCallOp
//===----------------------------------------------------------------------===//

def GenericCallOp : Typher_Op<"generic_call"> {
  let summary = "generic call operation";
  let description = [{
    Generic calls represent calls to a user defined function that needs to
    be specialized for the shape of its arguments. The callee name is attached
    as a symbol reference via an attribute. The arguments list must match the
    arguments expected by the callee. For example:

    ```mlir
     %4 = toy.generic_call @my_func(%1, %3)
           : (tensor<2x3xf64>, tensor<2x3xf64>) -> tensor<*xf64>
    ```

    This is only valid if a function named "my_func" exists and takes two
    arguments.
  }];

  // The generic call operation takes a symbol reference attribute as the
  // callee, and inputs for the call.
  let arguments = (ins FlatSymbolRefAttr:$callee, Variadic<C_Type>:$inputs);

  // The generic call operation returns a single value of TensorType.
  let results = (outs Variadic<C_Type>);

  // Specialize assembly printing and parsing using a declarative format.
  let assemblyFormat = [{
    $callee `(` $inputs `)` attr-dict `:` functional-type($inputs, results)
  }];

  // Add custom build methods for the generic call operation.
  let builders = [
    OpBuilder<(ins "StringRef":$callee, "ArrayRef<Value>":$arguments)>
  ];
}

//===----------------------------------------------------------------------===//
// IfOp
//===----------------------------------------------------------------------===//

def IfOp : Typher_Op<"if", [
    RecursiveMemoryEffects, // Allows optimizations to look inside the regions
    NoRegionArguments, // Standard 'if' blocks usually don't take args
    SingleBlock       
]> {
    let summary = "If-then-else conditional operation";
    let description = [{
        An operation that executes the 'then' region if the condition is true,
        and the 'else' region if the condition is false.
    }];

    // The condition must be a 1-bit integer (i1)
    let arguments = (ins I1:$condition);

    // We define two regions: 'then' and 'else'
    let regions = (region SizedRegion<1>:$thenRegion, 
                          MaxSizedRegion<1>:$elseRegion);

    let assemblyFormat = [{
        $condition $thenRegion (`else` $elseRegion^)? attr-dict
    }];

	let builders = [
		OpBuilder<(ins "Value":$condition, "bool":$withElseRegion)>
	];

	let hasVerifier = 1;
}

//===----------------------------------------------------------------------===//
// WhileOp
//===----------------------------------------------------------------------===//


def WhileOp : Typher_Op<"while", [RecursiveMemoryEffects]> {
    let summary = "C-like while loop";
    let description = [{
        A high-level while loop. 
        Region 0: The condition (must terminate with a custom ConditionYield).
        Region 1: The loop body.
    }];

    let regions = (region SizedRegion<1>:$condRegion, 
                         SizedRegion<1>:$bodyRegion);

    let assemblyFormat = [{
        attr-dict `cond` $condRegion `do` $bodyRegion
    }];
}


//===----------------------------------------------------------------------===//
// ReturnOp
//===----------------------------------------------------------------------===//

def ReturnOp : Typher_Op<"return", [Pure, ParentOneOf<["FuncOp", "IfOp"]>,
                                 Terminator]> {
  let summary = "return operation";
  let description = [{
    The "return" operation represents a return operation within a function.
    The operation takes an optional tensor operand and produces no results.
    The operand type must match the signature of the function that contains
    the operation. For example:

    ```mlir
      toy.func @foo() -> tensor<2xf64> {
        ...
        toy.return %0 : tensor<2xf64>
      }
    ```
  }];

  // The return operation takes an optional input operand to return. This
  // value must match the return type of the enclosing function.
  let arguments = (ins AnyTypeOf<[C_Type, AnyMemRef]>:$input);

  // The return operation only emits the input in the format if it is present.
  let assemblyFormat = "$input attr-dict `:` type($input)";

  // Allow building a ReturnOp with no return operand.
  let builders = [
    OpBuilder<(ins), [{ build($_builder, $_state, {}); }]>
  ];

  let extraClassDeclaration = [{
    bool hasOperand() { return getOperation()->getNumOperands() > 0; }
  }];

  // Invoke a static verify method to verify this return operation.
  let hasVerifier = 1;
}


//===----------------------------------------------------------------------===//
// EqualsOp
//===----------------------------------------------------------------------===//

def EqualsOp : Typher_Op<"eq"> {
    let arguments = (ins AnyTypeOf<[C_Type, AnyMemRef]>:$lhs, AnyTypeOf<[C_Type, AnyMemRef]>:$rhs);
    let results = (outs I1:$result);

    let builders = [
        // Custom builder: Only takes lhs and rhs
        OpBuilder<(ins "Value":$lhs, "Value":$rhs)>
    ];

    let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($lhs) `,` type($rhs)";
}

//===----------------------------------------------------------------------===//
// YieldOp
//===----------------------------------------------------------------------===//

// Yield Operator: The required terminator for IfOp regions
def YieldOp : Typher_Op<"yield", [Terminator]> {
    let summary = "Terminator for regions";
    let description = [{
        Ends a block within a region (like the then/else blocks of an IfOp).
    }];
    
    // We make it variadic so it can return values if your IfOp eventually does
    let arguments = (ins Variadic<AnyType>:$operands);
    let assemblyFormat = "attr-dict ($operands^ `:` type($operands))?";
	  let builders = [
        OpBuilder<(ins), [{ /* Empty builder just creates the op */ }]>
    ];
}


#endif // TYPHER_OPS