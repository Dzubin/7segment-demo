//
//  seg14.c - fourteen-segment (+ decimal point) digit-drawing engine.
//  See seg14.h for the segment layout diagram and bit order.
//

#include "drivers/lcd.h"
#include "seg14.h"

// vlen/half_len/midY together - used by draw_bars() and
// seg14_draw_chamfered(), which both touch every one of the 10 straight
// bars. Functions that only need some of these (seg14_draw(),
// seg14_draw_italic()) declare just what they use directly instead, so
// there's never an unused local under -Wall -Werror.
//
// half_len leaves a `gap`-wide centre gap between each split bar's two
// halves - t/2 (never less than 1px), not a full t. A full-t gap was the
// original design and it starved half_len at anything but tiny thickness
// (e.g. half_len=1 at the 26x42 style row's BOLD t=8, collapsing the split
// bars into near-invisible slivers next to the diagonals) - confirmed by
// rendering the exact formulas in a throwaway mockup before changing this.
#define SEG14_BAR_GEOMETRY(x, y, w, h, t) \
    int vlen = ((h) - 3 * (t)) / 2; \
    int gap = (((t) / 2) > 0) ? ((t) / 2) : 1; \
    int half_len = ((w) - 2 * (t) - gap) / 2; \
    int midY = (y) + ((h) - (t)) / 2

static uint16_t colour_for(uint16_t seg_mask, int bit, uint16_t on_colour, uint16_t off_colour)
{
    return (seg_mask & (1u << bit)) ? on_colour : off_colour;
}

// The dp is a t x t square - the same width/height as every other
// segment's own thickness - sitting immediately right of the box's right
// edge, bottom-aligned with it. See sevenseg.c's draw_dp() for the same
// design and why: it's meant to exactly fill a t-sized gap between tiled
// copies, the way a real multi-character display's dp lives in the space
// between character modules.
static void draw_dp(int box_right_x, int box_bottom_y, int t, bool dp_on, uint16_t on_colour, uint16_t off_colour)
{
    lcd_solid_rectangle(dp_on ? on_colour : off_colour, box_right_x, box_bottom_y - t, t, t);
}

// Approximates a thick diagonal line from (x0,y0) to (x1,y1) by stamping a
// t x t square at every unit step along it - the only diagonal-line
// "primitive" available when the LCD driver only offers axis-aligned
// filled rectangles.
static void draw_diag(int x0, int y0, int x1, int y1, int t, uint16_t colour)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int adx = (dx < 0) ? -dx : dx;
    int ady = (dy < 0) ? -dy : dy;
    int steps = (adx > ady) ? adx : ady;
    if (steps < 1)
    {
        steps = 1;
    }

    for (int i = 0; i <= steps; i++)
    {
        int px = x0 + dx * i / steps;
        int py = y0 + dy * i / steps;
        lcd_solid_rectangle(colour, px, py, t, t);
    }
}

// Draws the 10 straight bars, shifted horizontally by shift_a/_g/_d per
// row (all 0 for the plain seg14_draw()) - shared by every renderer so
// their bar geometry can never drift apart.
static void draw_bars(int x, int y, int w, int h, int t, uint16_t seg_mask,
                        int shift_a, int shift_g, int shift_d,
                        uint16_t on_colour, uint16_t off_colour)
{
    SEG14_BAR_GEOMETRY(x, y, w, h, t);

    lcd_solid_rectangle(colour_for(seg_mask, 0, on_colour, off_colour), x + t + shift_a,               y,         half_len, t);    // a1
    lcd_solid_rectangle(colour_for(seg_mask, 1, on_colour, off_colour), x + w - t - half_len + shift_a, y,         half_len, t);    // a2
    lcd_solid_rectangle(colour_for(seg_mask, 2, on_colour, off_colour), x + w - t,                      y + t,     t,        vlen); // b
    lcd_solid_rectangle(colour_for(seg_mask, 3, on_colour, off_colour), x + w - t,                      midY + t,  t,        vlen); // c
    lcd_solid_rectangle(colour_for(seg_mask, 4, on_colour, off_colour), x + t + shift_d,               y + h - t, half_len, t);    // d1
    lcd_solid_rectangle(colour_for(seg_mask, 5, on_colour, off_colour), x + w - t - half_len + shift_d, y + h - t, half_len, t);    // d2
    lcd_solid_rectangle(colour_for(seg_mask, 6, on_colour, off_colour), x,                              midY + t,  t,        vlen); // e
    lcd_solid_rectangle(colour_for(seg_mask, 7, on_colour, off_colour), x,                              y + t,     t,        vlen); // f
    lcd_solid_rectangle(colour_for(seg_mask, 8, on_colour, off_colour), x + t + shift_g,               midY,      half_len, t);    // g1
    lcd_solid_rectangle(colour_for(seg_mask, 9, on_colour, off_colour), x + w - t - half_len + shift_g, midY,      half_len, t);    // g2
}

