#include <omp.h>

#include <bits/stdc++.h>
#include <cmath>

#include "collision.h"
#include "io.h"
#include "sim_validator.h"
using namespace std;

int DIRS[8][2] = {{-1,0}, {-1,1}, {0,1}, {1,1}, {1,0}, {1,-1}, {0,-1}, {-1,-1}};

struct Simulator {
    int ROWS;
    double bin_length;
    vector<vector<int>> bins; // the particles are identified from their index

    vector<Particle>&  particles;
    Params& params;
    
   

    Simulator(Params& params, vector<Particle> particles): particles(particles), params(params) {
        
    }

    void process_bin() {
        bin_length = max(params.square_size / floor(sqrt(params.param_particles)), 4.0 * params.param_radius);
        ROWS = (int) floor(params.square_size / bin_length);
        bin_length = params.square_size / ROWS; // bins on the right/btm edge are likely bigger
        bins = vector(ROWS * ROWS, vector<int>());
    }

    void bin_particles() {
        #pragma omp parallel for
        for (size_t i = 0; i < bins.size(); i ++) {
            bins[i].clear();
        }

        #pragma omp parallel for 
        for (size_t i = 0; i < particles.size(); i ++) {
            Particle& p = particles[i];
            bins[to_bin(p)].push_back(p.i);
        }
        
    }


    bool process_wall_collision(vector<Particle>& particles, int square_size, int radius) {
        bool changed = false;
        int len = particles.size();
        
        #pragma omp parallel for 
        for (int i = 0; i < len; i ++) {
            Particle& p = particles[i];
            if (is_wall_collision(p.loc, p.vel, square_size, radius)) {
                changed = true;
                resolve_wall_collision(p.loc, p.vel, square_size, radius);
            }
        }
        
        return changed;
    }

    bool process_particle_collision(vector<Particle>& particles, int radius) {
        bool changed = false;
        int len = particles.size();
        int lim_grp = len - 1 + len - 2;
       
        for (int grp = 1; grp <= lim_grp; grp ++) {
            int lim = grp >> 1;
            #pragma omp parallel for
            for (int i = 0; i <= lim; i ++) {
                int j = grp - i;
                if (i == j || j >= len) continue;

                Particle& p0 = particles[i];
                Particle& p1 = particles[j];
                if (is_particle_collision(p0.loc, p0.vel, p1.loc, p1.vel, radius)) {
                    changed = true;
                    resolve_particle_collision(p0.loc, p0.vel, p1.loc, p1.vel);
                }
            }
        }
        
        return changed;
    }

    void process_timestep(vector<Particle>& particles, int square_size, int radius) {
        int len = particles.size();

        #pragma omp parallel for
        for (int i = 0; i < len; i ++) {
            Particle& p = particles[i];
            p.loc.x += p.vel.x;
            p.loc.y += p.vel.y;
        }

        while (true) {
            bool b0 = process_wall_collision(particles, square_size, radius);
            bool b1 = process_particle_collision(particles, radius);
            if (!b0 && !b1) break;
        }
    }
    private:
    int to_bin(const Particle& p) {
        int y_row= p.loc.y / bin_length;
        int x_col = p.loc.x / bin_length;
        if (y_row == ROWS) y_row = ROWS - 1;
        if (x_col == ROWS) x_col = ROWS - 1;
        return y_row * ROWS + x_col;
    }
};

