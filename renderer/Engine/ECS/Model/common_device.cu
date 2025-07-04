//
// Created by lenovo on 10/14/2024.
//

#include "common_device.cuh"

__global__  void transformVerticesKernel(float3* vertices, float3 center, uint32_t numVertices){
    auto idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < numVertices) {
        vertices[idx].x -= center.x;
        vertices[idx].y -= center.y;
        vertices[idx].z -= center.z;
    }
}


void gpu::transformVerticesCUDA(float3 *vertices, float3 center, uint32_t numVertices) {
    uint32_t threadsPerBlock = 256;
    uint32_t blocksPerGrid = (numVertices + threadsPerBlock - 1) / threadsPerBlock;
    transformVerticesKernel<<<blocksPerGrid, threadsPerBlock>>>(vertices, center, numVertices);
    cudaDeviceSynchronize();
}

