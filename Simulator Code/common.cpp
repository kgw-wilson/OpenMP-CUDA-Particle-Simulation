#include "common.h"

float BOX_WIDTH = 100.0f;
float G = 6.67430f;
float PARTICLE_MASS = 1.0f;

void print_positions(float *positions, int num_positions, FILE *file_ptr)
{
    // Prints out positions for a single frame to a file.
    // Positions for different particles are stored in 1D
    // and are separated by commas like this:
    // x1, y1, z1, x2, y2, z2, . . .
    // Positions for frames are separated by newlines
    int i = 0;
    for (; i < num_positions - 1; i++)
    {
        fprintf(file_ptr, "%f,%f,%f,", positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
    }
    i++;
    fprintf(file_ptr, "%f,%f,%f\n", positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
}

SimulationParams parse_sim_args(int argc, char *argv[])
{
    SimulationParams params{0, 0, false};

    if (argc != 3)
    {
        std::cerr << "Usage: ./sim <num_frames> <num_particles>\n";
        return params;
    }

    int num_frames = atoi(argv[1]);
    int num_particles = atoi(argv[2]);

    if (num_frames < 1 || num_particles < 1)
    {
        std::cerr << "Error: num_frames and num_particles must be >= 1\n";
        return params;
    }

    params.num_frames = num_frames;
    params.num_particles = num_particles;
    params.valid = true;
    return params;
}

// Returns a <mt19937 engine, uniform_real_distribution> pair for floats in [min, max)
std::pair<std::mt19937, std::uniform_real_distribution<float>> create_uniform_rng(float min, float max)
{
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_real_distribution<float> distr(min, max);
    return {eng, distr};
}