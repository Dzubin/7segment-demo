//
//  7segment-demo
//  =============
//
//  A comparison screen for LED-style digit displays on the PicoCalc's
//  320x320 LCD - built to pick a look before using one in a future
//  project. There are two entirely separate screens (never mixed, so
//  F1 just swaps which one is showing - see below):
//
//    - 7-SEGMENT: the classic 7-segment (+ decimal point) digit.
//        - A vertical SIZE SWEEP: six tiers from a bare 5x8px digit up to
//          the largest that still lets eight digits span the screen.
//        - A horizontal STYLE SWEEP: BLOCK / THIN / BOLD / ITALIC /
//          CHAMFER at one shared size.
//    - 14-SEGMENT: the alphanumeric 14-segment (+ decimal point) digit
//      (10 straight bars in the same split-7-segment layout, plus 4
//      diagonals). Same size tiers, same 5 styles, same controls.
//
//  On each screen, size and style are two independent selections, so
//  ENTER combines them: it fills the screen with the selected STYLE
//  rendered at the selected SIZE tier's own w/h/thickness.
//
//  Controls:
//    UP / DOWN ..... move the SIZE selection (wraps)
//    LEFT / RIGHT ... move the STYLE selection (wraps)
//    ENTER ......... tile the combined size+style across the screen
//                     ("detail view")
//    ESC ........... from detail view, back to the grid (no effect on the
//                     grid itself)
//    F1 ............ switch between the 7-segment and 14-segment screens
//                     (from either the grid or its detail view)
//    ~ ............. (SHIFT + backtick) reboot into BOOTSEL, from any
//                     screen
//
//  Author: Thomas Dzubin. Built on the Raspberry Pi Pico SDK and the LCD,
//  keyboard and south-bridge drivers from "picocalc-text-starter" by Blair
//  Leduc (drivers/lcd.c, drivers/keyboard.c, drivers/southbridge.c).
//
//  Source layout:
//    constants.h ......... every tunable value, colour and layout position
//                           (shared by both screens - they never show at
//                           the same time, so they can share the same
//                           screen real estate)
//    sevenseg.c / .h ...... the reusable 7-segment drawing engine (the
//                           part meant to be copied into a future project)
//    seg14.c / .h ......... the reusable 14-segment drawing engine (ditto)
//    demo.c / .h .......... the 7-segment screen's layout, built on sevenseg.h
//    demo14.c / .h ........ the 14-segment screen's layout, built on seg14.h
//    main.c ............... just main() and the selection/toggle/BOOTSEL
//                           key logic
//

#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "drivers/lcd.h"
#include "drivers/keyboard.h"

#include "demo.h"
#include "demo14.h"

// Referenced by the keyboard driver (set when the Brk key is pressed). We do
// not use it here, but the driver needs the symbol to link.
volatile bool user_interrupt = false;

typedef enum
{
    SCREEN_7SEG,
    SCREEN_14SEG
} screen_t;

int main(void)
{
    lcd_init();                 // also clears the screen and turns the display on
    lcd_enable_cursor(false);   // we are not drawing a text cursor

    keyboard_init();
    keyboard_set_background_poll(true);   // poll the keyboard on a timer

    screen_t screen = SCREEN_7SEG;
    bool detail = false;

    // Each screen keeps its own selection, so switching with F1 and back
    // returns you to where you left off on the other one.
    int size7 = 0, style7 = 0;
    int size14 = 0, style14 = 0;

    demo_draw_grid(size7, style7);

    while (true)
    {
        while (keyboard_key_available())
        {
            unsigned char key = (unsigned char)keyboard_get_key();

            if (key == '~')
            {
                rom_reset_usb_boot(0, 0);
            }
            else if (key == KEY_F1)
            {
                screen = (screen == SCREEN_7SEG) ? SCREEN_14SEG : SCREEN_7SEG;
                detail = false;
                if (screen == SCREEN_7SEG)
                {
                    demo_draw_grid(size7, style7);
                }
                else
                {
                    demo14_draw_grid(size14, style14);
                }
            }
            else if (screen == SCREEN_7SEG)
            {
                int *size_index = &size7;
                int *style_index = &style7;

                if (!detail)
                {
                    switch (key)
                    {
                        case KEY_UP:
                            *size_index = (*size_index + DEMO_SIZE_TIER_COUNT - 1) % DEMO_SIZE_TIER_COUNT;
                            demo_draw_grid(*size_index, *style_index);
                            break;

                        case KEY_DOWN:
                            *size_index = (*size_index + 1) % DEMO_SIZE_TIER_COUNT;
                            demo_draw_grid(*size_index, *style_index);
                            break;

                        case KEY_LEFT:
                            *style_index = (*style_index + DEMO_STYLE_COUNT - 1) % DEMO_STYLE_COUNT;
                            demo_draw_grid(*size_index, *style_index);
                            break;

                        case KEY_RIGHT:
                            *style_index = (*style_index + 1) % DEMO_STYLE_COUNT;
                            demo_draw_grid(*size_index, *style_index);
                            break;

                        case KEY_ENTER:
                        case KEY_RETURN:
                            detail = true;
                            demo_draw_detail(*size_index, *style_index);
                            break;

                        default:
                            break;
                    }
                }
                else if (key == KEY_ENTER || key == KEY_RETURN || key == KEY_ESC)
                {
                    detail = false;
                    demo_draw_grid(*size_index, *style_index);
                }
            }
            else   // SCREEN_14SEG
            {
                int *size_index = &size14;
                int *style_index = &style14;

                if (!detail)
                {
                    switch (key)
                    {
                        case KEY_UP:
                            *size_index = (*size_index + DEMO14_SIZE_TIER_COUNT - 1) % DEMO14_SIZE_TIER_COUNT;
                            demo14_draw_grid(*size_index, *style_index);
                            break;

                        case KEY_DOWN:
                            *size_index = (*size_index + 1) % DEMO14_SIZE_TIER_COUNT;
                            demo14_draw_grid(*size_index, *style_index);
                            break;

                        case KEY_LEFT:
                            *style_index = (*style_index + DEMO14_STYLE_COUNT - 1) % DEMO14_STYLE_COUNT;
                            demo14_draw_grid(*size_index, *style_index);
                            break;

                        case KEY_RIGHT:
                            *style_index = (*style_index + 1) % DEMO14_STYLE_COUNT;
                            demo14_draw_grid(*size_index, *style_index);
                            break;

                        case KEY_ENTER:
                        case KEY_RETURN:
                            detail = true;
                            demo14_draw_detail(*size_index, *style_index);
                            break;

                        default:
                            break;
                    }
                }
                else if (key == KEY_ENTER || key == KEY_RETURN || key == KEY_ESC)
                {
                    detail = false;
                    demo14_draw_grid(*size_index, *style_index);
                }
            }
        }

        tight_loop_contents();
    }
}
