/*!
 *=====================================================================================================================
 *
 *  @file	test_scope_stream.c
 *
 *  @brief	Demonstrates streaming data (continuous function of time) while scrolling the x-window.
 * 		This test intentionally uses two very different amplitudes to trigger dual-Y auto assignment.
 *
 * 		Keys:
 *   		ESC / Q : quit
 *
 *=====================================================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "scope_plot.h"


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void die(const char *msg) 
 *
 *  @brief	Convenient die function
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
void die(const char *msg) 
{
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(1);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *  Main function
 *---------------------------------------------------------------------------------------------------------------------
 */
int main(void) 
{
    // Init SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) die(SDL_GetError());

    SDL_Window *win = SDL_CreateWindow("scope_plot streaming dual-Y demo",
                                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       1100, 650, SDL_WINDOW_SHOWN);
    if (!win) die(SDL_GetError());

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) die(SDL_GetError());


    // Setup trace label
    scope_trace_desc_t tr[3] = 
    {
        {.name="sin(t) (1.0)",    .color={ 80, 160, 255, 255}},
        {.name="cos(t) (1.0)",    .color={ 80, 255,  80, 255}},
        {.name="100*sin(0.2*t)",  .color={255,  80,  80, 255}},
    };


    // Create the scope object
    scope_plot_cfg_t cfg = scope_plot_default_cfg();
    scope_plot_t *p = scope_plot_create(win, ren, 3, tr, &cfg);
    if (!p) die("scope_plot_create failed");

    // Set title and x_label
    scope_plot_set_title(p, "Streaming: dual Y auto-assign (small vs large amplitude)");
    scope_plot_set_x_label(p, "t (s)");


    // Loop to plot window by continuously pushing new value into plot buffer
    const double dt = 0.01;
    const double window_s = 10.0;

    double t = 0.0;
    bool quit = false;

    while (!quit) 
    {
        SDL_Event e;
        while (SDL_PollEvent(&e)) 
	{
            if (e.type == SDL_QUIT) quit = true;
            if (e.type == SDL_KEYDOWN) 
	    {
                SDL_Keycode k = e.key.keysym.sym;
                if (k == SDLK_ESCAPE || k == SDLK_q) quit = true;
            }
        }

        double y[3];
        y[0] = sin(t);
        y[1] = cos(t);
        y[2] = 100.0 * sin(0.2 * t);

        scope_plot_push(p, t, y);

        // scrolling x-range
        double x0 = t - window_s;
        double x1 = t;
        if (x0 < 0.0) x0 = 0.0;
        scope_plot_set_x_range(p, x0, x1);

        scope_plot_render(p);
        SDL_RenderPresent(ren);

        SDL_Delay((Uint32)(dt * 1000.0));
        t += dt;
    }

    // Clean up on exit
    scope_plot_destroy(p);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
