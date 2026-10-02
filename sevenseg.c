//
//  sevenseg.c - a reusable seven-segment (+ decimal point) digit-drawing
//  engine. See sevenseg.h for the segment layout diagram and bit order.
//
//  Every segment (and the decimal point) is drawn every call, in either
//  on_colour or off_colour, so an unlit LED-style "ghost" outline is always
//  available to a caller that wants it - pass off_colour == bg to hide it.
//

#include "drivers/lcd.h"
#include "sevenseg.h"

// Standard seven-segment bit order: bit0=a(top) .. bit6=g(middle).
static const uint8_t digit_masks[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

uint8_t seg7_digit_mask(int digit)
{
    return (digit >= 0 && digit <= 9) ? digit_masks[digit] : 0x00;
}

// Shared geometry: hlen/vlen are the horizontal/vertical segment lengths,
// midY is the top of the middle (g) segment.
#define SEG7_GEOMETRY(x, y, w, h, t) \
    int hlen = (w) - 2 * (t); \
    int vlen = ((h) - 3 * (t)) / 2; \
    int midY = (y) + ((h) - (t)) / 2

static uint16_t colour_for(uint8_t seg_mask, int bit, uint16_t on_colour, uint16_t off_colour)
{
    return (seg_mask & (1 << bit)) ? on_colour : off_colour;
}

// The dp is a t x t square - the same width/height as every other segment's
// thickness - sitting immediately right of the box's right edge, bottom-
// aligned with it. A caller that tiles copies of the glyph is expected to
// use a t-sized gap between them, so the dp exactly fills that gap rather
// than needing (or leaving) any space of its own.
static void draw_dp(int box_right_x, int box_bottom_y, int t, bool dp_on, uint16_t on_colour, uint16_t off_colour)
{
    lcd_solid_rectangle(dp_on ? on_colour : off_colour, box_right_x, box_bottom_y - t, t, t);
}

void seg7_draw(int x, int y, int w, int h, int t,
                uint8_t seg_mask, bool dp_on,
                uint16_t on_colour, uint16_t off_colour)
{
    SEG7_GEOMETRY(x, y, w, h, t);

    lcd_solid_rectangle(colour_for(seg_mask, 0, on_colour, off_colour), x + t,     y,             hlen, t);     // a
    lcd_solid_rectangle(colour_for(seg_mask, 1, on_colour, off_colour), x + w - t, y + t,         t,    vlen);  // b
    lcd_solid_rectangle(colour_for(seg_mask, 2, on_colour, off_colour), x + w - t, midY + t,      t,    vlen);  // c
    lcd_solid_rectangle(colour_for(seg_mask, 3, on_colour, off_colour), x + t,     y + h - t,     hlen, t);     // d
    lcd_solid_rectangle(colour_for(seg_mask, 4, on_colour, off_colour), x,         midY + t,      t,    vlen);  // e
    lcd_solid_rectangle(colour_for(seg_mask, 5, on_colour, off_colour), x,         y + t,         t,    vlen);  // f
    lcd_solid_rectangle(colour_for(seg_mask, 6, on_colour, off_colour), x + t,     midY,          hlen, t);     // g

    draw_dp(x + w, y + h, t, dp_on, on_colour, off_colour);
}

// Bevels one segment bar's own four corners with small bg_colour squares,
// turning its sharp rectangle into a rough octagon. chamfer must be <= half
// of both the bar's width and height, or the cuts would overlap each other.
static void cut_corners(int seg_x, int seg_y, int seg_w, int seg_h, int chamfer, uint16_t bg_colour)
{
    lcd_solid_rectangle(bg_colour, seg_x,                 seg_y,                 chamfer, chamfer);
    lcd_solid_rectangle(bg_colour, seg_x + seg_w - chamfer, seg_y,                 chamfer, chamfer);
    lcd_solid_rectangle(bg_colour, seg_x,                 seg_y + seg_h - chamfer, chamfer, chamfer);
    lcd_solid_rectangle(bg_colour, seg_x + seg_w - chamfer, seg_y + seg_h - chamfer, chamfer, chamfer);
}

void seg7_draw_chamfered(int x, int y, int w, int h, int t,
                            uint8_t seg_mask, bool dp_on,
                            uint16_t on_colour, uint16_t off_colour,
                            uint16_t bg_colour)
{
    seg7_draw(x, y, w, h, t, seg_mask, dp_on, on_colour, off_colour);

    // Bounding-box corners (x,y), (x+w-t,y) etc. are already empty gaps
    // between adjacent segments - bevelling there would be invisible.
    // A real chamfer has to cut into each segment bar's own corners
    // instead, so it's capped well under t/2 to leave a bar-shaped middle.
    SEG7_GEOMETRY(x, y, w, h, t);
    int chamfer = (t / 3 > 0) ? t / 3 : 1;

    cut_corners(x + t,     y,             hlen, t,    chamfer, bg_colour); // a
    cut_corners(x + w - t, y + t,         t,    vlen, chamfer, bg_colour); // b
    cut_corners(x + w - t, midY + t,      t,    vlen, chamfer, bg_colour); // c
    cut_corners(x + t,     y + h - t,     hlen, t,    chamfer, bg_colour); // d
    cut_corners(x,         midY + t,      t,    vlen, chamfer, bg_colour); // e
    cut_corners(x,         y + t,         t,    vlen, chamfer, bg_colour); // f
    cut_corners(x + t,     midY,          hlen, t,    chamfer, bg_colour); // g
}

// Draws a vertical segment as a stack of 1px-tall strips, each shifted right
// by an amount interpolated between shift_top (at the segment's top row) and
// shift_bottom (at its last row) - a plain-rectangle approximation of a
// slanted/parallelogram segment.
static void draw_vseg_slanted(int base_x, int y_top, int seg_h, int t,
                                int shift_top, int shift_bottom, uint16_t colour)
{
    if (seg_h <= 0)
    {
        return;
    }

    int denom = (seg_h > 1) ? (seg_h - 1) : 1;
    for (int row = 0; row < seg_h; row++)
    {
        int shift = shift_top + (shift_bottom - shift_top) * row / denom;
        lcd_solid_rectangle(colour, base_x + shift, y_top + row, t, 1);
    }
}

void seg7_draw_italic(int x, int y, int w, int h, int t,
                        uint8_t seg_mask, bool dp_on,
                        uint16_t on_colour, uint16_t off_colour)
{
    SEG7_GEOMETRY(x, y, w, h, t);

    // shift_a > shift_d so the top leans right of the bottom (the standard
    // "/" italic direction, like a real LED clock's slanted digits).
    int skew     = h / 6;
    int shift_a  = skew;
    int shift_g  = skew / 2;
    int shift_d  = 0;

    lcd_solid_rectangle(colour_for(seg_mask, 0, on_colour, off_colour), x + t + shift_a, y,         hlen, t); // a
    lcd_solid_rectangle(colour_for(seg_mask, 6, on_colour, off_colour), x + t + shift_g, midY,      hlen, t); // g
    lcd_solid_rectangle(colour_for(seg_mask, 3, on_colour, off_colour), x + t + shift_d, y + h - t, hlen, t); // d

    draw_vseg_slanted(x + w - t, y + t,    vlen, t, shift_a, shift_g, colour_for(seg_mask, 1, on_colour, off_colour)); // b
    draw_vseg_slanted(x,         y + t,    vlen, t, shift_a, shift_g, colour_for(seg_mask, 5, on_colour, off_colour)); // f
    draw_vseg_slanted(x + w - t, midY + t, vlen, t, shift_g, shift_d, colour_for(seg_mask, 2, on_colour, off_colour)); // c
    draw_vseg_slanted(x,         midY + t, vlen, t, shift_g, shift_d, colour_for(seg_mask, 4, on_colour, off_colour)); // e

    draw_dp(x + w + shift_d, y + h, t, dp_on, on_colour, off_colour);
}
