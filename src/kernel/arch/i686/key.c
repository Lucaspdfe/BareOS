#include "key.h"
#include "irq.h"
#include "io.h"

#define KEY_DATA    0x60
/* simple ring buffer for incoming scancodes to avoid races when multiple
    scancodes arrive before the consumer reads them (e.g. Shift + key). */
static volatile uint8_t kb_buf[256];
static volatile uint8_t kb_head = 0; /* write index */
static volatile uint8_t kb_tail = 0; /* read index */

/* helper: buffer empty when head == tail */
static inline bool kb_empty(void) { return kb_head == kb_tail; }
static inline bool kb_full(void) { return ((uint8_t)(kb_head + 1)) == kb_tail; }


/* state we track in the driver */
static bool saw_e0 = false; /* extended prefix */
static bool shiftPressed = false;
static bool capsLock = false;

void i686_KEY_Handler(Registers* regs) {
   /* read scancode from keyboard controller and push to ring buffer */
   uint8_t sc = i686_inb(KEY_DATA);
   uint8_t next = (uint8_t)(kb_head + 1);
   if (next != kb_tail) {
       kb_buf[kb_head] = sc;
       kb_head = next;
   } else {
       /* buffer full: drop scancode */
   }
}

void i686_KEY_Initialize(void) {
    i686_IRQ_RegisterHandler(1, i686_KEY_Handler);
}

typedef struct { char normal; char shifted; } KeyMap;

/* Partial scancode-set-1 mapping for common printable keys */
static const KeyMap keymap[128] = {
    [0x02] = {'1','!'},
    [0x03] = {'2','@'},
    [0x04] = {'3','#'},
    [0x05] = {'4','$'},
    [0x06] = {'5','%'},
    [0x07] = {'6','^'},
    [0x08] = {'7','&'},
    [0x09] = {'8','*'},
    [0x0A] = {'9','('},
    [0x0B] = {'0',')'},
    [0x0C] = {'-','_'},
    [0x0D] = {'=','+'},
    [0x0F] = {'\t','\t'},

    [0x10] = {'q','Q'},
    [0x11] = {'w','W'},
    [0x12] = {'e','E'},
    [0x13] = {'r','R'},
    [0x14] = {'t','T'},
    [0x15] = {'y','Y'},
    [0x16] = {'u','U'},
    [0x17] = {'i','I'},
    [0x18] = {'o','O'},
    [0x19] = {'p','P'},
    [0x1A] = {'[','{'},
    [0x1B] = {']','}'},

    [0x1E] = {'a','A'},
    [0x1F] = {'s','S'},
    [0x20] = {'d','D'},
    [0x21] = {'f','F'},
    [0x22] = {'g','G'},
    [0x23] = {'h','H'},
    [0x24] = {'j','J'},
    [0x25] = {'k','K'},
    [0x26] = {'l','L'},
    [0x27] = {';',':'},
    [0x28] = {'\'','"'},
    [0x29] = {'`','~'},
    [0x2B] = {'\\','|'},

    [0x2C] = {'z','Z'},
    [0x2D] = {'x','X'},
    [0x2E] = {'c','C'},
    [0x2F] = {'v','V'},
    [0x30] = {'b','B'},
    [0x31] = {'n','N'},
    [0x32] = {'m','M'},
    [0x33] = {',','<'},
    [0x34] = {'.','>'},
    [0x35] = {'/','?'},

    [0x39] = {' ',' '},
};

KEYState i686_KEY_ReadKey() {
    KEYState state;
    state.isSpecial = false;
    state.specialKey = 0;
    state.character = 0;

    if (kb_empty()) return state;

    uint8_t sc = kb_buf[kb_tail];
    kb_tail = (uint8_t)(kb_tail + 1);

    /* handle extended prefix */
    if (sc == 0xE0) {
        saw_e0 = true;
        return state; /* wait for next byte */
    }

    if (saw_e0) {
        /* extended scancode handling (common arrows) */
        saw_e0 = false;
        switch (sc) {
            case 0x48: state.isSpecial = true; state.specialKey = KEY_ARROW_UP; return state;
            case 0x50: state.isSpecial = true; state.specialKey = KEY_ARROW_DOWN; return state;
            case 0x4B: state.isSpecial = true; state.specialKey = KEY_ARROW_LEFT; return state;
            case 0x4D: state.isSpecial = true; state.specialKey = KEY_ARROW_RIGHT; return state;
            default: return state; /* unhandled extended */
        }
    }

    /* key release (high bit set in scancode-set-1) */
    if (sc & 0x80) {
        uint8_t code = sc & 0x7F;
        /* track shift releases */
        if (code == 0x2A || code == 0x36) shiftPressed = false;
        return state; /* releases don't generate characters */
    }

    /* key press */
    /* left/right shift */
    if (sc == 0x2A || sc == 0x36) { shiftPressed = true; return state; }

    /* caps lock */
    if (sc == 0x3A) { capsLock = !capsLock; state.isSpecial = true; state.specialKey = KEY_CAPS; return state; }

    /* Enter */
    if (sc == 0x1C) { state.character = '\n'; return state; }
    /* Backspace */
    if (sc == 0x0E) { state.character = '\b'; return state; }

    /* space */
    if (sc == 0x39) { state.character = ' '; return state; }

    /* printable mapping */
    if (sc < 128) {
        KeyMap km = keymap[sc];
        if (km.normal) {
            char base = km.normal;
            /* alphabetic -> consider capsLock xor shift */
            if (base >= 'a' && base <= 'z') {
                bool uppercase = (shiftPressed ^ capsLock);
                if (uppercase) state.character = (km.shifted ? km.shifted : (base - 'a' + 'A'));
                else state.character = base;
            } else {
                state.character = (shiftPressed ? (km.shifted ? km.shifted : km.normal) : km.normal);
            }
            return state;
        }
    }

    /* unhandled key -> no state change */
    return state;
}

bool i686_KEY_CheckKey() {
    return !kb_empty();
}

KEYState i686_KEY_WaitKey() {
    for (;;) {
        /* wait for at least one scancode to arrive */
        while (!i686_KEY_CheckKey()) {
            __asm__ volatile ("hlt");
        }

        KEYState st = i686_KEY_ReadKey();
        /* If ReadKey returned an actual event (printable char or special), return it.
           Otherwise (e.g. we consumed an 0xE0 prefix) keep waiting for the next scancode. */
        if (st.character != 0 || st.isSpecial) return st;
    }
}
