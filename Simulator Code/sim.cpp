// Imports
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <random>
#include <chrono>
#include "common.h"

// Provide some namespace shortcuts for timing
using std::cout;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

const char *FILE_NAME = "results_12-11.txt";

// Simple collision handling: if a particle is found outside the box, reset its
// position to the edge of the box and zero out its velocity and acceleration
// in that direction
void handle_collisions(float *positions, float *velocities, float *accelerations, int num_particles)
{
    for (int i = 0; i < num_particles; ++i)
    {
        for (int dim = 0; dim < 3; ++dim)
        {
            // Check if the particle will collide with the box in the current dimension
            if (positions[i * 3 + dim] < 0.0f)
            {
                // Particle is outside the box on the lower side
                positions[i * 3 + dim] = 0.0f;
                velocities[i * 3 + dim] = 0.0f;
                accelerations[i * 3 + dim] = 0.0f;
            }
            else if (positions[i * 3 + dim] > BOX_WIDTH)
            {
                // Particle is outside the box on the upper side
                positions[i * 3 + dim] = BOX_WIDTH;
                velocities[i * 3 + dim] = 0.0f;
                accelerations[i * 3 + dim] = 0.0f;
            }
        }
    }
}

void frame_update(float *positions, float *velocities, float *accelerations, int num_particles)
{
    // Performs the update to the pos, vel, and accel arrays
    // for a single frame
    // Update accelerations
    for (int i = 0; i < num_particles; ++i)
    {
        accelerations[i * 3 + 0] = 0.0f; // Initialize accelerations to zero
        accelerations[i * 3 + 1] = 0.0f;
        accelerations[i * 3 + 2] = 0.0f;

        for (int j = 0; j < num_particles; ++j)
        {
            if (i != j)
            {
                // Calculate distance between particles i and j
                float dx = positions[j * 3 + 0] - positions[i * 3 + 0];
                float dy = positions[j * 3 + 1] - positions[i * 3 + 1];
                float dz = positions[j * 3 + 2] - positions[i * 3 + 2];
                float r = std::sqrt(dx * dx + dy * dy + dz * dz);

                if (r > 0 && !std::isnan(r))
                {
                    float force = (G * PARTICLE_MASS * PARTICLE_MASS) / (r * r);

                    // Update acceleration of particle i
                    accelerations[i * 3 + 0] += force * (dx / r);
                    accelerations[i * 3 + 1] += force * (dy / r);
                    accelerations[i * 3 + 2] += force * (dz / r);
                }
            }
        }
    }

    // Update velocities and positions
    for (int i = 0; i < num_particles; ++i)
    {
        // Update velocities using the calculated accelerations
        velocities[i * 3 + 0] += accelerations[i * 3 + 0];
        velocities[i * 3 + 1] += accelerations[i * 3 + 1];
        velocities[i * 3 + 2] += accelerations[i * 3 + 2];

        // Update positions using the updated velocities
        positions[i * 3 + 0] += velocities[i * 3 + 0];
        positions[i * 3 + 1] += velocities[i * 3 + 1];
        positions[i * 3 + 2] += velocities[i * 3 + 2];
    }

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

    // Files info
    FILE *file_ptr;
    file_ptr = fopen(FILE_NAME, "w");

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

    // Print out initial positions before running sim
    // print_positions(positions, num_particles, file_ptr);

    // Start timer
    start = high_resolution_clock::now();

    // Run the sim (official timing should not include print_positions)
    for (int i = 0; i < num_frames - 1; i++)
    {
        frame_update(positions, velocities, accelerations, num_particles);
        // print_positions(positions, num_particles, file_ptr);
    }

    // Stop timer and print timing result
    end = high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<duration<double, std::milli>>(end - start);
    std::cout << "Time taken: " << duration_sec.count() << "ms";

    // Free the allocated memory
    delete[] positions;
    delete[] velocities;
    delete[] accelerations;

    // Close opened file
    fclose(file_ptr);

    return 0;
}