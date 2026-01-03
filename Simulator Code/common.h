#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <random>
#include <utility>

extern float BOX_WIDTH;
extern float G;
extern float PARTICLE_MASS;

void print_positions(float *positions, int num_positions, FILE *file_ptr);

struct SimulationParams
{
    int num_frames;
    int num_particles;
    bool valid;
};

SimulationParams parse_sim_args(int argc, char *argv[]);

std::pair<std::mt19937, std::uniform_real_distribution<float>> create_uniform_rng(float min = 0.0f, float max = 100.0f);

#endif