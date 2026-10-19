# Changelog

All notable changes to 7segment-demo.
Format loosely follows [Keep a Changelog](https://keepachangelog.com/);
the version here matches the `VERSION` define in `constants.h`.

## [Unreleased]

- Updated the vendored drivers to Blair Leduc's `picocalc-text-starter` of 2026-10-18 (commit
  `85a002e`) and moved to Pico SDK 2.3.1. Two local changes remain: `lcd_putstr()` cuts a string off
  at the right edge of the display, and `keyboard.c` treats a held key as a press, so that the arrow
  keys auto-repeat.

### Added
- Leaving: `Q` (any screen) and `ESC` (from a grid) now leave for the PicoCalc
  UF2 Loader menu (it asks the loader for its menu through the watchdog scratch
  registers; with no loader installed the program just restarts, and on a PC it
  closes). The footer line was reworded to fit the hint; `~` still goes to
  BOOTSEL.
- Desktop build: the build logic that was repeated in `desktop/CMakeLists.txt` now
  lives in `desktop/shim_desktop.cmake` (that file now only names the program and
  its sources), and the shim files were refreshed from a shared copy. The numeric
  keypad now works in the desktop build.
- Windows build: `desktop/` holds a small SDL2 shim that re-implements the
  picocalc-text-starter driver API so the same `main.c` runs on a PC; SDL2 is
  linked statically, giving one self-contained `7segment-demo-Windows.exe`.
  Build with `cmake -S desktop -B build-windows -G Ninja` then
  `ninja -C build-windows` from an MSYS2 UCRT64 shell.
- Linux build: the `desktop/` build (formerly `windows/`) now also builds on
  Linux and produces `7segment-demo-Linux`. SDL2 is linked dynamically there and the
  build copies `libSDL2-2.0.so.0` next to the executable (found through an
  `$ORIGIN` run-path); needs the SDL2 development package (e.g. `libsdl2-dev`).
- The firmware build now names every output with the chip (the `.uf2`, `.elf`, `.bin` and `.hex` all end in `-RP2040` / `-RP2350`) and copies the `.uf2` to the top-level folder as
  `7segment-demo-RP2040.uf2` / `7segment-demo-RP2350.uf2`, so the two chips'
  builds sit side by side.

- Fixed a gap between the 14-segment diagonals and the middle (g1/g2) bar:
  the previous round's 45-degree length clamp fixed the blobby-pillar look
  but as a side effect stopped each diagonal short of the middle row,
  reading as a disconnected stub rather than part of the glyph. Diagonals
  now always reach the full vertical distance to the middle row; the
  horizontal reach is whatever room is available, uncapped, which comes
  out steeper than 45 degrees on these tall/narrow boxes but reads as
  correctly connected rather than "properly angled" and floating. Confirmed
  clean (not reintroducing the pillar look) at all four size tiers and
  with THIN in the same mockup-based workflow as the earlier rounds.
