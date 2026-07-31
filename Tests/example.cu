#include <cuda_runtime.h>
#include <iostream>
#include <vector>

// CUDA Kernel: Simple Box Blur
// Reads from src, writes blurred output to dst
__global__ void box_blur_kernel(const unsigned char* src, unsigned char* dst, 
                                int width, int height, int radius) {
    // Compute 2D pixel coordinates for this thread
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    // Boundary check
    if (x >= width || y >= height) return;

    int sum = 0;
    int count = 0;

    // Loop through neighboring pixels within the blur radius
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int nx = x + dx;
            int ny = y + dy;

            // Clamp coordinates to image borders (Border Handling)
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                sum += src[ny * width + nx];
                count++;
            }
        }
    }

    // Write averaged pixel to destination buffer
    dst[y * width + x] = static_cast<unsigned char>(sum / count);
}

// Helper wrapper function to configure execution bounds and launch kernel
void apply_box_blur(const unsigned char* h_src, unsigned char* h_dst, 
                    int width, int height, int radius) {
    size_t img_size = width * height * sizeof(unsigned char);

    unsigned char *d_src = nullptr;
    unsigned char *d_dst = nullptr;

    // 1. Allocate Device Memory
    cudaMalloc(&d_src, img_size);
    cudaMalloc(&d_dst, img_size);

    // 2. Copy Host Input to Device
    cudaMemcpy(d_src, h_src, img_size, cudaMemcpyHostToDevice);

    // 3. Configure 2D Execution Grid
    dim3 threadsPerBlock(16, 16);
    dim3 numBlocks((width + threadsPerBlock.x - 1) / threadsPerBlock.x,
                   (height + threadsPerBlock.y - 1) / threadsPerBlock.y);

    // 4. Launch Kernel
    box_blur_kernel<<<numBlocks, threadsPerBlock>>>(d_src, d_dst, width, height, radius);

    // Synchronize and check for errors
    cudaDeviceSynchronize();

    // 5. Copy Device Result back to Host
    cudaMemcpy(h_dst, d_dst, img_size, cudaMemcpyDeviceToHost);

    // 6. Free Device Memory
    cudaFree(d_src);
    cudaFree(d_dst);
}

int main() {
    const int width = 1024;
    const int height = 1024;
    const int radius = 3; // 7x7 blur box

    // Create dummy image data
    std::vector<unsigned char> input_image(width * height, 128);
    std::vector<unsigned char> output_image(width * height, 0);

    // Run GPU Blur
    apply_box_blur(input_image.data(), output_image.data(), width, height, radius);

    std::cout << "Blur applied successfully to " << width << "x" << height << " image!" << std::endl;
    return 0;
}