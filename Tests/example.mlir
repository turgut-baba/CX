int b = 4;
int main() {
    int a = 3 + b;
}

module {
  // global int b = 4;
  memref.global "private" @b : memref<i32> = dense<4>

  func.func @main() -> i32 {
    // load global b
    %b = memref.get_global @b : memref<i32>
    %b_val = memref.load %b[] : memref<i32>

    // constant 3
    %c3 = arith.constant 3 : i32

    // a = 3 + b
    %sum = arith.addi %c3, %b_val : i32

    // allocate a (C semantics)
    %a = memref.alloca() : memref<i32>
    memref.store %sum, %a[] : memref<i32>

    // return a
    %ret = memref.load %a[] : memref<i32>
    return %ret : i32
  }
}




b:
    .long 4

main:
    pushq   %rbp
    movq    %rsp, %rbp

    movl    b(%rip), %eax     # load global b
    addl    $3, %eax          # eax = b + 3
    movl    %eax, -4(%rbp)
    xorl    %eax, %eax

    popq    %rbp
    ret



[[GPU]] int res(int a) {

a = a + 2;

return a;

}



int main() {

int b = 3;

int c = res(b);

}


module attributes {gpu.container_module} {
  gpu.module @kernels {
    // The 'return' is now a 'store' into a result buffer
    gpu.func @res_kernel(%a: i32, %out: memref<1xi32>) kernel {
      %c2 = arith.constant 2 : i32
      %res = arith.addi %a, %c2 : i32
      %c0 = arith.constant 0 : index
      memref.store %res, %out[%c0] : memref<1xi32>
      gpu.return
    }
  }

  func.func @main() {
    %b = arith.constant 3 : i32
    // Compiler allocates a small slot for the 'return' value
    %temp_buf = gpu.alloc() : memref<1xi32> 
    
    // Launching the GPU module
    gpu.launch_func @kernels::@res_kernel 
        args(%b : i32, %temp_buf : memref<1xi32>)
    
    // Load the 'returned' value back into a CPU variable
    %c = memref.load %temp_buf[%c0] : memref<1xi32>
    return
  }
}