- 14-segment diagonals are now full segment thickness (matching the bars),
  not half-thickness - and to keep that from collapsing back into the
  blobby-merge problem the first fix solved, the 14-segment screen now has
  its own, more generous size tiers and style-row reference size instead
  of reusing 7-segment's: four tiers (16x24 up to 40x64) instead of six,
  dropping the 5x8/8x13 tiers entirely (too small to read a full-thickness
  diagonal at all), and a 30x48 style-row size instead of 26x42. Confirmed
  in the same mockup-based workflow as the first fix: keeping thickness
  proportional to size the way 7-segment's tiers do just doesn't leave
  full-thickness diagonals enough room, regardless of overall scale - what
  matters is `t` growing *slower* than `w`/`h` (roughly `t = h/12`..`h/13`
  here, vs. 7-segment's `h/9`..`h/10`).
- Fixed 14-segment rendering, which shipped badly broken: the diagonals
  spanned almost the full box height on almost no width (every tier is
  much taller than wide), so they came out nearly vertical and sat right
  on top of the b/c/e/f verticals - `BLOCK` and `BOLD` looked like two
  solid pillars, `CHAMFER` like a stick figure. Diagnosed by rendering the
  exact same integer formulas in a throwaway HTML/canvas mockup rather
  than guessing through more flash cycles. Fixed by: clamping each
  diagonal's length to the smaller of its room to the centre column and to
  the middle row (a genuine ~45 degrees instead of stretching to the true
  centre point), drawing diagonals at half the segment thickness instead
  of the full thickness (a thick stamp on a short diagonal was reading as
  a blob), and shrinking the split-bars' centre gap from a full `t` to
  `t/2` (min 1px), which was starving `half_len` at anything but tiny
  thickness.
- Added a second, entirely separate screen for 14-segment (alphanumeric)
  displays, toggled with `F1` (works from either screen's grid or detail
  view). Structurally identical to the 7-segment screen - same six size
  tiers, same five styles (`BLOCK`/`THIN`/`BOLD`/`ITALIC`/`CHAMFER`), same
  independent size+style selection and combined detail view - just built
  on a new `seg14.c`/`seg14.h` engine instead of `sevenseg.c`/`sevenseg.h`.
  Each screen remembers its own selection independently, so switching back
  and forth doesn't lose your place. The two screens are never mixed or
  shown together, per design.
- New reusable engine (`seg14.c` / `seg14.h`): `seg14_draw()`,
  `seg14_draw_chamfered()`, `seg14_draw_italic()`. A 14-segment glyph is
  the same 10 straight bars as a split-top/split-bottom/split-middle
  7-segment digit, plus 4 diagonals from each corner toward the centre.
  The diagonals are approximated by stamping a `t x t` square at every
  step along a straight line between two endpoints, since the LCD driver
  only offers axis-aligned filled rectangles - the same kind of trick
  `seg7_draw_italic()` uses for its slanted verticals, generalised to any
  direction.
- Selectable grid: UP/DOWN move a highlight box through the size tiers,
  LEFT/RIGHT independently move a second highlight through the styles.
  ENTER tiles the *combination* - the selected style, rendered at the
  selected size tier's own w/h/thickness, not overlapping - across the
  whole screen ("detail view"); ESC returns to the grid. Earlier in this
  round the two axes were one flat 11-item list (UP/LEFT = previous,
  DOWN/RIGHT = next); that's been replaced by the independent-axes model.
- Style thickness is now a *delta* from whatever base thickness it's
  combined with, not a fixed pixel value - so THIN/BOLD stay proportional
  at every size tier, not just the style row's own fixed 26x42 reference.
- `ITALIC` now leans the standard "/" direction (top shifted right of the
  bottom), not backslash-direction - it was backwards in 0.01A.
- Fixed `CHAMFER`: it was bevelling the *bounding box's* corners, which are
  already-empty gaps between segments (a no-op, indistinguishable from
  `BLOCK`). It now bevels each segment bar's own corners instead.
- The decimal point is now a plain `t x t` square - the same width/height
  as every other segment's own thickness - instead of scaling to its own
  formula. The gap between tiled copies now matches that same `t`, so the
  dp exactly fills the space between digits rather than adding extra width
  to either one, the same way a real multi-digit display's dp lives in
  the gap between digit modules. Since `t` now varies by combination
  (`demo.c`'s `style_thickness()`), the fixed `DETAIL_TILE_GAP` constant
  is gone - the tile gap and both grid highlight boxes' slack are
  computed per-combination instead.

## [0.01A]

First cut -

### Comparison screen
- `SIZE SWEEP`: six tiers (5x8, 8x13, 12x20, 18x30, 24x40, 34x56px) in one
  consistent blocky style, smallest to largest. The largest tier is sized so
  eight digits fit across the 320px-wide screen.
- `STYLE SWEEP`: `BLOCK`, `THIN`, `BOLD`, `ITALIC` and `CHAMFER` samples at
  one shared 26x42px size, so only the segment shape differs.
- Every sample shows the same full "8." glyph (all 7 segments + the decimal
  point lit) - a pure size/style comparison, not a digit comparison.
- Static screen, drawn once; `~` (SHIFT + backtick) reboots into BOOTSEL.

### Reusable engine (`sevenseg.c` / `sevenseg.h`)
- `seg7_draw()` - baseline rectangular segment renderer + decimal point.
- `seg7_draw_chamfered()` - bevels the glyph's four outer corners.
- `seg7_draw_italic()` - slants the glyph like a real LED clock, using
  stacked 1px strips for the vertical segments (no new LCD primitive
  needed - pure `lcd_solid_rectangle` calls).
- `seg7_digit_mask()` - digit (0-9) to segment-mask lookup, for a future
  project that wants to display real numbers rather than a fixed "8.".
