cd third_party/llvm-project
mkdir -p build && cd build

# Configure LLVM with MLIR enabled
cmake -G Ninja ../llvm \
  -DLLVM_ENABLE_PROJECTS="mlir" \
  -DLLVM_BUILD_EXAMPLES=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_TARGETS_TO_BUILD="Native"

# Build all MLIR static libraries (including libMLIRTransforms.a)
ninja -j2 check-mlir || ninja -j2 mlir-headers mlir-libraries
