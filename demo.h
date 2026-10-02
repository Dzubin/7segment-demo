//
//  demo.h - the 7segment-demo screen layout: a grid with two independent
//  cursors (a size tier and a style, moved separately), plus a combined
//  detail view. The reusable drawing engine lives in sevenseg.c/.h;
//  everything here is specific to this comparison screen.
//

#pragma once

#define DEMO_SIZE_TIER_COUNT  6
#define DEMO_STYLE_COUNT      5

// Draws the whole comparison grid (title, size sweep, style sweep, footer)
// with a highlight box around size tier `size_index` and a second one
// around style `style_index` - two independent selections at once.
void demo_draw_grid(int size_index, int style_index);

// Clears the screen and tiles the *combination* of the two selections -
// style `style_index`'s segment shape, rendered at size tier
// `size_index`'s own w/h/thickness - across the whole screen, as many
// non-overlapping copies as fit, centered as a block.
void demo_draw_detail(int size_index, int style_index);
