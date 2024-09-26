#include <omp.h>
#include <bits/stdc++.h>
#include <cmath>

#include "collision.h"
#include "io.h"

using namespace std;

//vector<vector<int>> DIRS = {{-1,0}, {-1,1}, {0,1}, {1,1}, {1,0}, {1,-1}, {0,-1}, {-1,-1}};

struct Simulator {
    int ROWS;
    double bin_length;
    vector<vector<int>> bins; // the particles are identified from their inde
    
    vector<Particle>&  particles;
    Params& params;
    
   

    Simulator(Params& params, vector<Particle>& particles): particles(particles), params(params) {
        process_bin();
    }

    void process_bin() {
        bin_length = max((double) params.square_size / sqrt(params.param_particles), 5.0 * (double) params.param_radius);
        ROWS = (double) params.square_size / bin_length; //typecasted to int, so floor is taken already
        ROWS ++;
        bin_length = (double) params.square_size / ROWS;
        bins = vector(ROWS * ROWS, vector<int>());
    }    

    int change_coor(int r, int c) {
        return r * ROWS + c;
    }

    bool valid(int r, int c) {
        return r >= 0 && c >= 0 && r < ROWS && c < ROWS;
        return r >= 0 && c >= 0 && r < ROWS && c < ROWS;
    }

    void bin_particles() {
        //#pragma omp parallel for
        for (int i = 0; i < (int) bins.size(); i ++) {
            bins[i].clear();
        }

        int len = particles.size();
        for (int i = 0; i < len; i ++) {
            Particle& p = particles[i];
            
            int y_row = p.loc.y / bin_length;
            y_row = min(max(0, y_row), ROWS - 2);
            
            // if row is even, then it is shifted, if it is odd, not shifted.
            int x_col = (y_row & 1) ? p.loc.x / bin_length : (p.loc.x - bin_length / 2) / bin_length + 1; 
            x_col = min(max(0, x_col), ROWS - 1 - (y_row & 1));
            
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
        for (int r = 0; r < ROWS - 1; r ++) {
            for (int c = 0; c < ROWS; c ++) {
                if ((r & 1) && c == ROWS - 1) continue;
                
                vector<int>& bin = bins[change_coor(r, c)];
                int len = bin.size();
                for (int i = 0; i < len; i ++) {
                    for (int j = i + 1; j < len; j ++) {
                        Particle& p0 = particles[bin[i]];
                        Particle& p1 = particles[bin[j]];
                        if (is_particle_collision(p0.loc, p0.vel, p1.loc, p1.vel, params.param_radius)) {
                            changed = true;
                            resolve_particle_collision(p0.loc, p0.vel, p1.loc, p1.vel);
                        }
                    }
                }
            }
        }


        // 3 neighbor cells to check, top right, right, bottom right
        // check right
        #pragma omp parallel for collapse(2)
        for (int r = 0; r < ROWS - 1; r ++) {
            for (int c = 0; c < ROWS; c ++) {
                if (((r & 1) && c == ROWS - 1) || !valid(r, c + 1)) continue;
                
                vector<int>& bin = bins[change_coor(r, c)];
                vector<int>& neighbor_bin = bins[change_coor(r, c + 1)];
                int bin_len = bin.size(); 
                int neighbor_bin_len = neighbor_bin.size();
                for (int i = 0; i < bin_len; i ++) {
                    for (int j = 0; j < neighbor_bin_len; j ++) {
                        Particle& p0 = particles[bin[i]];
                        Particle& p1 = particles[neighbor_bin[j]];
                        if (is_particle_collision(p0.loc, p0.vel, p1.loc, p1.vel, params.param_radius)) {
                            changed = true;
                            resolve_particle_collision(p0.loc, p0.vel, p1.loc, p1.vel);
                        }
                    }
                }
            }
        }

        // check top right and bottom right
        for (int d = -1; d <= 1; d += 2) {
            #pragma omp parallel for
            for (int r = 0; r < ROWS - 1; r ++) {
                for (int c = 0; c < ROWS; c ++) {
                    if (((r & 1) && c == ROWS - 1) || !valid(r + d, c + (r & 1))) continue;
                    
                    vector<int>& bin = bins[change_coor(r, c)];
                    vector<int>& neighbor_bin = bins[change_coor(r + d, c + (r & 1))];
                    int bin_len = bin.size(); 
                    int neighbor_bin_len = neighbor_bin.size();
                    for (int i = 0; i < bin_len; i ++) {
                        for (int j = 0; j < neighbor_bin_len; j ++) {
                            Particle& p0 = particles[bin[i]];
                            Particle& p1 = particles[neighbor_bin[j]];
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

