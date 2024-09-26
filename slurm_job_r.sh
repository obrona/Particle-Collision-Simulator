#!/bin/bash

## This is an example Slurm template job script for A1 that just runs the script and arguments you pass in via `srun`.

#SBATCH --job-name=a1-perf-r
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --job-name=array_job_1
#SBATCH --output=output_%A_%a.txt
#SBATCH --error=error_%A_%a.txt
#SBATCH --array=0-5  # Adjust this
#SBATCH --mem=4gb
#SBATCH --partition=i7-7700
#SBATCH --time=00:10:00


values=(15 16 17 18 19 20)
# Get the current array index
index=$SLURM_ARRAY_TASK_ID
# Access the corresponding value from the array
value=${values[$index]}
# Runs your script with the arguments you passed in
temp_file="temp_file_${SLURM_JOB_ID}_${SLURM_ARRAY_TASK_ID}.txt"
python3 gen_testcase.py 50000 10000 $value 100 0 5 > "$temp_file"
echo "50000 10000 $value 100 0 5 8" 1>&2
srun perf stat -r 3 ./sim.perf "$temp_file" 8
rm "$temp_file"