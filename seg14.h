//
//  seg14.h - a reusable fourteen-segment (really fifteen-segment: 14 +
//  the decimal point) alphanumeric digit-drawing engine for the PicoCalc
//  LCD.
//
//  This file plus seg14.c is meant to be copied whole into a future
//  project that wants a 14-segment-style alphanumeric readout. It has no
//  dependency on anything in this demo besides drivers/lcd.h.
//
//  A 14-segment glyph is the same split-top/split-bottom/split-middle
//  layout as a "9-segment" digit (a1/a2, b, c, d1/d2, e, f, g1/g2 - 10
//  straight bars), plus four diagonals (h/i/j/k) running from each corner
//  toward the centre, which is what lets it form letters as well as
//  digits:
//
//   (x,y) .a1.a2.
//         f h  i b
//         f  h i  b
//         .g1..g2.
//         e  j k  c
//         e j  k  c
//         .d1.d2.        (dp) <- t x t square, lower right, outside the box
//
//  Segment bit order: bit0=a1 bit1=a2 bit2=b bit3=c bit4=d1 bit5=d2
//  bit6=e bit7=f bit8=g1 bit9=g2 bit10=h(NW diag) bit11=i(NE diag)
//  bit12=j(SW diag) bit13=k(SE diag). The decimal point is a 15th
//  segment, passed separately as dp_on rather than a mask bit, since (like
//  sevenseg.h's dp) it sits outside the w x h box - see seg14.c's draw_dp
//  comment for why it's sized to match the segment thickness exactly.
//
//  The diagonals are approximated by stamping a t x t square at every
//  step along a straight line between two endpoints - there is no
//  diagonal-line primitive on this LCD, only axis-aligned filled
//  rectangles, so this is the same kind of trick sevenseg.h's
//  seg7_draw_italic() uses for its slanted verticals, generalised to any
//  direction.
//

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define SEG14_MASK_ALL 0x3FFF   // every straight + diagonal segment lit

// Baseline "blocky" renderer: draws all 14 segments (on_colour if the
// matching mask bit is set, off_colour otherwise) plus the decimal point,
// in a w x h box with top-left corner (x, y) and segment thickness `t`.
void seg14_draw(int x, int y, int w, int h, int t,
                  uint16_t seg_mask, bool dp_on,
                  uint16_t on_colour, uint16_t off_colour);

// Same as seg14_draw(), but bevels the 10 straight bars' own corners with
// small bg_colour squares (the 4 diagonals are stamped shapes, not plain
// rectangles, so they aren't chamfered).
void seg14_draw_chamfered(int x, int y, int w, int h, int t,
                            uint16_t seg_mask, bool dp_on,
                            uint16_t on_colour, uint16_t off_colour,
                            uint16_t bg_colour);

// Same as seg14_draw(), but leans the glyph like a real LED clock: the top
// row (a1/a2) shifts right the most, the middle row (g1/g2) half as much,
// and the bottom row (d1/d2) not at all - the standard "/" italic
// direction, matching seg7_draw_italic(). The verticals (b/c/e/f) are
// drawn as stacked slanted strips between their neighbouring rows' shifts,
// and each diagonal is translated by its own row's shift (its angle
// doesn't change, just its position).
void seg14_draw_italic(int x, int y, int w, int h, int t,
                         uint16_t seg_mask, bool dp_on,
                         uint16_t on_colour, uint16_t off_colour);
