#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#define NUM_BODIES 20
#define WINDOW_W 800
#define WINDOW_H 600

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
            force = -6.674e-11 * (bodies[m1].mass * bodies[m2].mass / distance);

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

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* texture = NULL;

    // Init sdl and test
    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        fprintf(stderr, "SDL initialisation failed: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window and test
    window = SDL_CreateWindow(
        "N Body simulator",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        fprintf(stderr, "SDL window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // We have a valid window now. Let's create a renderer.
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL){
        fprintf(stderr, "SDL renderer creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    body bodies[NUM_BODIES];

    for (size_t i = 0; i < NUM_BODIES; i++){
        bodies[i].rx = rand() % WINDOW_W;
        bodies[i].ry = rand() % WINDOW_H;

        bodies[i].vx = 0;
        bodies[i].vy = 0;

        bodies[i].radius = rand() % 7;
        bodies[i].mass = bodies[i].radius * bodies[i].radius * M_PI;
    }

    // Loop and handle events
    SDL_Event event;
    bool running = true;
    while (running){
        while (SDL_PollEvent(&event) != 0){
            if (event.type == SDL_QUIT){
                running = false;
            }
            else if (event.type == SDL_KEYDOWN){
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                    case SDLK_SPACE:
                    case SDLK_RETURN:
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    default:
                        break;
                }
            }
            else {}
        }

        // Draw
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (size_t i = 0; i < NUM_BODIES; i++){
            filledCircleRGBA(renderer, bodies[i].rx, bodies[i].ry, bodies[i].radius, 255, 0, 255, 255);
        }
        SDL_RenderPresent(renderer);
        step(bodies, NUM_BODIES, 1000);
//        printf("New step\n");
//        for (size_t i = 0; i < NUM_BODIES; i++){
//            printf("Body %zu: l: (%3.5f,%3.5f), v: (%3.5f,%3.5f)\n", i, bodies[i].rx, bodies[i].ry, bodies[i].vx, bodies[i].vy);
//        }
//        printf("\n");
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
