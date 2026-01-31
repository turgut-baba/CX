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




module {
  // Global variable: int b (default initialized to 0)
  memref.global "private" @b : memref<i32> = dense<0>

  // Global variable: int c = 5
  memref.global "private" @c : memref<i32> = dense<5>

  // double test()
  func.func @test() -> f64 {
    // Load global c
    %c_ptr = memref.get_global @c : memref<i32>
    %c_val = memref.load %c_ptr[] : memref<i32>

    // (5 + 6)
    %c5 = arith.constant 5 : i32
    %c6 = arith.constant 6 : i32
    %sum1 = arith.addi %c5, %c6 : i32

    // (c + c)
    %sum2 = arith.addi %c_val, %c_val : i32

    // (5 + 6) + (c + c)
    %total_int = arith.addi %sum1, %sum2 : i32

    // double d = (cast int result to double)
    %d = arith.sitofp %total_int : i32 to f64

    // Return the double
    return %d : f64
  }

  // int main()
  func.func @main() -> i32 {
    // Load b and c
    %b_ptr = memref.get_global @b : memref<i32>
    %b_val = memref.load %b_ptr[] : memref<i32>

    %c_ptr = memref.get_global @c : memref<i32>
    %c_val = memref.load %c_ptr[] : memref<i32>

    // (c + c)
    %cc_sum = arith.addi %c_val, %c_val : i32

    // a = b + (c + c)
    %a = arith.addi %b_val, %cc_sum : i32

    // Return the integer result
    return %a : i32
  }
}



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
    %c1 = arith.constant 1 : index
    %c0 = arith.constant 0 : index
    
    // Allocate GPU memory
    %temp_buf = gpu.alloc() : memref<1xi32> 
    
    // Corrected Launch: Must specify Grid (blocks) and Block (threads) dimensions
    gpu.launch_func @kernels::@res_kernel 
        blocks in (%c1, %c1, %c1) 
        threads in (%c1, %c1, %c1)
        args(%b : i32, %temp_buf : memref<1xi32>)
    
    // Load result back
    %c = memref.load %temp_buf[%c0] : memref<1xi32>
    
    // (Optional) You might want a way to verify this, like a print call
    return
  }
}