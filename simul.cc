#include <omp.h>
#include <semaphore>
#include <bits/stdc++.h>
#include "collision.h"
#include "io.h"
#include "sim_validator.h"
using namespace std;

struct Simulator {
   

    Simulator(int size)  {
        size++;
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
        
       
        for (int i = 0; i < len; i ++) {
            for (int j = i + 1; j < len; j ++) {
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
};

