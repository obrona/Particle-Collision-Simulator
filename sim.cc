#include <omp.h>

#include <cmath>
#include <fstream>
#include <vector>
#include <iostream>
using namespace std;

#include "collision.h"
#include "io.h"
#include "sim_validator.h"
#include "simul.cc"

int main(int argc, char* argv[]) {
    // Read arguments and input file
    Params params{};
    std::vector<Particle> particles;
    read_args(argc, argv, params, particles);

    // Set number of threads
    omp_set_num_threads(params.param_threads);
    Simulator simulator(params, particles);
   

#if CHECK == 1
    // Initialize collision checker
    SimulationValidator validator(params.param_particles, params.square_size, params.param_radius);
    // Initialize with starting positions
    validator.initialize(particles);
    // Uncomment the line below to enable visualization (makes program much slower)
    // validator.enable_viz_output("test.out");
#endif

    // TODO: this is the part where you simulate particle behavior.

    /*
    After simulating each timestep, you must call:

    #if CHECK == 1
    validator.validate_step(particles);
    #endif
    */
    for (int step = 0; step <= params.param_steps; step ++) {
        //cout << "Step: " << step << endl;
        simulator.process_timestep(particles, params.square_size, params.param_radius);
        #if CHECK == 1
            validator.validate_step(particles);
        #endif
    }
}
