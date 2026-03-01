/*!
 *=====================================================================================================================
 *
 *  @file	scope_plot.c
 *
 *=====================================================================================================================
 */
#include "scope_plot.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <SDL2/SDL_ttf.h>


typedef struct 
{
    double *x;      // ring buffer (shared x per trace would be nicer, but simpler per trace: keep x once globally)
    double *y;      // ring buffer
    int head;       // next write index
    int size;       // number of valid samples
    SDL_Color color;
    char *name;
} trace_t;


struct scope_plot 
{
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

    // Dual Y-axes (0=left, 1=right). Auto-assigned each render.
    double y_min[2], y_max[2];
    bool   axis_used[2];
    uint8_t *trace_axis; // length trace_count, values 0 or 1

    scope_plot_cfg_t cfg;
    SDL_Color bg;

    char *title;

    char *x_label;

    // TTF
    bool ttf_inited_here;
    TTF_Font *font;
};


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		char *str_dup(const char *s) 
 *
 *
 *  @brief	string duplicate
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
char *str_dup(const char *s) 
{
    if (!s) return NULL;
    size_t n = strlen(s);
    char *p = (char*)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n + 1);
    return p;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		scope_plot_cfg_t scope_plot_default_cfg(void) 
 *
 *  @brief	Returns default config for scope_plot
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
scope_plot_cfg_t scope_plot_default_cfg(void) 
{
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
    c.margin_right  = 70; // increased for right Y-axis labels
    c.margin_top    = 20;
    c.margin_bottom = 55;

    c.y_padding_frac = 0.05f;
    c.max_points     = 24000000;

    c.ttf_path = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
    c.font_px  = 14;
    return c;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void set_color(SDL_Renderer *r, SDL_Color c) 
 *
 *  @brief	Set color
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
void set_color(SDL_Renderer *r, SDL_Color c) 
{
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void draw_line(SDL_Renderer *r, int x1, int y1, int x2, int y2, SDL_Color c) 
 *
 *  @brief	Draw a line
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
void draw_line(SDL_Renderer *r, int x1, int y1, int x2, int y2, SDL_Color c) 
{
    set_color(r, c);
    SDL_RenderDrawLine(r, x1, y1, x2, y2);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void fill_rect(SDL_Renderer *r, SDL_Rect rc, SDL_Color c) 
 *
 *  @brief	Fill a rectangle
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
void fill_rect(SDL_Renderer *r, SDL_Rect rc, SDL_Color c) 
{
    set_color(r, c);
    SDL_RenderFillRect(r, &rc);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void render_text(SDL_Renderer *r, TTF_Font *font, const char *txt, SDL_Color color, int x, int y) 
 *
 *  @brief	Render text
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
void render_text(SDL_Renderer *r, TTF_Font *font, const char *txt, SDL_Color color, int x, int y) 
{
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


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void render_text_center(SDL_Renderer *r, TTF_Font *font, const char *txt, 
 *                                      SDL_Color color, int cx, int cy) 
 *
 *  @brief	Render text centered
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
void render_text_center(SDL_Renderer *r, TTF_Font *font, const char *txt, SDL_Color color, int cx, int cy) 
{
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

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		bool in_x_window(double x, double xmin, double xmax) 
 *
 *  @brief	Check if x inside window
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
bool in_x_window(double x, double xmin, double xmax) 
{
    return (x >= xmin && x <= xmax);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void compute_y_autoscale(scope_plot_t *p) 
 *
 *  @brief	Compute Y autoscaling
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
void compute_y_autoscale(scope_plot_t *p) 
{
    // Dual-Y auto scale & trace-to-axis assignment based on per-trace data ranges
    // within the current x-window.
    const double EPS = 1e-12;
    const double SPLIT_RATIO = 20.0; // only bother splitting if ranges differ a lot

    double t_min[64], t_max[64], t_rng[64], t_logrng[64];
    if (p->trace_count > 64) 
    {
        // Fallback: keep everything on left for very large trace counts
        for (int t = 0; t < p->trace_count; t++) p->trace_axis[t] = 0;
    } 
    else 
    {
        // per-trace min/max
        for (int t = 0; t < p->trace_count; t++) 
	{
            bool have = false;
            double mn = 0.0, mx = 0.0;

            for (int i = 0; i < p->size; i++) 
	    {
                int idx = (p->head - p->size + i);
                if (idx < 0) idx += p->cap * ((-idx / p->cap) + 1);
                idx %= p->cap;

                double x = p->xbuf[idx];
                if (!in_x_window(x, p->x_min, p->x_max)) continue;

                double y = p->traces[t].y[idx];
                if (!have) { mn = mx = y; have = true; }
                else { if (y < mn) mn = y; if (y > mx) mx = y; }
            }

            if (!have) 
	    { 
	       mn = 0.0; 
	       mx = 0.0; 
	    }

            t_min[t] = mn;
            t_max[t] = mx;
            double rng = t_max[t] - t_min[t];

            if (rng < EPS) rng = EPS;
            t_rng[t] = rng;
            t_logrng[t] = log10(rng);
        }

        // Decide whether dual-axis is warranted
        double rng_min = t_rng[0], rng_max = t_rng[0];
        for (int t = 1; t < p->trace_count; t++) 
	{
            if (t_rng[t] < rng_min) rng_min = t_rng[t];
            if (t_rng[t] > rng_max) rng_max = t_rng[t];
        }

        bool do_split = (rng_max / rng_min) >= SPLIT_RATIO;

        if (!do_split) 
	{
            for (int t = 0; t < p->trace_count; t++) 
	       p->trace_axis[t] = 0;
        } 
	else 
	{
            // 1D k-means (k=2) on log10(range)
            double c1 = t_logrng[0], c2 = t_logrng[0];
            for (int t = 1; t < p->trace_count; t++) 
	    {
                if (t_logrng[t] < c1) c1 = t_logrng[t];
                if (t_logrng[t] > c2) c2 = t_logrng[t];
            }

            for (int it = 0; it < 12; it++) 
	    {
                double s1 = 0.0, s2 = 0.0;
                int n1 = 0, n2 = 0;

                for (int t = 0; t < p->trace_count; t++) 
		{
                    double d1 = fabs(t_logrng[t] - c1);
                    double d2 = fabs(t_logrng[t] - c2);
                    uint8_t a = (d2 < d1) ? 1 : 0;
                    p->trace_axis[t] = a;
                    if (a == 0) 
		    { 
		       s1 += t_logrng[t]; 
		       n1++; 
		    }
                    else 
		    { 
		       s2 += t_logrng[t]; 
		       n2++; 
		    }
                }

                if (n1 > 0) c1 = s1 / (double)n1;
                if (n2 > 0) c2 = s2 / (double)n2;
                if (n1 == 0 || n2 == 0) break;
            }

            // Ensure both axes have at least one trace; if not, split by median
            int n0 = 0, n1 = 0;
            for (int t = 0; t < p->trace_count; t++) 
	       (p->trace_axis[t] == 0) ? n0++ : n1++;

            if (n0 == 0 || n1 == 0) 
	    {
                // median split
                double tmp[64];
                for (int t = 0; t < p->trace_count; t++) 
                   tmp[t] = t_logrng[t];

                // simple selection sort (small n)
                for (int i = 0; i < p->trace_count; i++) 
		{
                   int k = i;
                   for (int j = i+1; j < p->trace_count; j++) 
		      if (tmp[j] < tmp[k]) k = j;

                   double v = tmp[i]; tmp[i] = tmp[k]; tmp[k] = v;
                }

                double med = tmp[p->trace_count/2];
                for (int t = 0; t < p->trace_count; t++) 
                    p->trace_axis[t] = (t_logrng[t] > med) ? 1 : 0;
            }

            // Choose which cluster is left/right:
            // Put the cluster with MORE traces on the left (more readable). If tie, larger-range cluster on left.
            double sum_rng0 = 0.0, sum_rng1 = 0.0;
            n0 = 0; n1 = 0;
            for (int t = 0; t < p->trace_count; t++) 
	    {
                if (p->trace_axis[t] == 0) 
		{ 
	           sum_rng0 += t_rng[t]; 
		   n0++; 
		}
                else
		{ 
	           sum_rng1 += t_rng[t]; 
		   n1++; 
		}
            }

            bool swap = false;
            if (n1 > n0) 
               swap = true;
            else if (n1 == n0 && sum_rng1 > sum_rng0) 
	       swap = true;

            if (swap) 
	    {
                for (int t = 0; t < p->trace_count; t++) 
	           p->trace_axis[t] ^= 1;
            }
        }
    }

    // Compute y min/max per axis from visible samples
    for (int a = 0; a < 2; a++) 
    {
        p->axis_used[a] = false;
        p->y_min[a] = 0.0;
        p->y_max[a] = 0.0;
    }

    for (int a = 0; a < 2; a++) 
    {
        bool have = false;
        double ymin = 0.0, ymax = 0.0;

        for (int i = 0; i < p->size; i++) 
	{
            int idx = (p->head - p->size + i);
            if (idx < 0) idx += p->cap * ((-idx / p->cap) + 1);
            idx %= p->cap;

            double x = p->xbuf[idx];
            if (!in_x_window(x, p->x_min, p->x_max)) continue;

            for (int t = 0; t < p->trace_count; t++) 
	    {
                if ((int)p->trace_axis[t] != a) 
                   continue;

                double y = p->traces[t].y[idx];

                if (!have) 
		{ 
	           ymin = ymax = y; 
		   have = true; 
		}
                else 
		{ 
	           if (y < ymin) 
	              ymin = y; 
		   if (y > ymax) 
	              ymax = y; 
		}
            }
        }

        if (!have) continue;

        if (ymax - ymin < 1e-12) 
	{
            double c = 0.5 * (ymax + ymin);
            ymin = c - 1.0;
            ymax = c + 1.0;
        }

        double span = ymax - ymin;
        double pad = (double)p->cfg.y_padding_frac * span;
        p->y_min[a] = ymin - pad;
        p->y_max[a] = ymax + pad;
        p->axis_used[a] = true;
    }

    // If only one axis has data, mirror it so math doesn't divide by 0
    if (p->axis_used[0] && !p->axis_used[1]) 
    {
        p->y_min[1] = p->y_min[0];
        p->y_max[1] = p->y_max[0];
    } 
    else if (!p->axis_used[0] && p->axis_used[1]) 
    {
        p->y_min[0] = p->y_min[1];
        p->y_max[0] = p->y_max[1];

        // also move traces to left for consistency
        for (int t = 0; t < p->trace_count; t++) 
           p->trace_axis[t] = 0;

        p->axis_used[0] = true;
        p->axis_used[1] = false;
    } 
    else if (!p->axis_used[0] && !p->axis_used[1]) 
    {
        p->y_min[0] = -1.0; p->y_max[0] = 1.0;
        p->y_min[1] = -1.0; p->y_max[1] = 1.0;
        p->axis_used[0] = true;
        p->axis_used[1] = false;
        for (int t = 0; t < p->trace_count; t++) 
           p->trace_axis[t] = 0;
    }
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int map_x(scope_plot_t *p, double x, int plot_x, int plot_w) 
 *
 *  @brief	Map x into plot
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
int map_x(scope_plot_t *p, double x, int plot_x, int plot_w) 
{
    double den = (p->x_max - p->x_min);
    if (fabs(den) < 1e-18) 
       return plot_x;

    double u = (x - p->x_min) / den;
    if (u < 0.0) u = 0.0;
    if (u > 1.0) u = 1.0;
    return plot_x + (int)lround(u * (double)plot_w);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int map_y(scope_plot_t *p, double y, int axis, int plot_y, int plot_h) 
 *
 *  @brief	Map Y into plot
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
int map_y(scope_plot_t *p, double y, int axis, int plot_y, int plot_h) 
{
    double den = (p->y_max[axis] - p->y_min[axis]);
    if (fabs(den) < 1e-18) return plot_y + plot_h/2;
    double v = (y - p->y_min[axis]) / den; // 0..1 bottom->top
    if (v < 0.0) v = 0.0;
    if (v > 1.0) v = 1.0;
    return plot_y + plot_h - (int)lround(v * (double)plot_h);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void draw_grid(scope_plot_t *p, SDL_Rect pr) 
 *
 *  @brief	Draw plot grids
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
void draw_grid(scope_plot_t *p, SDL_Rect pr) 
{
    // Minor grid
    int major_x = (p->cfg.grid_major_div_x > 0) ? p->cfg.grid_major_div_x : 10;
    int major_y = (p->cfg.grid_major_div_y > 0) ? p->cfg.grid_major_div_y : 10;
    int minor   = (p->cfg.grid_minor_div   > 0) ? p->cfg.grid_minor_div   : 5;

    // Vertical lines
    for (int mx = 0; mx <= major_x; mx++) 
    {
        int x = pr.x + (int)lround((double)mx * pr.w / (double)major_x);

        // minor subdivisions between this and next major
        if (mx < major_x) 
	{
            int x_next = pr.x + (int)lround((double)(mx+1) * pr.w / (double)major_x);
            for (int k = 1; k < minor; k++) 
	    {
                int xm = x + (int)lround((double)k * (x_next - x) / (double)minor);
                draw_line(p->ren, xm, pr.y, xm, pr.y + pr.h, p->cfg.grid_minor);
            }
        }
        draw_line(p->ren, x, pr.y, x, pr.y + pr.h, p->cfg.grid_major);
    }

    // Horizontal lines
    for (int my = 0; my <= major_y; my++) 
    {
        int y = pr.y + (int)lround((double)my * pr.h / (double)major_y);

        if (my < major_y) 
	{
            int y_next = pr.y + (int)lround((double)(my+1) * pr.h / (double)major_y);
            for (int k = 1; k < minor; k++) 
	    {
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


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void draw_axis_labels(scope_plot_t *p, SDL_Rect pr) 
 *
 *  @brief	Draw axis labels
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
void draw_axis_labels(scope_plot_t *p, SDL_Rect pr) 
{
    int major_x = (p->cfg.grid_major_div_x > 0) ? p->cfg.grid_major_div_x : 10;
    int major_y = (p->cfg.grid_major_div_y > 0) ? p->cfg.grid_major_div_y : 10;

    // X labels (bottom)
    for (int i = 0; i <= major_x; i++) 
    {
        double x = p->x_min + (p->x_max - p->x_min) * ((double)i / (double)major_x);
        int sx = pr.x + (int)lround((double)i * pr.w / (double)major_x);

        char buf[64];
        snprintf(buf, sizeof(buf), "%.3g", x);

        render_text_center(p->ren, p->font, buf, p->cfg.text, sx, pr.y + pr.h + 18);

        // tick mark
        draw_line(p->ren, sx, pr.y + pr.h, sx, pr.y + pr.h + 6, p->cfg.axis);
    }

    // X axis label (centered below ticks)
    if (p->x_label) 
    {
        render_text_center(p->ren, p->font, p->x_label, p->cfg.text,
                           pr.x + pr.w/2, pr.y + pr.h + 38);
    }

    // Y labels (left, axis 0)
    for (int i = 0; i <= major_y; i++) 
    {
        double y = p->y_max[0] - (p->y_max[0] - p->y_min[0]) * ((double)i / (double)major_y);
        int sy = pr.y + (int)lround((double)i * pr.h / (double)major_y);

        char buf[64];
        snprintf(buf, sizeof(buf), "%.3g", y);

        render_text(p->ren, p->font, buf, p->cfg.text, pr.x - p->cfg.margin_left + 8, sy - 8);
        draw_line(p->ren, pr.x - 6, sy, pr.x, sy, p->cfg.axis);
    }

    // Y labels (right, axis 1) if used
    if (p->axis_used[1]) 
    {
        for (int i = 0; i <= major_y; i++) 
	{
            double y = p->y_max[1] - (p->y_max[1] - p->y_min[1]) * ((double)i / (double)major_y);
            int sy = pr.y + (int)lround((double)i * pr.h / (double)major_y);

            char buf[64];
            snprintf(buf, sizeof(buf), "%.3g", y);

            render_text(p->ren, p->font, buf, p->cfg.text, pr.x + pr.w + 10, sy - 8);
            draw_line(p->ren, pr.x + pr.w, sy, pr.x + pr.w + 6, sy, p->cfg.axis);
        }
    }

    if (p->title) 
    {
        render_text(p->ren, p->font, p->title, p->cfg.text, pr.x + 6, pr.y + 6);
    }
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void draw_legend(scope_plot_t *p, SDL_Rect pr) 
 *
 *  @brief	Draw legend
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
void draw_legend(scope_plot_t *p, SDL_Rect pr) 
{
    // Legend box top-right inside plot
    const int pad = 8;
    const int swatch = 18;
    const int line_h = p->cfg.font_px + 6;

    bool show_axis_tag = p->axis_used[1];

    // Measure width roughly
    int max_chars = 0;
    for (int t = 0; t < p->trace_count; t++) 
    {
        const char *nm = p->traces[t].name ? p->traces[t].name : "(null)";
        int n = (int)strlen(nm) + (show_axis_tag ? 4 : 0); // " [R]"
        if (n > max_chars) max_chars = n;
    }
    int box_w = pad*3 + swatch + max_chars * (p->cfg.font_px/2 + 1);
    int box_h = pad*2 + p->trace_count * line_h;

    SDL_Rect box = 
    {
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

    for (int t = 0; t < p->trace_count; t++) 
    {
        int y = box.y + pad + t * line_h + (line_h/2);
        int x0 = box.x + pad;

        // swatch line
        draw_line(p->ren, x0, y, x0 + swatch, y, p->traces[t].color);

        // name (optionally with axis tag)
        char buf[256];
        const char *nm = p->traces[t].name ? p->traces[t].name : "(null)";
        if (show_axis_tag) 
	{
            snprintf(buf, sizeof(buf), "%s [%c]", nm, (p->trace_axis[t] ? 'R' : 'L'));
        } 
	else 
	{
            snprintf(buf, sizeof(buf), "%s", nm);
        }

        render_text(p->ren, p->font, buf, p->cfg.text, x0 + swatch + pad, y - (p->cfg.font_px/2));
    }
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		scope_plot_t *scope_plot_create(SDL_Window *win, SDL_Renderer *ren, int trace_count,
 *                                              const scope_trace_desc_t *traces, const scope_plot_cfg_t *cfg)
 *
 *  @brief	Create the plot
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
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
    if (!p->xbuf) 
    { 
       free(p); 
       return NULL; 
    }

    p->traces = (trace_t*)calloc((size_t)trace_count, sizeof(trace_t));
    if (!p->traces) 
    { 
       free(p->xbuf); 
       free(p); 
       return NULL; 
    }

    for (int i = 0; i < trace_count; i++) 
    {
       p->traces[i].y = (double*)calloc((size_t)p->cap, sizeof(double));
       if (!p->traces[i].y) 
       {
          for (int k = 0; k < i; k++) 
             free(p->traces[k].y);

          free(p->traces);
          free(p->xbuf);
          free(p);
          return NULL;
       }

       p->trace_axis = (uint8_t*)calloc((size_t)trace_count, sizeof(uint8_t));
       if (!p->trace_axis) 
       {
          free(p->xbuf);
          free(p->traces);
          free(p);
          return NULL;
       }

       p->axis_used[0] = true;
       p->axis_used[1] = false;
       p->y_min[0] = -1.0; p->y_max[0] = 1.0;
       p->y_min[1] = -1.0; p->y_max[1] = 1.0;
       p->traces[i].color = traces[i].color;
       p->traces[i].name  = str_dup(traces[i].name ? traces[i].name : "");

       if (!p->traces[i].name) 
       {
          for (int k = 0; k <= i; k++) 
	  { 
	     free(p->traces[k].y); 
	     free(p->traces[k].name); 
	  }
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
    if (TTF_WasInit() == 0) 
    {
       if (TTF_Init() != 0) 
       {
          scope_plot_destroy(p);
          return NULL;
       }
       p->ttf_inited_here = true;
    }

    p->font = TTF_OpenFont(p->cfg.ttf_path, p->cfg.font_px);
    if (!p->font) 
    {
       scope_plot_destroy(p);
       return NULL;
    }

    return p;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void scope_plot_destroy(scope_plot_t *p) 
 *
 *  @brief	Destroy plot
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void scope_plot_destroy(scope_plot_t *p) 
{
    if (!p) return;

    if (p->font) 
    {
        TTF_CloseFont(p->font);
        p->font = NULL;
    }
    if (p->ttf_inited_here) 
    {
        TTF_Quit();
        p->ttf_inited_here = false;
    }

    if (p->traces) 
    {
        for (int i = 0; i < p->trace_count; i++) 
	{
            free(p->traces[i].y);
            free(p->traces[i].name);
        }
        free(p->traces);
    }
    free(p->xbuf);
    free(p->title);
    free(p);
}

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void scope_plot_set_x_range(scope_plot_t *p, double x_min, double x_max) 
 *
 *  @brief	Set X range
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void scope_plot_set_x_range(scope_plot_t *p, double x_min, double x_max) 
{
    if (!p) return;
    if (x_max == x_min) x_max = x_min + 1.0;
    if (x_max < x_min) 
    {
        double tmp = x_min; x_min = x_max; x_max = tmp;
    }
    p->x_min = x_min;
    p->x_max = x_max;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void scope_plot_set_title(scope_plot_t *p, const char *title) 
 *
 *  @brief	Set plot title
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void scope_plot_set_title(scope_plot_t *p, const char *title) 
{
    if (!p) return;
    free(p->title);
    p->title = str_dup(title);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void scope_plot_set_x_label(scope_plot_t *p, const char *x_label) 
 *
 *  @brief	Set X label
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void scope_plot_set_x_label(scope_plot_t *p, const char *x_label) 
{
    if (!p) return;
    if (p->x_label) 
    { 
       free(p->x_label); 
       p->x_label = NULL; 
    }
    if (x_label) p->x_label = str_dup(x_label);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void scope_plot_set_background(scope_plot_t *p, SDL_Color bg) 
 *
 *  @brief	Set plot background color
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void scope_plot_set_background(scope_plot_t *p, SDL_Color bg) 
{
    if (!p) return;
    p->bg = bg;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		bool scope_plot_push(scope_plot_t *p, double x, const double *y) 
 *
 *  @brief	Push data into plot buffer
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
bool scope_plot_push(scope_plot_t *p, double x, const double *y) 
{
    if (!p || !y) return false;

    // Write at head
    p->xbuf[p->head] = x;
    for (int t = 0; t < p->trace_count; t++) 
    {
        p->traces[t].y[p->head] = y[t];
    }

    p->head = (p->head + 1) % p->cap;
    if (p->size < p->cap) p->size++;

    return true;
}

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void draw_traces(scope_plot_t *p, SDL_Rect pr) 
 *
 *  @brief	Draw traces
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
void draw_traces(scope_plot_t *p, SDL_Rect pr) 
{
   // For each trace: connect successive visible points
   for (int t = 0; t < p->trace_count; t++) 
   {
      bool have_prev = false;
      int px_prev = 0, py_prev = 0;

      for (int i = 0; i < p->size; i++) 
      {
         int idx = (p->head - p->size + i);
         if (idx < 0) idx += p->cap * ((-idx / p->cap) + 1);
         idx %= p->cap;

         double x = p->xbuf[idx];
         if (!in_x_window(x, p->x_min, p->x_max)) 
	 {
            have_prev = false;
            continue;
         }

         double y = p->traces[t].y[idx];
         int px = map_x(p, x, pr.x, pr.w);
         int axis = (int)p->trace_axis[t];
         int py = map_y(p, y, axis, pr.y, pr.h);

         if (have_prev) 
	 {
            draw_line(p->ren, px_prev, py_prev, px, py, p->traces[t].color);
         }
         px_prev = px; py_prev = py;
         have_prev = true;
      }
   }
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void scope_plot_render(scope_plot_t *p) 
 *
 *  @brief	Render plot
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void scope_plot_render(scope_plot_t *p) 
{
    if (!p) return;

    int w = 0, h = 0;
    SDL_GetWindowSize(p->win, &w, &h);

    // Full background
    set_color(p->ren, p->bg);
    SDL_RenderClear(p->ren);

    SDL_Rect pr = 
    {
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

