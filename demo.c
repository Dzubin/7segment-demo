//
//  demo.c - lays out the 7segment-demo comparison screen using the
//  sevenseg.h drawing engine. Every sample shows the same full "8." glyph
//  (every segment plus the decimal point lit) so each row/column is a pure
//  comparison of size or style, not of which digit is shown.
//
//  Size and style are two independent selections (UP/DOWN moves the size
//  cursor, LEFT/RIGHT the style cursor - see main.c), so ENTER's detail
//  view renders the selected STYLE using the selected SIZE tier's own
//  w/h/thickness - any size+style combination, not just the fixed 26x42
//  the style row itself is shown at.
//
//  size_tier_pos()/style_item_pos() are the single source of truth for
//  where each item sits, so the grid, its highlight boxes and the detail
//  view can never disagree about a position.
//

#include <stdio.h>

#include "pico/stdlib.h"

#include "drivers/lcd.h"
#include "constants.h"
#include "sevenseg.h"
#include "demo.h"

// --- Size sweep: smallest workable digit up to the largest that still lets
// eight digits span the 320px-wide screen. That top tier is 34px wide, and
// 8*34 + 7*6px gaps = 314px, so eight of them (with a few px to spare) fit
// across the LCD - hence "8 digits @ 320px" in its label. Always drawn in
// BLOCK style in the grid - it's a size comparison, not a style one.
typedef struct
{
    int w, h, t;
    const char *label;
} size_tier_t;

static const size_tier_t size_tiers[DEMO_SIZE_TIER_COUNT] = {
    { 5,  8, 1, "5x8   T1  (bare minimum)" },
    { 8, 13, 2, "8x13  T2" },
    {12, 20, 3, "12x20 T3" },
    {18, 30, 4, "18x30 T4" },
    {24, 40, 5, "24x40 T5" },
    {34, 56, 6, "34x56 T6  (8 digits @320px)" },
};

// --- Style sweep: five looks, each a rendering function plus a thickness
// *delta* from whatever base thickness it's combined with - not a fixed
// pixel value - so THIN/BOLD stay proportional at any size, not just the
// style row's own fixed 26x42 reference. At that reference (base
// STYLE_THICKNESS = 5), the deltas below reproduce the exact THIN/BOLD
// pixel thicknesses this demo shipped with before (3 and 8).
typedef struct
{
    const char *name;
    int t_delta;
} style_variant_t;

static const style_variant_t style_variants[DEMO_STYLE_COUNT] = {
    {"BLOCK",   0},
    {"THIN",   -2},
    {"BOLD",    3},
    {"ITALIC",  0},
    {"CHAMFER", 0},
};

// Keeps hlen = w - 2t and vlen = (h - 3t) / 2 (sevenseg.c's SEG7_GEOMETRY)
// from going negative. lcd_solid_rectangle()'s width/height are uint16_t,
// so a negative length would silently wrap into a huge garbage rectangle
// rather than just failing to draw - this must never be skipped.
static int clamp_thickness(int t, int w, int h)
{
    int max_t = (w / 2 < h / 3) ? w / 2 : h / 3;
    if (max_t < 1) max_t = 1;
    if (t > max_t) t = max_t;
    return (t < 1) ? 1 : t;
}

static int style_thickness(int style_index, int base_t, int w, int h)
{
    int t = base_t + style_variants[style_index].t_delta;
    return clamp_thickness(t, w, h);
}

static void draw_style_item(int style_index, int x, int y, int w, int h, int base_t)
{
    int t = style_thickness(style_index, base_t, w, h);
    switch (style_index)
    {
        case 3:  seg7_draw_italic(x, y, w, h, t, SEG7_MASK_ALL, true, COLOR_ON, COLOR_OFF); break;
        case 4:  seg7_draw_chamfered(x, y, w, h, t, SEG7_MASK_ALL, true, COLOR_ON, COLOR_OFF, COLOR_BG); break;
        default: seg7_draw(x, y, w, h, t, SEG7_MASK_ALL, true, COLOR_ON, COLOR_OFF); break;
    }
}

// --- Shared position lookups (grid layout, highlight boxes and detail
// view all call these, so they can never disagree with each other) -------

static void size_tier_pos(int index, int *x, int *y)
{
    int cy = SIZE_SWEEP_TOP_Y;
    for (int i = 0; i < index; i++)
    {
        cy += size_tiers[i].h + SIZE_SWEEP_ROW_GAP;
    }
    *x = SIZE_SWEEP_LEFT_X;
    *y = cy;
}

static void style_item_pos(int index, int *x, int *y)
{
    *x = STYLE_LEFT_X + index * STYLE_CELL_W;
    *y = STYLE_ROW_Y;
}

// --- Grid section drawing -------------------------------------------------

static void draw_size_sweep(void)
{
    lcd_set_foreground(COLOR_HEADER);
    lcd_set_background(COLOR_BG);
    lcd_putstr(2, SIZE_HEADER_ROW, "SIZE SWEEP (same blocky style):");

    for (int i = 0; i < DEMO_SIZE_TIER_COUNT; i++)
    {
        const size_tier_t *tier = &size_tiers[i];
        int x, y;
        size_tier_pos(i, &x, &y);

        seg7_draw(x, y, tier->w, tier->h, tier->t, SEG7_MASK_ALL, true, COLOR_ON, COLOR_OFF);

        int label_row = y / GLYPH_HEIGHT;   // top-aligned with the glyph; no horizontal overlap either way
        lcd_set_foreground(COLOR_TEXT);
        lcd_putstr(SIZE_SWEEP_LABEL_COL, label_row, tier->label);
    }
}

