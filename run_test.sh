mlir-opt-18 test.mlir \
    --finalize-memref-to-llvm \
    --convert-func-to-llvm \
    --convert-arith-to-llvm \
    --reconcile-unrealized-casts | \
mlir-translate-18 --mlir-to-llvmir -o output.ll

llc -filetype=obj output.ll -o output.o
clang output.o -o test_compiler

./test_compiler
echo $?

#cleanup
rm output.o
rm output.ll
rm test_compiler