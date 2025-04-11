compile perf: `g++ -std=c++20 -fopenmp -O3 -DCHECK=0 io.cc sim.cc -o sim.perf`
<br>
compile check: `g++ -std=c++20 -fopenmp -O3 -DCHECK=1 io.cc sim.cc -o sim.perf`
<br>
run: `./sim.perf tests/large_fixed/100k_density_0.8.in 12`