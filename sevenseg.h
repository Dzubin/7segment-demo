//
//  sevenseg.h - a reusable seven-segment (really eight-segment: a..g + the
//  decimal point) digit-drawing engine for the PicoCalc LCD.
//
//  This file plus sevenseg.c is meant to be copied whole into a future
//  project that wants a 7-segment-style readout (score, clock, counter...).
//  It has no dependency on anything in this demo besides drivers/lcd.h.
//

#pragma once

#include <stdbool.h>
#include <stdint.h>

// Segment bit order (classic naming): bit0=a(top) bit1=b(upper-right)
// bit2=c(lower-right) bit3=d(bottom) bit4=e(lower-left) bit5=f(upper-left)
// bit6=g(middle). The decimal point is an 8th segment, passed separately
// as dp_on rather than a mask bit, since it sits outside the w x h box:
//
//   (x,y) .aaaa.
//         f    b
//         f    b
//         .gggg.
//         e    c
//         e    c
//         .dddd.        (dp) <- t x t square, lower right, outside the box
//
// The dp is a t x t square (the same width/height as every segment's own
// thickness), placed immediately right of the box with no gap of its own.
// A caller tiling copies of the glyph side by side should leave a
// t-sized gap between them - the dp exactly fills that gap, the same way a
// real multi-digit display's dp lives in the space between digit modules
// rather than adding extra width to either one.
//
#define SEG7_MASK_ALL 0x7F   // every segment a..g lit (the "8" glyph)

// Digit (0-9) to segment mask (a..g). Returns 0 (blank) for any other value.
uint8_t seg7_digit_mask(int digit);

// Baseline "blocky" renderer: draws all 7 segments (on_colour if the matching
// mask bit is set, off_colour otherwise) plus the decimal point, in a w x h
// box with top-left corner (x, y) and segment thickness `thickness`.
void seg7_draw(int x, int y, int w, int h, int thickness,
                uint8_t seg_mask, bool dp_on,
                uint16_t on_colour, uint16_t off_colour);

// Same as seg7_draw(), but bevels the glyph's four outer corners with small
// bg_colour squares (an easy, rectangle-only way to soften the silhouette).
void seg7_draw_chamfered(int x, int y, int w, int h, int thickness,
                            uint8_t seg_mask, bool dp_on,
                            uint16_t on_colour, uint16_t off_colour,
                            uint16_t bg_colour);

// Same as seg7_draw(), but leans the glyph like a real LED clock: each
// horizontal segment (a/g/d) shifts right by an amount that shrinks toward
// the bottom (top leans right of bottom - the standard "/" italic
// direction), and each vertical segment (b/c/e/f) is drawn as a stack of
// thin strips whose shift interpolates between its two neighbouring
// horizontals - giving a slanted parallelogram look using only
// axis-aligned rectangles.
void seg7_draw_italic(int x, int y, int w, int h, int thickness,
                        uint8_t seg_mask, bool dp_on,
                        uint16_t on_colour, uint16_t off_colour);
