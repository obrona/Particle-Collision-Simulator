#!/bin/bash

#SBATCH --job-name=a1-perf-i7-7700
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --job-name=array_job_1
#SBATCH --output=output_i7-7700.txt
#SBATCH --error=error_i7-7700.txt
#SBATCH --mem=4gb
#SBATCH --partition=i7-7700
#SBATCH --time=00:10:00


srun time ./sim.perf tests/large_fixed/100k_density_0.7.in 12
srun time ./sim.perf tests/large_fixed/100k_density_0.7.in 12
srun time ./sim.perf tests/large_fixed/100k_density_0.7.in 12