#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define M_PI 3.14159265358979323846
#define NUM_BODIES 500
#define NUM_STEPS 10

typedef struct {
    double rx;
    double ry;

    double vx;
    double vy;

    unsigned int radius;
    double mass;
} body;

void step(body bodies[], size_t num_bodies, size_t num_steps){
    double fx;  // force x
    double fy;
    double force;

    double d_vx;    // acceleration x
    double d_vy;

    double dx;  // distance x
    double dy;
    double distance;

    for (size_t step = 0; step < num_steps; step++)
    for (size_t m1 = 0; m1 < num_bodies; m1++){
        // Reset force accumulators
        fx = 0;
        fy = 0;

        // Accumulate forces
        for (size_t m2 = 0; m2 < num_bodies; m2++){
            if (m1 == m2) { continue; }
            dx = bodies[m1].rx - bodies[m2].rx;
            dy = bodies[m1].ry - bodies[m2].ry;

            distance = sqrt(dx*dx + dy*dy);
            force = 6.674e-11 * (bodies[m1].mass * bodies[m2].mass / distance);

            fx += force * (dx / distance);
            fy += force * (dy / distance);
        }

        // acceleration = force / mass
        d_vx = fx / bodies[m1].mass;
        d_vy = fy / bodies[m1].mass;

        // speed = time * acceleration
        bodies[m1].vx += 1 * fx;
        bodies[m1].vy += 1 * fy;

        // distance = time * speed
        bodies[m1].rx += 1 * bodies[m1].vx; // We just did a double x/y add?!
        bodies[m1].ry += 1 * bodies[m1].vy; // Oh, distance is the double-integration of acceleration!
    }
}

int main(){
    srand(time(NULL));

    body bodies[NUM_BODIES];

    for (size_t i = 0; i < NUM_BODIES; i++){
        bodies[i].rx = (rand() % 201) - 100; // -100 - 100
        bodies[i].ry = (rand() % 201) - 100;

        bodies[i].vx = 0;
        bodies[i].vy = 0;

        bodies[i].radius = rand() % 10;
        bodies[i].mass = bodies[i].radius * bodies[i].radius * M_PI;
    }

    for (size_t i = 0; i < NUM_STEPS; i++){
        printf("Step %zu.\n", i);
        step(bodies, NUM_BODIES, 1000);
    }

    return 0;
}
