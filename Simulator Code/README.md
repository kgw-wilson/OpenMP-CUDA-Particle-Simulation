# Simulator Code

This directory contains multiple implementations of a particle simulation using C++ and CUDA. The simulation models particles in a 3D box under mutual gravitational forces, including basic collision handling. The project supports three versions:

1. Serial CPU simulation (sim.cpp)

2. Multithreaded CPU simulation with OpenMP (omp_sim.cpp)

3. GPU-accelerated simulation with CUDA (gpu_sim.cu)

Shared particle utility functions (printing positions or densities) are located in particle_functions.cpp and particle_functions.h.

## Interaction with SLURM Scripts

The `SLURM Scripts` directory contains shell scripts that the job scheduler for the cluster uses to compile the code in this directory into executables and subit jobs. 
