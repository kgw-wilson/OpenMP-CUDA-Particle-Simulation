// Imports
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <random>
#include <chrono>
#include "cuda.h"
#include <cuda_runtime.h>
#include "common.h"

// Provide some namespace shortcuts for timing
using std::cout;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

// CUDA kernel for handling collisions
__global__ void handle_collisions_kernel(float *positions, float *velocities, float *accelerations, int num_particles)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;

    if (tid < num_particles)
    {
        // Check and handle collisions for the x dimension
        if (positions[tid * 3] < 0.0f)
        {
            positions[tid * 3] = 0.0f;
            velocities[tid * 3] = 0.0f;
            accelerations[tid * 3] = 0.0f;
        }
        else if (positions[tid * 3] > BOX_WIDTH)
        {
            positions[tid * 3] = BOX_WIDTH;
            velocities[tid * 3] = 0.0f;
            accelerations[tid * 3] = 0.0f;
        }

        // Check and handle collisions for the y dimension
        if (positions[tid * 3 + 1] < 0.0f)
        {
            positions[tid * 3 + 1] = 0.0f;
            velocities[tid * 3 + 1] = 0.0f;
            accelerations[tid * 3 + 1] = 0.0f;
        }
        else if (positions[tid * 3 + 1] > BOX_WIDTH)
        {
            positions[tid * 3 + 1] = BOX_WIDTH;
            velocities[tid * 3 + 1] = 0.0f;
            accelerations[tid * 3 + 1] = 0.0f;
        }

        // Check and handle collisions for the z dimension
        if (positions[tid * 3 + 2] < 0.0f)
        {
            positions[tid * 3 + 2] = 0.0f;
            velocities[tid * 3 + 2] = 0.0f;
            accelerations[tid * 3 + 2] = 0.0f;
        }
        else if (positions[tid * 3 + 2] > BOX_WIDTH)
        {
            positions[tid * 3 + 2] = BOX_WIDTH;
            velocities[tid * 3 + 2] = 0.0f;
            accelerations[tid * 3 + 2] = 0.0f;
        }
    }
}

// CUDA kernel for frame update
__global__ void frame_update_kernel(float *positions, float *velocities, float *accelerations, int num_particles)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;

    if (tid < num_particles)
    {
        float acc_x = 0.0f;
        float acc_y = 0.0f;
        float acc_z = 0.0f;

        for (int j = 0; j < num_particles; ++j)
        {
            if (tid != j)
            {
                float dx = positions[j * 3 + 0] - positions[tid * 3 + 0];
                float dy = positions[j * 3 + 1] - positions[tid * 3 + 1];
                float dz = positions[j * 3 + 2] - positions[tid * 3 + 2];
                float r = sqrtf(dx * dx + dy * dy + dz * dz);

                if (r > 0 && !std::isnan(r))
                {
                    float force = (G * PARTICLE_MASS * PARTICLE_MASS) / (r * r);

                    acc_x += force * (dx / r);
                    acc_y += force * (dy / r);
                    acc_z += force * (dz / r);
                }
            }
        }

        // Update accelerations
        accelerations[tid * 3 + 0] = acc_x;
        accelerations[tid * 3 + 1] = acc_y;
        accelerations[tid * 3 + 2] = acc_z;

        // Update velocities and positions
        velocities[tid * 3 + 0] += accelerations[tid * 3 + 0];
        velocities[tid * 3 + 1] += accelerations[tid * 3 + 1];
        velocities[tid * 3 + 2] += accelerations[tid * 3 + 2];

        positions[tid * 3 + 0] += velocities[tid * 3 + 0];
        positions[tid * 3 + 1] += velocities[tid * 3 + 1];
        positions[tid * 3 + 2] += velocities[tid * 3 + 2];
    }
}

// Wrapper function for handling collisions on GPU
void handle_collisions_cuda(float *positions, float *velocities, float *accelerations, int num_particles)
{
    int num_threads = 256;
    int num_blocks = (num_particles + num_threads - 1) / num_threads;

    handle_collisions_kernel<<<num_blocks, num_threads>>>(positions, velocities, accelerations, num_particles);
    cudaDeviceSynchronize(); // Ensure the kernel execution is completed before proceeding
}

// Wrapper function for frame update on GPU
void frame_update_cuda(float *positions, float *velocities, float *accelerations, int num_particles)
{
    int num_threads = 256;
    int num_blocks = (num_particles + num_threads - 1) / num_threads;

    frame_update_kernel<<<num_blocks, num_threads>>>(positions, velocities, accelerations, num_particles);
    cudaDeviceSynchronize(); // Ensure the kernel execution is completed before proceeding

    // Handle the collisions next
    handle_collisions_cuda(positions, velocities, accelerations, num_particles);
}

// Main
int main(int argc, char *argv[])
{

    // Read and validate arguments passed when running executable
    SimulationParams params = parse_sim_args(argc, argv);
    if (!params.valid)
        return 1;

    int num_frames = params.num_frames;
    int num_particles = params.num_particles;

    // Random number setup
    auto [eng, distr] = create_uniform_rng(0.0f, BOX_WIDTH);

    // Timing setup
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    // Initialize arrays using CUDA managed memory
    float *positions;
    float *velocities;
    float *accelerations;

    cudaMallocManaged(&positions, num_particles * 3 * sizeof(float));
    cudaMallocManaged(&velocities, num_particles * 3 * sizeof(float));
    cudaMallocManaged(&accelerations, num_particles * 3 * sizeof(float));

    // Populate positions with randomly distributed particles
    for (int i = 0; i < num_particles; i++)
    {
        positions[i * 3] = distr(eng);
        positions[i * 3 + 1] = distr(eng);
        positions[i * 3 + 2] = distr(eng);
    }

    // Start timer
    cudaEventRecord(start);

    // Run the sim
    for (int i = 0; i < num_frames - 1; i++)
    {
        frame_update_cuda(positions, velocities, accelerations, num_particles);
    }

    // Stop timer and print timing result
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    float elapsed_time;
    cudaEventElapsedTime(&elapsed_time, start, stop);
    std::cout << "Time taken: " << elapsed_time << "ms";

    // Free the allocated memory
    cudaFree(positions);
    cudaFree(velocities);
    cudaFree(accelerations);

    return 0;
}