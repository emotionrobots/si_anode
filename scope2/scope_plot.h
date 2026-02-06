/*!
 *=====================================================================================================================
 * 
 *  @file 	scope_plot.h
 *
 *  @brief	Scope header
 *
 *=====================================================================================================================
 */
#ifndef __SCOPE_PLOT_H__
#define __SCOPE_PLOT_H__

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct scope_plot scope_plot_t;

typedef struct 
{
    const char *name;      // legend label
    SDL_Color   color;     // trace color
} 
scope_trace_desc_t;


typedef struct 
{
    SDL_Color background;
    SDL_Color grid_major;
    SDL_Color grid_minor;
    SDL_Color axis;
    SDL_Color text;
    SDL_Color legend_bg;

    int grid_major_div_x;  // e.g., 10
    int grid_major_div_y;  // e.g., 10
    int grid_minor_div;    // minor subdivisions per major (e.g., 5)

    int margin_left;
    int margin_right;
    int margin_top;
    int margin_bottom;

    float y_padding_frac;  // e.g., 0.05 -> 5% padding
    int max_points;        // ring buffer capacity per trace

    const char *ttf_path;  // font path (e.g., /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf)
    int font_px;           // e.g., 14
} 
scope_plot_cfg_t;

scope_plot_cfg_t scope_plot_default_cfg(void);


/*!
 *---------------------------------------------------------------------------------------------------------------------
 * Create a plot with N traces.
 * - window/renderer are owned by the caller (plot does not destroy them).
 * - plot owns its internal buffers + font resources.
 *---------------------------------------------------------------------------------------------------------------------
 */
scope_plot_t *scope_plot_create(SDL_Window *win,
                                SDL_Renderer *ren,
                                int trace_count,
                                const scope_trace_desc_t *traces,
                                const scope_plot_cfg_t *cfg);

void scope_plot_destroy(scope_plot_t *p);


/*!
 *---------------------------------------------------------------------------------------------------------------------
 * Set the x-axis range (NOT auto-scaled). 
 *---------------------------------------------------------------------------------------------------------------------
 */
void scope_plot_set_x_range(scope_plot_t *p, double x_min, double x_max);


/*!
 *---------------------------------------------------------------------------------------------------------------------
 * Optional: set plot area title shown top-left (may be NULL). 
 *---------------------------------------------------------------------------------------------------------------------
 */
void scope_plot_set_title(scope_plot_t *p, const char *title);

/*!
 *---------------------------------------------------------------------------------------------------------------------
 * Optional: set x-axis label shown centered below the axis (may be NULL). 
 *---------------------------------------------------------------------------------------------------------------------
 */
void scope_plot_set_x_label(scope_plot_t *p, const char *x_label);

/*!
 *---------------------------------------------------------------------------------------------------------------------
 * Background color setter (overrides cfg background). 
 *---------------------------------------------------------------------------------------------------------------------
 */
void scope_plot_set_background(scope_plot_t *p, SDL_Color bg);

/*!
 *---------------------------------------------------------------------------------------------------------------------
 * Push one sample:
 * - x is the independent variable
 * - y[] must have trace_count elements
 *---------------------------------------------------------------------------------------------------------------------
 */
bool scope_plot_push(scope_plot_t *p, double x, const double *y);

/*!
 *---------------------------------------------------------------------------------------------------------------------
 * Render the plot into the current renderer target. 
 *---------------------------------------------------------------------------------------------------------------------
 */
void scope_plot_render(scope_plot_t *p);

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *  Duplicate string
 *---------------------------------------------------------------------------------------------------------------------
 */
char *str_dup(const char *s);

#ifdef __cplusplus
}
#endif

#endif // __SCOPE_PLOT_H__

