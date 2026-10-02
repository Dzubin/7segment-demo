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
#define FOOTER_COL         0

// --- Leaving for the PicoCalc UF2 Loader (pelrun/uf2loader) ------------
// The loader has no call for an app to use, but its own menu hands commands to
// its start-up code through the chip's watchdog scratch registers, which
// survive a watchdog reboot: scratch 0 holds a magic number, 1 the boot mode,
// 2 an argument. Asking for boot mode "SD" and then rebooting makes the loader
// show its menu again. With no loader installed the program just restarts.
#define LOADER_COMMAND_MAGIC     0xE98CC638u  // PICOCALC_BL_MAGIC in the loader's proginfo.h
#define LOADER_BOOT_MODE_SD      1            // BOOT_SD: load the menu from the SD card
#define LOADER_SCRATCH_MAGIC     0
#define LOADER_SCRATCH_MODE      1
#define LOADER_SCRATCH_ARGUMENT  2
#define LOADER_EXIT_MSG_MS       300          // how long "BACK TO THE LOADER..." shows
#define LOADER_REBOOT_DELAY_MS   10           // the reboot happens this long after it is asked for
#define LOADER_MSG_COL           2            // the message goes on the footer row
