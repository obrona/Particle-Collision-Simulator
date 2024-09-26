#!/bin/bash

#SBATCH --job-name=a1 perf xs-4114
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --job-name=array_job_1
#SBATCH --output=output_xs-4114.txt
#SBATCH --error=error_xs-4114.txt
#SBATCH --mem=4gb
#SBATCH --partition=xs-4114
#SBATCH --time=00:10:00


srun time ./sim.perf tests/large/100k_density_0.7.in 12
srun time ./sim.perf tests/large/100k_density_0.7.in 12
srun time ./sim.perf tests/large/100k_density_0.7.in 12