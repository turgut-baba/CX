module {                                                                                                                                                                                                             
   func.func @caller() -> i32 {
      %0 = arith.constant 5 : i32
      %1 = arith.constant 5 : i32
      %2 = llvm.add %0, %1 : i32
      func.return %2 : i32
   }

   func.func @main() -> i32 {                                                                                                                                                                                         
      %0 = llvm.generic_call @caller() : () -> i32                                                                                                                                                                     
      %1 = arith.constant 5 : i32
      %2 = llvm.add %0, %1 : i32
      %3 = arith.constant 0 : i32
      func.return %3 : i32
   }                                                                                                                                                                                                                
}