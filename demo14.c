//
//  demo14.c - lays out the 14-segment comparison screen using the
//  seg14.h drawing engine. Structurally identical to demo.c's 7-segment
//  screen (same 5 styles, same independent-cursor + combined-detail-view
//  model) - see demo.h/demo.c for the design notes this mirrors; comments
//  here only call out what's different for 14 segments specifically. Its
//  own size_tiers[] and STYLE14_* reference size are NOT the same as
//  7-segment's - see the comment on size_tiers[] below for why.
//

#include <stdio.h>

#include "pico/stdlib.h"

#include "drivers/lcd.h"
#include "constants.h"
#include "seg14.h"
#include "demo14.h"

// Four tiers, NOT the same as the 7-segment screen's six (demo.c). A
// 14-segment glyph's diagonals converge toward the centre from all four
// corners; if thickness scales with size the way 7-segment's does, the
// diagonals come out as thick as the box is short, and the convergence
// point turns into a solid blob rather than a legible "X" crossing (this
// shipped broken twice, in two different ways - see CHANGELOG). The fix
// confirmed in a mockup: keep the same w:h aspect ratio as before, but
// give each tier much more w/h relative to t (roughly t = h/12..h/13,
// vs. 7-segment's t = h/9..h/10) - hence four, smaller-range tiers
// instead of six, rather than a 5x8-scale minimum that just isn't
// possible to read with full-thickness diagonals.
typedef struct
{
    int w, h, t;
    const char *label;
} size_tier_t;

static const size_tier_t size_tiers[DEMO14_SIZE_TIER_COUNT] = {
    {16, 24, 2, "16x24 T1  (bare minimum)" },
    {24, 36, 3, "24x36 T2" },
    {30, 48, 4, "30x48 T3" },
    {40, 64, 5, "40x64 T4" },
};

// Same style names/deltas as demo.c's style_variants[] - see that file's
// comment for why a delta (not a fixed pixel value) keeps THIN/BOLD
// proportional at any size.
typedef struct
{
    const char *name;
    int t_delta;
} style_variant_t;

static const style_variant_t style_variants[DEMO14_STYLE_COUNT] = {
    {"BLOCK",   0},
    {"THIN",   -2},
    {"BOLD",    3},
    {"ITALIC",  0},
    {"CHAMFER", 0},
};

// Keeps half_len = (w - 2t - gap)/2 and vlen = (h - 3t)/2 (seg14.c's
// SEG14_BAR_GEOMETRY) from going negative - the /3 on w here (not /2, as
// sevenseg.c's clamp uses) is a deliberately conservative margin: 14
// segments pack more into the same box than 7 do, so it's safer to leave
// more headroom than the bar-length formula strictly requires.
static int clamp_thickness(int t, int w, int h)
{
    int max_t = (w / 3 < h / 3) ? w / 3 : h / 3;
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
        case 3:  seg14_draw_italic(x, y, w, h, t, SEG14_MASK_ALL, true, COLOR_ON, COLOR_OFF); break;
        case 4:  seg14_draw_chamfered(x, y, w, h, t, SEG14_MASK_ALL, true, COLOR_ON, COLOR_OFF, COLOR_BG); break;
        default: seg14_draw(x, y, w, h, t, SEG14_MASK_ALL, true, COLOR_ON, COLOR_OFF); break;
    }
}

// --- Shared position lookups (identical layout math to demo.c) ---------

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

    for (int i = 0; i < DEMO14_SIZE_TIER_COUNT; i++)
    {
        const size_tier_t *tier = &size_tiers[i];
        int x, y;
        size_tier_pos(i, &x, &y);

        seg14_draw(x, y, tier->w, tier->h, tier->t, SEG14_MASK_ALL, true, COLOR_ON, COLOR_OFF);

        int label_row = y / GLYPH_HEIGHT;
        lcd_set_foreground(COLOR_TEXT);
        lcd_putstr(SIZE_SWEEP_LABEL_COL, label_row, tier->label);
    }
}

static void draw_style_sweep(void)
{
    lcd_set_foreground(COLOR_HEADER);
    lcd_set_background(COLOR_BG);
    char header[48];
    snprintf(header, sizeof(header), "STYLE SWEEP (same %dx%d size):", STYLE14_W, STYLE14_H);
    lcd_putstr(2, STYLE_HEADER_ROW, header);

    int label_row = (STYLE_ROW_Y + STYLE14_H + STYLE_LABEL_GAP + GLYPH_HEIGHT - 1) / GLYPH_HEIGHT;

    for (int i = 0; i < DEMO14_STYLE_COUNT; i++)
    {
        int x, y;
        style_item_pos(i, &x, &y);

        draw_style_item(i, x, y, STYLE14_W, STYLE14_H, STYLE14_THICKNESS);

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
    const int margin = 3;
    const size_tier_t *tier = &size_tiers[size_index];
    int x, y;
    size_tier_pos(size_index, &x, &y);
    draw_highlight_box(x - margin, y - margin,
                        tier->w + tier->t + margin, tier->h + 2 * margin, COLOR_SELECT);
}

static void highlight_style_item(int style_index)
{
    const int margin = 3;
    int t = style_thickness(style_index, STYLE14_THICKNESS, STYLE14_W, STYLE14_H);
    int x, y;
    style_item_pos(style_index, &x, &y);
    int w = STYLE14_W + ((style_index == 3) ? STYLE14_H / 6 : 0);   // ITALIC's skew overhang
    draw_highlight_box(x - margin, y - margin,
                        w + t + margin, STYLE14_H + 2 * margin, COLOR_SELECT);
}

// --- Public API ------------------------------------------------------------

void demo14_draw_grid(int size_index, int style_index)
{
    lcd_clear_screen();

    lcd_set_foreground(COLOR_TITLE);
    lcd_set_background(COLOR_BG);
    lcd_putstr(4, TITLE_ROW, "14-SEGMENT DISPLAY DEMO " VERSION);

    draw_size_sweep();
    draw_style_sweep();

    highlight_size_tier(size_index);
    highlight_style_item(style_index);

    lcd_set_foreground(COLOR_TEXT);
    lcd_putstr(2, FOOTER_ROW, "U/D=sz L/R=stl ENTER F1=7SEG ~=BOOT");
}

void demo14_draw_detail(int size_index, int style_index)
{
    lcd_clear_screen();
    lcd_set_background(COLOR_BG);

    const size_tier_t *tier = &size_tiers[size_index];
    // See demo.c's demo_draw_detail() for why the gap is this
    // combination's own effective thickness, and why ITALIC needs no
    // extra overhang allowance here specifically.
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
