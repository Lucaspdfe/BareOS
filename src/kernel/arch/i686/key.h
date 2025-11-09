#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool isSpecial;
    uint8_t specialKey;
    char character;
} KEYState;

/**
 * Initialize keyboard driver and IRQ handler
 * Must be called before any other keyboard functions
 */
void i686_KEY_Initialize(void);

/**
 * Read current key (non-blocking)
 * \return State of key. If isSpecial and character is NUL, nothing has been pressed.
 */
KEYState i686_KEY_ReadKey();

/**
 * Checks if a key has been pressed.
 * \return boolean value that says if there is a non-read key.
 */
bool i686_KEY_CheckKey();

/**
 * Wait for key, then returns (blocking)
 * \return State of key.
 */
KEYState i686_KEY_WaitKey();

typedef enum {
    KEY_ESC            = 0,
    KEY_F1             = 1,
    KEY_F2             = 2,
    KEY_F3             = 3,
    KEY_F4             = 4,
    KEY_F5             = 5,
    KEY_F6             = 6,
    KEY_F7             = 7,
    KEY_F8             = 8,
    KEY_F9             = 9,
    KEY_F10            = 10,
    KEY_F11            = 11,
    KEY_F12            = 12,
    KEY_PRINT          = 13,
    KEY_NUM            = 14,
    KEY_CAPS           = 15,
    KEY_INSERT         = 16,
    KEY_SHIFTL         = 17,
    KEY_SHIFTR         = 18,
    KEY_CTRLL          = 19,
    KEY_CTRLR          = 20,
    KEY_ALTL           = 21,
    KEY_ALTR           = 22,
    KEY_SUPER          = 23,
    KEY_FN             = 24,
    KEY_TAB            = 25,
    KEY_ARROW_UP       = 26,
    KEY_ARROW_DOWN     = 27,
    KEY_ARROW_LEFT     = 28,
    KEY_ARROW_RIGHT    = 29,
} KEYSpecialID;
