#include "scope_plot.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <SDL2/SDL_ttf.h>

typedef struct {
    double *x;      // ring buffer (shared x per trace would be nicer, but simpler per trace: keep x once globally)
    double *y;      // ring buffer
    int head;       // next write index
    int size;       // number of valid samples
    SDL_Color color;
    char *name;
} trace_t;

struct scope_plot {
    SDL_Window   *win;
    SDL_Renderer *ren;

    int trace_count;
    trace_t *traces;

    // We store X once, shared across traces, to ensure alignment
    double *xbuf;
    int head;
    int size;
    int cap;

    double x_min, x_max;   // caller-controlled
    double y_min, y_max;   // auto (computed each render)

    scope_plot_cfg_t cfg;
    SDL_Color bg;

    char *title;

    // TTF
    bool ttf_inited_here;
    TTF_Font *font;
};

static char *str_dup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char *p = (char*)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n + 1);
    return p;
}

scope_plot_cfg_t scope_plot_default_cfg(void) {
    scope_plot_cfg_t c;
    c.background   = (SDL_Color){  18,  18,  18, 255 };
    c.grid_major   = (SDL_Color){  80,  80,  80, 255 };
    c.grid_minor   = (SDL_Color){  45,  45,  45, 255 };
    c.axis         = (SDL_Color){ 180, 180, 180, 255 };
    c.text         = (SDL_Color){ 220, 220, 220, 255 };
    c.legend_bg    = (SDL_Color){  25,  25,  25, 220 };

    c.grid_major_div_x = 10;
    c.grid_major_div_y = 10;
    c.grid_minor_div   = 5;

    c.margin_left   = 70;
    c.margin_right  = 20;
    c.margin_top    = 20;
    c.margin_bottom = 55;

    c.y_padding_frac = 0.05f;
    c.max_points     = 4000;

    c.ttf_path = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
    c.font_px  = 14;
    return c;
}

static void set_color(SDL_Renderer *r, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
}

static void draw_line(SDL_Renderer *r, int x1, int y1, int x2, int y2, SDL_Color c) {
    set_color(r, c);
    SDL_RenderDrawLine(r, x1, y1, x2, y2);
}

static void fill_rect(SDL_Renderer *r, SDL_Rect rc, SDL_Color c) {
    set_color(r, c);
    SDL_RenderFillRect(r, &rc);
}

