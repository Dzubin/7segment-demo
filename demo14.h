//
//  demo14.h - the 14-segment comparison screen: same structure as demo.h's
//  7-segment screen (independent size/style cursors, a combined detail
//  view), just built on seg14.h instead of sevenseg.h. Kept as a fully
//  separate screen rather than merged with the 7-segment one, since the
//  two are never shown or combined together - see main.c.
//

#pragma once

#define DEMO14_SIZE_TIER_COUNT  4
#define DEMO14_STYLE_COUNT      5

// Draws the whole 14-segment comparison grid (title, size sweep, style
// sweep, footer) with a highlight box around size tier `size_index` and a
// second one around style `style_index`.
void demo14_draw_grid(int size_index, int style_index);

// Clears the screen and tiles the combination of the two selections -
// style `style_index`'s segment shape, rendered at size tier
// `size_index`'s own w/h/thickness - across the whole screen.
void demo14_draw_detail(int size_index, int style_index);
