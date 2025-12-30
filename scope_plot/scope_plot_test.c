#include "scope_plot.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define M_PI 			(3.14159265358979323846)


int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "scope_plot demo (SDL2)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1100, 650,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (!win) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    scope_trace_desc_t traces[3] = {
        { "sin(t)",  (SDL_Color){ 255,  80,  80, 255 } },
        { "cos(t)",  (SDL_Color){  80, 255,  80, 255 } },
        { "ramp",    (SDL_Color){  80,  80, 255, 255 } },
    };

    scope_plot_cfg_t cfg = scope_plot_default_cfg();
    cfg.max_points = 6000;
    cfg.font_px = 14;

    scope_plot_t *plot = scope_plot_create(win, ren, 3, traces, &cfg);
    if (!plot) {
        fprintf(stderr, "scope_plot_create failed (check SDL2_ttf + font path)\n");
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    scope_plot_set_title(plot, "Auto Y-scale | Fixed X-window (caller-controlled)");
    //scope_plot_set_background(plot, (SDL_Color){ 12, 12, 16, 255 });
    scope_plot_set_background(plot, (SDL_Color){ 255, 255, 255, 255 });

    const double x_window = 120.0;     // show last 10 seconds
    double t = 0.0;
    // double dt = 1.0 / 120.0;          // simulation sample step
    double dt = 0.250;          // simulation sample step

    bool running = true;
    bool pause = false;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN) {
		if (e.key.keysym.sym == SDLK_ESCAPE) running = false;
                if (e.key.keysym.sym == SDLK_q) pause = !pause;
            }
        }

	if (!pause)
        {
           // Generate signals
           double y[3];
           y[0] = sin(2.0 * M_PI * 0.05 * t);
           y[1] = cos(2.0 * M_PI * 0.01 * t) * 0.7;
           y[2] = 0.2 * (2.0 * fmod(t, 5.0) - 5.0); // saw-ish ramp in [-1,1] scaled

           // Push sample
           scope_plot_push(plot, t, y);

           // FIXED X axis range (NOT auto scaled):
           // emulate a scope by sliding a fixed-length x-window.
           scope_plot_set_x_range(plot, t - x_window, t);

           // Render
           scope_plot_render(plot);
           SDL_RenderPresent(ren);

           // advance time
           t += dt;

           // keep t from exploding in long runs (purely cosmetic)
           if (t > 1e9) t = 0.0;
	} 
    }

    scope_plot_destroy(plot);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