// Diagonals are the same thickness as every other segment - t. This only
// reads cleanly (not as a blobby merge at the centre where all four
// diagonals converge) when the box is generously sized relative to t; see
// demo14.c's size_tiers[]/STYLE14_* comments for the w/h:t ratio that
// takes to look right - confirmed by rendering the exact formulas in a
// mockup across several ratios before picking one.
static int diag_thickness(int t)
{
    return t;
}

void seg14_draw(int x, int y, int w, int h, int t,
                  uint16_t seg_mask, bool dp_on,
                  uint16_t on_colour, uint16_t off_colour)
{
    int cx = x + w / 2;
    int cy = y + (h - t) / 2 + t / 2;
    int dt = diag_thickness(t);

    draw_bars(x, y, w, h, t, seg_mask, 0, 0, 0, on_colour, off_colour);

    // Each diagonal always spans the full vertical distance to the middle
    // row (dy_top/dy_bot), so it visually connects to g1/g2 with no gap;
    // dx is whatever horizontal room is available, uncapped. That comes
    // out steeper than 45 degrees for our tall/narrow boxes, but a
    // properly-45-degree diagonal that stops short of the middle looked
    // worse - it read as a disconnected stub, not part of the glyph.
    int dx     = cx - t - (x + t);
    int dy_top = cy - (y + t);
    int dy_bot = (y + h - t) - cy;

    draw_diag(x + t,     y + t,     x + t + dx,     y + t + dy_top,     dt, colour_for(seg_mask, 10, on_colour, off_colour)); // h  NW
    draw_diag(x + w - t, y + t,     x + w - t - dx, y + t + dy_top,     dt, colour_for(seg_mask, 11, on_colour, off_colour)); // i  NE
    draw_diag(x + t,     y + h - t, x + t + dx,     y + h - t - dy_bot, dt, colour_for(seg_mask, 12, on_colour, off_colour)); // j  SW
    draw_diag(x + w - t, y + h - t, x + w - t - dx, y + h - t - dy_bot, dt, colour_for(seg_mask, 13, on_colour, off_colour)); // k  SE

    draw_dp(x + w, y + h, t, dp_on, on_colour, off_colour);
}

// Bevels one bar's own four corners with small bg_colour squares, turning
// its sharp rectangle into a rough octagon - same idea as sevenseg.c's
// cut_corners().
static void cut_corners(int seg_x, int seg_y, int seg_w, int seg_h, int chamfer, uint16_t bg_colour)
{
    lcd_solid_rectangle(bg_colour, seg_x,                   seg_y,                   chamfer, chamfer);
    lcd_solid_rectangle(bg_colour, seg_x + seg_w - chamfer, seg_y,                   chamfer, chamfer);
    lcd_solid_rectangle(bg_colour, seg_x,                   seg_y + seg_h - chamfer, chamfer, chamfer);
    lcd_solid_rectangle(bg_colour, seg_x + seg_w - chamfer, seg_y + seg_h - chamfer, chamfer, chamfer);
}

