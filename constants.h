//
//  constants.h - compile-time configuration for 7segment-demo.
//
//  Every tunable value and colour lives here. main.c and demo.c both
//  include it; change layout/appearance here, not in the .c files.
//

#pragma once

#include "drivers/lcd.h"   // for RGB(), WIDTH/HEIGHT

#define VERSION "V0.01A"

// --- Colours (RGB565) --------------------------------------------------
#define COLOR_BG      RGB(0, 0, 0)          // background
#define COLOR_ON      RGB(255, 60, 20)      // lit segment - classic LED red-orange
#define COLOR_OFF     RGB(40, 15, 10)       // unlit segment "ghost" (unused here - every glyph is full-on)
#define COLOR_TITLE   RGB(255, 220, 60)     // screen title
#define COLOR_HEADER  RGB(120, 220, 255)    // section headers
#define COLOR_TEXT    RGB(200, 200, 200)    // row labels / footer
#define COLOR_SELECT  RGB(255, 255, 0)      // selection highlight box

// --- Size-sweep section: one column of glyphs, smallest to largest -----
// Layout positions only, shared by both screens - the tier count and each
// tier's own w/h/t live in demo.c's / demo14.c's own size_tiers[] tables
// (7-segment: six tiers; 14-segment: four, since its diagonals need more
// width/height relative to thickness than 7-segment's straight bars do).
#define SIZE_SWEEP_LEFT_X     8    // glyph left edge, pixels
#define SIZE_SWEEP_LABEL_COL  8    // fixed label column (all rows line up)
#define SIZE_SWEEP_TOP_Y      20   // first tier's top, pixels
#define SIZE_SWEEP_ROW_GAP    6    // vertical gap between tiers, pixels

// --- Style-sweep section: five styles at one shared size ---------------
// (positions/pitch below are shared by both screens; the reference glyph
// size itself is per-screen - see STYLE_* for 7-segment, STYLE14_* for
// 14-segment, which need a much larger size for the same thickness so its
// diagonals have room - see demo14.c's size_tiers[] comment)
#define STYLE_W          26   // 7-segment shared glyph width
#define STYLE_H          42   // 7-segment shared glyph height
#define STYLE_THICKNESS   5   // 7-segment shared baseline segment thickness

#define STYLE14_W        30   // 14-segment shared glyph width
#define STYLE14_H        48   // 14-segment shared glyph height
#define STYLE14_THICKNESS 4   // 14-segment shared baseline segment thickness

#define STYLE_CELL_W     64   // horizontal pitch between style samples, pixels
#define STYLE_LEFT_X      8   // first style sample's left edge, pixels
#define STYLE_LABEL_GAP   4   // gap between glyph bottom and its label, pixels

// Note: the detail view's tile gap is NOT a constant here - it's each
// combination's own effective thickness (demo.c's style_thickness()),
// since that's exactly the dp's size (see sevenseg.c's draw_dp()).

// --- Section header rows (text rows, 0..31) -----------------------------
#define TITLE_ROW          0
#define SIZE_HEADER_ROW    1
#define STYLE_HEADER_ROW  22
#define STYLE_ROW_Y       (STYLE_HEADER_ROW * GLYPH_HEIGHT + 12)  // pixels
#define FOOTER_ROW        31