static void draw_style_sweep(void)
{
    lcd_set_foreground(COLOR_HEADER);
    lcd_set_background(COLOR_BG);
    char header[48];
    snprintf(header, sizeof(header), "STYLE SWEEP (same %dx%d size):", STYLE_W, STYLE_H);
    lcd_putstr(2, STYLE_HEADER_ROW, header);

    int label_row = (STYLE_ROW_Y + STYLE_H + STYLE_LABEL_GAP + GLYPH_HEIGHT - 1) / GLYPH_HEIGHT;

    for (int i = 0; i < DEMO_STYLE_COUNT; i++)
    {
        int x, y;
        style_item_pos(i, &x, &y);

        draw_style_item(i, x, y, STYLE_W, STYLE_H, STYLE_THICKNESS);

        lcd_set_foreground(COLOR_TEXT);
        lcd_putstr(x / 8, label_row, style_variants[i].name);
    }
}

// --- Selection highlights ---------------------------------------------------

static void draw_highlight_box(int x, int y, int w, int h, uint16_t colour)
{
    lcd_solid_rectangle(colour, x,         y,         w, 2);
    lcd_solid_rectangle(colour, x,         y + h - 2, w, 2);
    lcd_solid_rectangle(colour, x,         y,         2, h);
    lcd_solid_rectangle(colour, x + w - 2, y,         2, h);
}

static void highlight_size_tier(int size_index)
{
    // Unlike the detail view's tiling, there's no adjacent tile here to
    // "absorb" the dp sticking out past tier->w, so the box's right edge
    // needs its own allowance for it - the dp is a tier->t x tier->t
    // square (see sevenseg.c's draw_dp()), so tier->t of slack is exactly
    // enough, no more.
    const int margin = 3;
    const size_tier_t *tier = &size_tiers[size_index];
    int x, y;
    size_tier_pos(size_index, &x, &y);
    draw_highlight_box(x - margin, y - margin,
                        tier->w + tier->t + margin, tier->h + 2 * margin, COLOR_SELECT);
}

static void highlight_style_item(int style_index)
{
    // Same reasoning as highlight_size_tier(): an isolated sample (not
    // tiled) needs its own allowance - this style's own effective
    // thickness at the style row's reference size - for the dp sticking
    // out past its right edge, on top of any ITALIC skew overhang.
    const int margin = 3;
    int t = style_thickness(style_index, STYLE_THICKNESS, STYLE_W, STYLE_H);
    int x, y;
    style_item_pos(style_index, &x, &y);
    int w = STYLE_W + ((style_index == 3) ? STYLE_H / 6 : 0);
    draw_highlight_box(x - margin, y - margin,
                        w + t + margin, STYLE_H + 2 * margin, COLOR_SELECT);
}

// --- Public API ------------------------------------------------------------

void demo_draw_grid(int size_index, int style_index)
{
    lcd_clear_screen();

    lcd_set_foreground(COLOR_TITLE);
    lcd_set_background(COLOR_BG);
    lcd_putstr(6, TITLE_ROW, "7-SEGMENT DISPLAY DEMO " VERSION);

    draw_size_sweep();
    draw_style_sweep();

    highlight_size_tier(size_index);
    highlight_style_item(style_index);

    lcd_set_foreground(COLOR_TEXT);
    lcd_putstr(FOOTER_COL, FOOTER_ROW, "ARROWS ENTER F1=14SEG ESC/Q=EXIT ~=BOOT");
}

void demo_draw_detail(int size_index, int style_index)
{
    lcd_clear_screen();
    lcd_set_background(COLOR_BG);

    const size_tier_t *tier = &size_tiers[size_index];
    // The gap between tiles must equal this combination's actual rendered
    // thickness t, since that's exactly the dp's own size (see draw_dp()
    // in sevenseg.c): tile pitch = tier->w + t puts the next tile's left
    // edge exactly where this tile's dp ends. ITALIC's own skew overhang
    // (unlike the grid's standalone highlight box) needs no extra
    // allowance here - clamp_thickness() keeps t large enough, relative to
    // every size tier actually defined below, that 2t already exceeds the
    // skew (h/6), so the dp's reach alone clears the lean with room to
    // spare. Re-check that if you add a tier with an unusually large h:t
    // ratio.
    int gap = style_thickness(style_index, tier->t, tier->w, tier->h);
    int fw = tier->w;
    int fh = tier->h;

    int cell_w = fw + gap;
    int cell_h = fh + gap;

    int cols = WIDTH / cell_w;
    int rows = HEIGHT / cell_h;
    if (cols < 1) cols = 1;
    if (rows < 1) rows = 1;

    int used_w = cols * fw + (cols - 1) * gap;
    int used_h = rows * fh + (rows - 1) * gap;
    int start_x = (WIDTH - used_w) / 2;
    int start_y = (HEIGHT - used_h) / 2;

    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            int x = start_x + col * cell_w;
            int y = start_y + row * cell_h;
            draw_style_item(style_index, x, y, tier->w, tier->h, tier->t);
        }
    }
}
