// Imports
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <random>
#include <chrono>
#include "omp.h"
#include "common.h"

// Provide some namespace shortcuts for timing
using std::cout;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

void handle_collisions(float *positions, float *velocities, float *accelerations, int num_particles)
{
#pragma omp parallel for
    for (int i = 0; i < num_particles; ++i)
    {
        // Check if the particle will collide with the box in the x dimension
        if (positions[i * 3] < 0.0f)
        {
            positions[i * 3] = 0.0f;
            velocities[i * 3] = 0.0f;
            accelerations[i * 3] = 0.0f;
        }
        else if (positions[i * 3] > BOX_WIDTH)
        {
            positions[i * 3] = BOX_WIDTH;
            velocities[i * 3] = 0.0f;
            accelerations[i * 3] = 0.0f;
        }

        // Check if the particle will collide with the box in the y dimension
        if (positions[i * 3 + 1] < 0.0f)
        {
            positions[i * 3 + 1] = 0.0f;
            velocities[i * 3 + 1] = 0.0f;
            accelerations[i * 3 + 1] = 0.0f;
        }
        else if (positions[i * 3 + 1] > BOX_WIDTH)
        {
            positions[i * 3 + 1] = BOX_WIDTH;
            velocities[i * 3 + 1] = 0.0f;
            accelerations[i * 3 + 1] = 0.0f;
        }

        // Check if the particle will collide with the box in the z dimension
        if (positions[i * 3 + 2] < 0.0f)
        {
            positions[i * 3 + 2] = 0.0f;
            velocities[i * 3 + 2] = 0.0f;
            accelerations[i * 3 + 2] = 0.0f;
        }
        else if (positions[i * 3 + 2] > BOX_WIDTH)
        {
            positions[i * 3 + 2] = BOX_WIDTH;
            velocities[i * 3 + 2] = 0.0f;
            accelerations[i * 3 + 2] = 0.0f;
        }
    }
}

void frame_update(float *positions, float *velocities, float *accelerations, int num_particles)
{
// Update accelerations
#pragma omp parallel for
    for (int i = 0; i < num_particles; ++i)
    {
        float acc_x = 0.0f;
        float acc_y = 0.0f;
        float acc_z = 0.0f;

#pragma omp parallel for reduction(+ : acc_x, acc_y, acc_z)
        for (int j = 0; j < num_particles; ++j)
        {
            if (i != j)
            {
                float dx = positions[j * 3 + 0] - positions[i * 3 + 0];
                float dy = positions[j * 3 + 1] - positions[i * 3 + 1];
                float dz = positions[j * 3 + 2] - positions[i * 3 + 2];
                float r = std::sqrt(dx * dx + dy * dy + dz * dz);

                if (r > 0 && !std::isnan(r))
                {
                    float force = (G * PARTICLE_MASS * PARTICLE_MASS) / (r * r);

                    acc_x += force * (dx / r);
                    acc_y += force * (dy / r);
                    acc_z += force * (dz / r);
                }
            }
        }

        accelerations[i * 3 + 0] = acc_x;
        accelerations[i * 3 + 1] = acc_y;
        accelerations[i * 3 + 2] = acc_z;
    }

// Update velocities and positions
#pragma omp parallel for
    for (int i = 0; i < num_particles; ++i)
    {
        velocities[i * 3 + 0] += accelerations[i * 3 + 0];
        velocities[i * 3 + 1] += accelerations[i * 3 + 1];
        velocities[i * 3 + 2] += accelerations[i * 3 + 2];

        positions[i * 3 + 0] += velocities[i * 3 + 0];
        positions[i * 3 + 1] += velocities[i * 3 + 1];
        positions[i * 3 + 2] += velocities[i * 3 + 2];
    }

    // Assuming handle_collisions is already parallelized
    handle_collisions(positions, velocities, accelerations, num_particles);
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
    high_resolution_clock::time_point start;
    high_resolution_clock::time_point end;
    duration<double, std::milli> duration_sec;

    // Initialize arrays (1d arrays like x,y,z for each component of quantity measured)
    float *positions = new float[num_particles * 3];
    float *velocities = new float[num_particles * 3];
    float *accelerations = new float[num_particles * 3];

    // Populate positions with randomly distributed particles
    for (int i = 0; i < num_particles; i++)
    {
        positions[i * 3] = distr(eng);
        positions[i * 3 + 1] = distr(eng);
        positions[i * 3 + 2] = distr(eng);
    }

    // Start timer
    start = high_resolution_clock::now();

    // Run the sim
    for (int i = 0; i < num_frames - 1; i++)
    {
        frame_update(positions, velocities, accelerations, num_particles);
    }

    // Stop timer and print timing result
    end = high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    std::cout << "Time taken: " << duration_sec.count() << "ms";

    // Free the allocated memory
    delete[] positions;
    delete[] velocities;
    delete[] accelerations;

    return 0;
}