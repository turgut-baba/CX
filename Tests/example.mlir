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
    %c0 = arith.constant 0 : index
    %c1 = arith.constant 1 : index
    
    %temp_buf = gpu.alloc() : memref<1xi32> 
    
    gpu.launch_func @kernels::@res_kernel 
        blocks in (%c1, %c1, %c1)
        threads in (%c1, %c1, %c1)
        args(%b : i32, %temp_buf : memref<1xi32>)
    
    %c = memref.load %temp_buf[%c0] : memref<1xi32>
    return
  }
}
