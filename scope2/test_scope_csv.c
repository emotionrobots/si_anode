/*
 * test_scope_csv.c
 *
 * Demonstrates reading a CSV that matches the format of t11.csv:
 *   first row: labels (x_label,y1_label,y2_label,...)
 *   subsequent rows: numeric values
 *
 * Keys:
 *   ESC / Q : quit
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "scope_plot_dual.h"

static 
void die(const char *msg) 
{
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(1);
}


static 
int split_csv_line(char *line, char **out, int max_out) 
{
    // Simple CSV splitter (no quotes) good enough for numeric logs
    int n = 0;
    char *p = line;
    while (p && *p && n < max_out) 
    {
        // trim leading spaces
        while (*p == ' ' || *p == '\t') p++;
        out[n++] = p;
        char *comma = strchr(p, ',');
        if (!comma) break;
        *comma = '\0';
        p = comma + 1;
    }

    // trim trailing newline/spaces for each token
    for (int i = 0; i < n; i++) 
    {
        char *s = out[i];
        size_t L = strlen(s);
        while (L && (s[L-1] == '\n' || s[L-1] == '\r' || s[L-1] == ' ' || s[L-1] == '\t')) {
            s[L-1] = '\0';
            L--;
        }
    }
    return n;
}



static 
SDL_Color palette(int i) 
{
    static SDL_Color p[] = {
        {255,  80,  80, 255},
        { 80, 255,  80, 255},
        { 80, 160, 255, 255},
        {255, 200,  80, 255},
        {220,  80, 255, 255},
        { 80, 255, 220, 255},
    };
    return p[i % (int)(sizeof(p)/sizeof(p[0]))];
}



int main(int argc, char **argv) 
{
    const char *csv_path = (argc >= 2) ? argv[1] : "t11.csv";
    FILE *f = fopen(csv_path, "r");
    if (!f) die("Failed to open CSV (pass path as argv[1] or put t11.csv in cwd)");

    char line[4096];
    if (!fgets(line, sizeof(line), f)) die("CSV empty");

    // Parse header labels
    char *cols[64] = {0};
    int ncol = split_csv_line(line, cols, 64);
    if (ncol < 2) die("CSV must have at least 2 columns");

    const char *x_label = cols[0];
    int trace_count = ncol - 1;

    scope_trace_desc_t *tr = (scope_trace_desc_t*)calloc((size_t)trace_count, sizeof(*tr));
    if (!tr) die("OOM traces");

    for (int i = 0; i < trace_count; i++) 
    {
        tr[i].name = str_dup(cols[i+1]); // freed by OS on exit; ok for unit test
        tr[i].color = palette(i);
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) die(SDL_GetError());


    SDL_Window *win = SDL_CreateWindow("scope_plot CSV dual-Y demo",
                                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       1100, 650, SDL_WINDOW_SHOWN);
    if (!win) die(SDL_GetError());


    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) die(SDL_GetError());


    scope_plot_cfg_t cfg = scope_plot_default_cfg();
    scope_plot_t *p = scope_plot_create(win, ren, trace_count, tr, &cfg);
    if (!p) die("scope_plot_create failed");

    scope_plot_set_title(p, "CSV: t11.csv (dual Y auto-assign)");
    scope_plot_set_x_label(p, x_label);

    // Read data into plot
    double x_min = 0.0, x_max = 1.0;
    bool first = true;

    double *y = (double*)calloc((size_t)trace_count, sizeof(double));
    if (!y) die("OOM y");

    while (fgets(line, sizeof(line), f)) 
    {
        if (line[0] == '\0' || line[0] == '\n' || line[0] == '\r') continue;

        char *tok[64] = {0};
        int nt = split_csv_line(line, tok, 64);
        if (nt != ncol) continue; // skip malformed rows

        double x = strtod(tok[0], NULL);
        for (int i = 0; i < trace_count; i++) y[i] = strtod(tok[i+1], NULL);

        if (first) { x_min = x_max = x; first = false; }
        else { if (x < x_min) x_min = x; if (x > x_max) x_max = x; }

        scope_plot_push(p, x, y);
    }
    fclose(f);


    // Set x-range to full CSV time span
    scope_plot_set_x_range(p, x_min, x_max);


    // Event loop
    bool quit = false;
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
            if (e.type == SDL_KEYDOWN) {
                SDL_Keycode k = e.key.keysym.sym;
                if (k == SDLK_ESCAPE || k == SDLK_q) quit = true;
            }
        }

        scope_plot_render(p);
        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    scope_plot_destroy(p);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