static void render_text(SDL_Renderer *r, TTF_Font *font, const char *txt,
                        SDL_Color color, int x, int y) {
    if (!font || !txt) return;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, txt, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    if (!tex) { SDL_FreeSurface(surf); return; }
    SDL_Rect dst = { x, y, surf->w, surf->h };
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

static void render_text_center(SDL_Renderer *r, TTF_Font *font, const char *txt,
                               SDL_Color color, int cx, int cy) {
    if (!font || !txt) return;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, txt, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    if (!tex) { SDL_FreeSurface(surf); return; }
    SDL_Rect dst = { cx - surf->w/2, cy - surf->h/2, surf->w, surf->h };
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

static bool in_x_window(double x, double xmin, double xmax) {
    return (x >= xmin && x <= xmax);
}

static void compute_y_autoscale(scope_plot_t *p) {
    // Find min/max y among samples that fall within [x_min, x_max]
    bool have = false;
    double ymin = 0.0, ymax = 0.0;

    for (int i = 0; i < p->size; i++) {
        int idx = (p->head - p->size + i);
        if (idx < 0) idx += p->cap * ((-idx / p->cap) + 1);
        idx %= p->cap;

        double x = p->xbuf[idx];
        if (!in_x_window(x, p->x_min, p->x_max)) continue;

        for (int t = 0; t < p->trace_count; t++) {
            double y = p->traces[t].y[idx];
            if (!have) { ymin = ymax = y; have = true; }
            else {
                if (y < ymin) ymin = y;
                if (y > ymax) ymax = y;
            }
        }
    }

    if (!have) {
        p->y_min = -1.0;
        p->y_max =  1.0;
        return;
    }

    if (ymax - ymin < 1e-12) {
        // flat line: expand a bit
        double c = 0.5 * (ymax + ymin);
        ymin = c - 1.0;
        ymax = c + 1.0;
    }

    double span = ymax - ymin;
    double pad = (double)p->cfg.y_padding_frac * span;
    p->y_min = ymin - pad;
    p->y_max = ymax + pad;
}

static int map_x(scope_plot_t *p, double x, int plot_x, int plot_w) {
    double den = (p->x_max - p->x_min);
    if (fabs(den) < 1e-18) return plot_x;
    double u = (x - p->x_min) / den;
    if (u < 0.0) u = 0.0;
    if (u > 1.0) u = 1.0;
    return plot_x + (int)lround(u * (double)plot_w);
}

static int map_y(scope_plot_t *p, double y, int plot_y, int plot_h) {
    double den = (p->y_max - p->y_min);
    if (fabs(den) < 1e-18) return plot_y + plot_h/2;
    double v = (y - p->y_min) / den; // 0..1 bottom->top
    if (v < 0.0) v = 0.0;
    if (v > 1.0) v = 1.0;
    // screen y grows downward, so invert
    return plot_y + plot_h - (int)lround(v * (double)plot_h);
}

static void draw_grid(scope_plot_t *p, SDL_Rect pr) {
    // Minor grid
    int major_x = (p->cfg.grid_major_div_x > 0) ? p->cfg.grid_major_div_x : 10;
    int major_y = (p->cfg.grid_major_div_y > 0) ? p->cfg.grid_major_div_y : 10;
    int minor   = (p->cfg.grid_minor_div   > 0) ? p->cfg.grid_minor_div   : 5;

    // Vertical lines
    for (int mx = 0; mx <= major_x; mx++) {
        int x = pr.x + (int)lround((double)mx * pr.w / (double)major_x);

        // minor subdivisions between this and next major
        if (mx < major_x) {
            int x_next = pr.x + (int)lround((double)(mx+1) * pr.w / (double)major_x);
            for (int k = 1; k < minor; k++) {
                int xm = x + (int)lround((double)k * (x_next - x) / (double)minor);
                draw_line(p->ren, xm, pr.y, xm, pr.y + pr.h, p->cfg.grid_minor);
            }
        }
        draw_line(p->ren, x, pr.y, x, pr.y + pr.h, p->cfg.grid_major);
    }

    // Horizontal lines
    for (int my = 0; my <= major_y; my++) {
        int y = pr.y + (int)lround((double)my * pr.h / (double)major_y);

        if (my < major_y) {
            int y_next = pr.y + (int)lround((double)(my+1) * pr.h / (double)major_y);
            for (int k = 1; k < minor; k++) {
                int ym = y + (int)lround((double)k * (y_next - y) / (double)minor);
                draw_line(p->ren, pr.x, ym, pr.x + pr.w, ym, p->cfg.grid_minor);
            }
        }
        draw_line(p->ren, pr.x, y, pr.x + pr.w, y, p->cfg.grid_major);
    }

    // Plot border (axis)
    draw_line(p->ren, pr.x, pr.y, pr.x + pr.w, pr.y, p->cfg.axis);
    draw_line(p->ren, pr.x, pr.y + pr.h, pr.x + pr.w, pr.y + pr.h, p->cfg.axis);
    draw_line(p->ren, pr.x, pr.y, pr.x, pr.y + pr.h, p->cfg.axis);
    draw_line(p->ren, pr.x + pr.w, pr.y, pr.x + pr.w, pr.y + pr.h, p->cfg.axis);
}

static void draw_axis_labels(scope_plot_t *p, SDL_Rect pr) {
    int major_x = (p->cfg.grid_major_div_x > 0) ? p->cfg.grid_major_div_x : 10;
    int major_y = (p->cfg.grid_major_div_y > 0) ? p->cfg.grid_major_div_y : 10;

    // X labels (bottom)
    for (int i = 0; i <= major_x; i++) {
        double x = p->x_min + (p->x_max - p->x_min) * ((double)i / (double)major_x);
        int sx = pr.x + (int)lround((double)i * pr.w / (double)major_x);

        char buf[64];
        snprintf(buf, sizeof(buf), "%.3g", x);

        render_text_center(p->ren, p->font, buf, p->cfg.text, sx, pr.y + pr.h + 18);

        // tick mark
        draw_line(p->ren, sx, pr.y + pr.h, sx, pr.y + pr.h + 6, p->cfg.axis);
    }

    // Y labels (left)
    for (int i = 0; i <= major_y; i++) {
        // i=0 top, i=major_y bottom
        double y = p->y_max - (p->y_max - p->y_min) * ((double)i / (double)major_y);
        int sy = pr.y + (int)lround((double)i * pr.h / (double)major_y);

        char buf[64];
        snprintf(buf, sizeof(buf), "%.3g", y);

        // right-aligned-ish: render then shift by width is annoying; quick approx:
        render_text(p->ren, p->font, buf, p->cfg.text, pr.x - p->cfg.margin_left + 8, sy - 8);

        // tick
        draw_line(p->ren, pr.x - 6, sy, pr.x, sy, p->cfg.axis);
    }

    if (p->title) {
        render_text(p->ren, p->font, p->title, p->cfg.text, pr.x + 6, pr.y + 6);
    }
}

static void draw_legend(scope_plot_t *p, SDL_Rect pr) {
    // Legend box top-right inside plot
    const int pad = 8;
    const int swatch = 18;
    const int line_h = p->cfg.font_px + 6;

    // Measure width roughly
    int max_chars = 0;
    for (int t = 0; t < p->trace_count; t++) {
        int n = (p->traces[t].name) ? (int)strlen(p->traces[t].name) : 0;
        if (n > max_chars) max_chars = n;
    }
    int box_w = pad*3 + swatch + max_chars * (p->cfg.font_px/2 + 1);
    int box_h = pad*2 + p->trace_count * line_h;

    SDL_Rect box = {
        pr.x + pr.w - box_w - 8,
        pr.y + 8,
        box_w,
        box_h
    };
    fill_rect(p->ren, box, p->cfg.legend_bg);

    // border
    draw_line(p->ren, box.x, box.y, box.x + box.w, box.y, p->cfg.axis);
    draw_line(p->ren, box.x, box.y + box.h, box.x + box.w, box.y + box.h, p->cfg.axis);
    draw_line(p->ren, box.x, box.y, box.x, box.y + box.h, p->cfg.axis);
    draw_line(p->ren, box.x + box.w, box.y, box.x + box.w, box.y + box.h, p->cfg.axis);

    for (int t = 0; t < p->trace_count; t++) {
        int y = box.y + pad + t * line_h + (line_h/2);
        int x0 = box.x + pad;

        // swatch line
        draw_line(p->ren, x0, y, x0 + swatch, y, p->traces[t].color);

        // name
        render_text(p->ren, p->font,
                    p->traces[t].name ? p->traces[t].name : "(null)",
                    p->cfg.text,
                    x0 + swatch + pad, y - (p->cfg.font_px/2));
    }
}

scope_plot_t *scope_plot_create(SDL_Window *win,
                                SDL_Renderer *ren,
                                int trace_count,
                                const scope_trace_desc_t *traces,
                                const scope_plot_cfg_t *cfg)
{
    if (!win || !ren || trace_count <= 0 || !traces) return NULL;

    scope_plot_t *p = (scope_plot_t*)calloc(1, sizeof(*p));
    if (!p) return NULL;

    p->win = win;
    p->ren = ren;
    p->trace_count = trace_count;

    p->cfg = cfg ? *cfg : scope_plot_default_cfg();
    p->bg  = p->cfg.background;

    p->cap = (p->cfg.max_points > 16) ? p->cfg.max_points : 1024;
    p->xbuf = (double*)calloc((size_t)p->cap, sizeof(double));
    if (!p->xbuf) { free(p); return NULL; }

    p->traces = (trace_t*)calloc((size_t)trace_count, sizeof(trace_t));
    if (!p->traces) { free(p->xbuf); free(p); return NULL; }

    for (int i = 0; i < trace_count; i++) {
        p->traces[i].y = (double*)calloc((size_t)p->cap, sizeof(double));
        if (!p->traces[i].y) {
            for (int k = 0; k < i; k++) free(p->traces[k].y);
            free(p->traces);
            free(p->xbuf);
            free(p);
            return NULL;
        }
        p->traces[i].color = traces[i].color;
        p->traces[i].name  = str_dup(traces[i].name ? traces[i].name : "");
        if (!p->traces[i].name) {
            for (int k = 0; k <= i; k++) { free(p->traces[k].y); free(p->traces[k].name); }
            free(p->traces);
            free(p->xbuf);
            free(p);
            return NULL;
        }
    }

    // X defaults
    p->x_min = 0.0;
    p->x_max = 1.0;

    // TTF init
    if (TTF_WasInit() == 0) {
        if (TTF_Init() != 0) {
            scope_plot_destroy(p);
            return NULL;
        }
        p->ttf_inited_here = true;
    }
    p->font = TTF_OpenFont(p->cfg.ttf_path, p->cfg.font_px);
    if (!p->font) {
        scope_plot_destroy(p);
        return NULL;
    }

    return p;
}

void scope_plot_destroy(scope_plot_t *p) {
    if (!p) return;

    if (p->font) {
        TTF_CloseFont(p->font);
        p->font = NULL;
    }
    if (p->ttf_inited_here) {
        TTF_Quit();
        p->ttf_inited_here = false;
    }

    if (p->traces) {
        for (int i = 0; i < p->trace_count; i++) {
            free(p->traces[i].y);
            free(p->traces[i].name);
        }
        free(p->traces);
    }
    free(p->xbuf);
    free(p->title);
    free(p);
}

void scope_plot_set_x_range(scope_plot_t *p, double x_min, double x_max) {
    if (!p) return;
    if (x_max == x_min) x_max = x_min + 1.0;
    if (x_max < x_min) {
        double tmp = x_min; x_min = x_max; x_max = tmp;
    }
    p->x_min = x_min;
    p->x_max = x_max;
}

void scope_plot_set_title(scope_plot_t *p, const char *title) {
    if (!p) return;
    free(p->title);
    p->title = str_dup(title);
}

void scope_plot_set_background(scope_plot_t *p, SDL_Color bg) {
    if (!p) return;
    p->bg = bg;
}

bool scope_plot_push(scope_plot_t *p, double x, const double *y) {
    if (!p || !y) return false;

    // Write at head
    p->xbuf[p->head] = x;
    for (int t = 0; t < p->trace_count; t++) {
        p->traces[t].y[p->head] = y[t];
    }

    p->head = (p->head + 1) % p->cap;
    if (p->size < p->cap) p->size++;
    return true;
}

static void draw_traces(scope_plot_t *p, SDL_Rect pr) {
    // For each trace: connect successive visible points
    for (int t = 0; t < p->trace_count; t++) {
        bool have_prev = false;
        int px_prev = 0, py_prev = 0;

        for (int i = 0; i < p->size; i++) {
            int idx = (p->head - p->size + i);
            if (idx < 0) idx += p->cap * ((-idx / p->cap) + 1);
            idx %= p->cap;

            double x = p->xbuf[idx];
            if (!in_x_window(x, p->x_min, p->x_max)) {
                have_prev = false;
                continue;
            }

            double y = p->traces[t].y[idx];
            int px = map_x(p, x, pr.x, pr.w);
            int py = map_y(p, y, pr.y, pr.h);

            if (have_prev) {
                draw_line(p->ren, px_prev, py_prev, px, py, p->traces[t].color);
            }
            px_prev = px; py_prev = py;
            have_prev = true;
        }
    }
}

void scope_plot_render(scope_plot_t *p) {
    if (!p) return;

    int w = 0, h = 0;
    SDL_GetWindowSize(p->win, &w, &h);

    // Full background
    set_color(p->ren, p->bg);
    SDL_RenderClear(p->ren);

    SDL_Rect pr = {
        p->cfg.margin_left,
        p->cfg.margin_top,
        w - p->cfg.margin_left - p->cfg.margin_right,
        h - p->cfg.margin_top - p->cfg.margin_bottom
    };
    if (pr.w < 50 || pr.h < 50) return;

    // auto y-scale each frame
    compute_y_autoscale(p);

    draw_grid(p, pr);
    draw_traces(p, pr);
    draw_axis_labels(p, pr);
    draw_legend(p, pr);
}