void seg14_draw_chamfered(int x, int y, int w, int h, int t,
                            uint16_t seg_mask, bool dp_on,
                            uint16_t on_colour, uint16_t off_colour,
                            uint16_t bg_colour)
{
    seg14_draw(x, y, w, h, t, seg_mask, dp_on, on_colour, off_colour);

    SEG14_BAR_GEOMETRY(x, y, w, h, t);
    int chamfer = (t / 3 > 0) ? t / 3 : 1;

    cut_corners(x + t,                y,         half_len, t,    chamfer, bg_colour); // a1
    cut_corners(x + w - t - half_len, y,         half_len, t,    chamfer, bg_colour); // a2
    cut_corners(x + w - t,            y + t,     t,        vlen, chamfer, bg_colour); // b
    cut_corners(x + w - t,            midY + t,  t,        vlen, chamfer, bg_colour); // c
    cut_corners(x + t,                y + h - t, half_len, t,    chamfer, bg_colour); // d1
    cut_corners(x + w - t - half_len, y + h - t, half_len, t,    chamfer, bg_colour); // d2
    cut_corners(x,                    midY + t,  t,        vlen, chamfer, bg_colour); // e
    cut_corners(x,                    y + t,     t,        vlen, chamfer, bg_colour); // f
    cut_corners(x + t,                midY,      half_len, t,    chamfer, bg_colour); // g1
    cut_corners(x + w - t - half_len, midY,      half_len, t,    chamfer, bg_colour); // g2
}

void seg14_draw_italic(int x, int y, int w, int h, int t,
                         uint16_t seg_mask, bool dp_on,
                         uint16_t on_colour, uint16_t off_colour)
{
    int vlen = (h - 3 * t) / 2;
    int midY = y + (h - t) / 2;
    int cx   = x + w / 2;
    int cy   = midY + t / 2;

    // Same three-row shift as seg7_draw_italic(): top leans right of
    // bottom, the standard "/" italic direction.
    int skew    = h / 6;
    int shift_a = skew;
    int shift_g = skew / 2;
    int shift_d = 0;

    draw_bars(x, y, w, h, t, seg_mask, shift_a, shift_g, shift_d, on_colour, off_colour);

    // Verticals lean too, same trick as seg7_draw_italic()'s
    // draw_vseg_slanted(): stack 1px strips, shift interpolated linearly.
    int denom = (vlen > 1) ? (vlen - 1) : 1;
    for (int row = 0; row < vlen; row++)
    {
        int shift_top = shift_a + (shift_g - shift_a) * row / denom;
        lcd_solid_rectangle(colour_for(seg_mask, 2, on_colour, off_colour), x + w - t + shift_top, y + t + row, t, 1); // b
        lcd_solid_rectangle(colour_for(seg_mask, 7, on_colour, off_colour), x + shift_top,          y + t + row, t, 1); // f
    }
    for (int row = 0; row < vlen; row++)
    {
        int shift_bot = shift_g + (shift_d - shift_g) * row / denom;
        lcd_solid_rectangle(colour_for(seg_mask, 3, on_colour, off_colour), x + w - t + shift_bot, midY + t + row, t, 1); // c
        lcd_solid_rectangle(colour_for(seg_mask, 6, on_colour, off_colour), x + shift_bot,          midY + t + row, t, 1); // e
    }

    // Diagonals keep their own angle and always reach the middle row (see
    // seg14_draw()'s comment) - each is just translated by its row's
    // shift (top two by shift_a, bottom two by shift_d).
    int dt     = diag_thickness(t);
    int dx     = cx - t - (x + t);
    int dy_top = cy - (y + t);
    int dy_bot = (y + h - t) - cy;

    draw_diag(x + t + shift_a,     y + t,     x + t + shift_a + dx,     y + t + dy_top,     dt, colour_for(seg_mask, 10, on_colour, off_colour)); // h
    draw_diag(x + w - t + shift_a, y + t,     x + w - t + shift_a - dx, y + t + dy_top,     dt, colour_for(seg_mask, 11, on_colour, off_colour)); // i
    draw_diag(x + t + shift_d,     y + h - t, x + t + shift_d + dx,     y + h - t - dy_bot, dt, colour_for(seg_mask, 12, on_colour, off_colour)); // j
    draw_diag(x + w - t + shift_d, y + h - t, x + w - t + shift_d - dx, y + h - t - dy_bot, dt, colour_for(seg_mask, 13, on_colour, off_colour)); // k

    draw_dp(x + w + shift_d, y + h, t, dp_on, on_colour, off_colour);
}
