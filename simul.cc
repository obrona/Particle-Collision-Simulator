#include <omp.h>

#include <bits/stdc++.h>
#include <cmath>

#include "collision.h"
#include "io.h"

using namespace std;

//vector<vector<int>> DIRS = {{-1,0}, {-1,1}, {0,1}, {1,1}, {1,0}, {1,-1}, {0,-1}, {-1,-1}};
vector<vector<int>> DIRS = {{-1,0}, {-1,1}, {0,1}, {1,1}};

struct Simulator {
    int ROWS;
    double bin_length;
    vector<vector<int>> bins; // the particles are identified from their index

    vector<Particle>&  particles;
    Params& params;
    
   

    Simulator(Params& params, vector<Particle>& particles): particles(particles), params(params) {
        process_bin();
    }

    void process_bin() {
        //bin_length = max((double) params.square_size / sqrt(params.param_particles), 5.0 * (double) params.param_radius);
        bin_length = 5.0 * (double) params.param_radius;
        ROWS = (double) params.square_size / bin_length; //typecasted to int, so floor is taken already
        bin_length = (double) params.square_size / ROWS;
        bins = vector(ROWS * ROWS, vector<int>());
    }    

    int change_coor(int r, int c) {
        return r * ROWS + c;
    }

    bool valid(int r, int c) {
        return r >= 0 && r < ROWS && c >= 0 && c < ROWS;
    }

    void bin_particles() {
        //#pragma omp parallel for
        for (int i = 0; i < (int) bins.size(); i ++) {
            bins[i].clear();
        }

        int len = particles.size();
        //#pragma omp parallel for 
        for (int i = 0; i < len; i ++) {
            Particle& p = particles[i];
            
            int y_row = p.loc.y / bin_length;
            int x_col = p.loc.x / bin_length;

            // the particle can be outside the box
            y_row = min(max(0, y_row), ROWS - 1);
            x_col = min(max(0, x_col), ROWS - 1);
            
            bins[y_row * ROWS + x_col].push_back(i);
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


    
    bool process_particle_collision() {
        bool changed = false;
        // process own block 1st. In each block, we have to process collisions serially, but each block can be done
        // in parallel, as each particle is in 1 block only.
        #pragma omp parallel for collapse(2)
        for (int r = 0; r < ROWS; r ++) {
            for (int c = 0; c < ROWS; c ++) {
                vector<int>& bin = bins[change_coor(r, c)];
                int len = bin.size();
                for (int i = 0; i < len; i ++) {
                    for (int j = i + 1; j < len; j ++) {
                        Particle& p0 = particles[bin.at(i)];
                        Particle& p1 = particles[bin.at(j)];
                        if (is_particle_collision(p0.loc, p0.vel, p1.loc, p1.vel, params.param_radius)) {
                            changed = true;
                            resolve_particle_collision(p0.loc, p0.vel, p1.loc, p1.vel);
                        }
                    }
                }
            }
        }


        // when we check collision in a block with its adjacent blocks, we do it this way:
        // for all blocks, resolve collisons with its left block, then right block, top, bottom, top left ...
        // since the blocks are large, >= 5 * radius, let say, we doing block right neighbour, and p0 in b0 collide with p1 in b1,
        // then p1 cannot collide with any block in b2
        // but for particles in each block, have to do serially, because p0 and p0' in b0 can collide with p1 in b1
        //
        // another optimisation is to just check the top/left/right/bottom half only, for ther other half
        // the ajacent block will do the check
        for (int d = 0; d < 4; d ++) {
            vector<int>& dir = DIRS[d];
            #pragma omp parallel for collapse(2) 
            for (int r = 0; r < ROWS; r ++) {
                for (int c = 0; c < ROWS; c ++) {
                    if (!valid(r + dir[0], c + dir[1])) continue;

                    vector<int>& bin = bins[change_coor(r, c)];
                    vector<int>& neighbour_bin = bins[change_coor(r + dir[0], c + dir[1])];
                    for (int i = 0; i < (int) bin.size(); i ++) {
                        for (int j = 0; j < (int) neighbour_bin.size(); j ++) {
                            Particle& p0 = particles[bin[i]];
                            Particle& p1 = particles[neighbour_bin[j]];
                            if (is_particle_collision(p0.loc, p0.vel, p1.loc, p1.vel, params.param_radius)) {
                                changed = true;
                                resolve_particle_collision(p0.loc, p0.vel, p1.loc, p1.vel);
                            }
                        }
                    }
                }
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
        
        bin_particles();

        while (true) {
            bool b0 = process_wall_collision(particles, square_size, radius);
            bool b1 = process_particle_collision();
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

