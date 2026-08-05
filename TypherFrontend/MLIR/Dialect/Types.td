#ifndef TYPHER_TYPES
#define TYPHER_TYPES

include "mlir/IR/AttrTypeBase.td"
include "mlir/IR/BuiltinTypeInterfaces.td"

// Base class for Typher types
class Typher_Type<string name, string typeMnemonic, list<Trait> traits = []>
    : TypeDef<Typher_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

// Custom Pointer Type Definition
def Typher_Pointer : Typher_Type<"Pointer", "ptr", [MemRefElementTypeInterface]> {
    let summary = "Typher pointer type";
    let description = [{
      Represents a pointer to a piece of memory holding a specific type.
      Syntax: `!typher.ptr<elementType>`
    }];

    let parameters = (ins "::mlir::Type":$elementType);
    let assemblyFormat = "`<` $elementType `>`";
}

def Typher_Array : Typher_Type<"Array", "array", [MemRefElementTypeInterface]> {
    let summary = "Typher fixed-size or dynamic array type";
    let description = [{
        Represents an array storing contiguous elements of a specific type.
        Supports fixed sizes (e.g., `!typher.array<10 x i32>`) or dynamic sizes 
        represented by `-1` or `?` (e.g., `!typher.array<? x i32>`).

        Syntax: `!typher.array<10 x i32>`
    }];

    let parameters = (ins 
        "int64_t":$size,
        "::mlir::Type":$elementType
    );

    let assemblyFormat = "`<` $size `x` $elementType `>`";
}

def C_Type : AnyTypeOf<[I1, I8, I16, I32, I64, F32, F64, Typher_Pointer]>;

#endif // TYPHER_TYPES