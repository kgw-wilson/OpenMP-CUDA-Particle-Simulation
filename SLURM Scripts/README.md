# SLURM Scripts

SLURM is a job scheduling system for High Performance Computing (HPC) clusters. SLURM schedules each job according to the requested resources and other active/pending jobs. 

After completion, output and error logs will be written to the specified files (e.g., baseline_sim.out, baseline_sim.err).

Each script in this directory corresponds to a different execution mode: baseline CPU, OpenMP parallel CPU, and GPU-accelerated.

## Compilation

Compilation

All C++ files are compiled with the following flags:

• -Wall – Enables compiler warnings

• -O3 – Optimizes for maximum performance

• -std=c++17 – Uses the C++17 standard

## sbatch Flags

• --cpus-per-task defines how many hardware threads SLURM guarantees for your task. This was set to 1 for the baseline CPU condition and 8 for the OpenMP condition to test the timing benefits of parallelization on the CPU.
    
• --nodes only matters if you want multiple physical machines. For OpenMP, I used 1 node (physical machine) because OpenMP doesn’t natively parallelize across nodes (that’s what MPI is for).

• --gres=gpu:1 requests 1 GPU, to make a fair comparison between the conditions.
