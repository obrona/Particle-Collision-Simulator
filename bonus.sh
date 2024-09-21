# bash script to make bonus and bonus.sh
flags="-Wall -Wextra -Werror -pedantic -std=c++20 -fopenmp -O3"
g++ $flags -DCHECK=1 sim_validator.a io.cc sim.cc -o bonus
g++ $flags -DCHECK=0 io.cc sim.cc -o bonus.perf