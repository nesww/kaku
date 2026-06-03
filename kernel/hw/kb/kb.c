#include "kb.h"
#include "hw/vga/vga.h"
#include "hw/io/io.h"
#include "tty/tty.h"

static char kb_map[128] = {
  [KB_SC_PRESSED_ONE]   =  '1',
  [KB_SC_PRESSED_TWO]   =  '2',
  [KB_SC_PRESSED_THREE] =  '3',
  [KB_SC_PRESSED_FOUR]  =  '4',
  [KB_SC_PRESSED_FIVE]  =  '5',
  [KB_SC_PRESSED_SIX]   =  '6',
  [KB_SC_PRESSED_SEVEN] =  '7',
  [KB_SC_PRESSED_EIGHT] =  '8',
  [KB_SC_PRESSED_NINE]  =  '9',
  [KB_SC_PRESSED_ZERO]  =  '0',

  [KB_SC_PRESSED_DASH] = '-',
  [KB_SC_PRESSED_EQUAL] = '=',
  [KB_SC_PRESSED_BACKSPACE] = '\b',
  [KB_SC_PRESSED_TAB] = '\t',


  [KB_SC_PRESSED_Q] = 'q',
  [KB_SC_PRESSED_W] = 'w',
  [KB_SC_PRESSED_E] = 'e',
  [KB_SC_PRESSED_R] = 'r',
  [KB_SC_PRESSED_T] = 't',
  [KB_SC_PRESSED_Y] = 'y',
  [KB_SC_PRESSED_U] = 'u',
  [KB_SC_PRESSED_I] = 'i',
  [KB_SC_PRESSED_O] = 'o',
  [KB_SC_PRESSED_P] = 'p',


  [KB_SC_PRESSED_LEFT_SQ_BRACKETS]  = '[',
  [KB_SC_PRESSED_RIGHT_SQ_BRACKETS] = ']',
  [KB_SC_PRESSED_ENTER] = '\n',


  [KB_SC_PRESSED_A] = 'a',
  [KB_SC_PRESSED_S] = 's',
  [KB_SC_PRESSED_D] = 'd',
  [KB_SC_PRESSED_F] = 'f',
  [KB_SC_PRESSED_G] = 'g',
  [KB_SC_PRESSED_H] = 'h',
  [KB_SC_PRESSED_J] = 'j',
  [KB_SC_PRESSED_K] = 'k',
  [KB_SC_PRESSED_L] = 'l',

  [KB_SC_PRESSED_SEMICOLON]    = ';',
  [KB_SC_PRESSED_SINGLE_QUOTE] = '\'',
  [KB_SC_PRESSED_BACK_TICK]    = '`',
  [KB_SC_PRESSED_BACKSLASH]    = '\\',

  [KB_SC_PRESSED_Z] = 'z',
  [KB_SC_PRESSED_X] = 'x',
  [KB_SC_PRESSED_C] = 'c',
  [KB_SC_PRESSED_V] = 'v',
  [KB_SC_PRESSED_B] = 'b',
  [KB_SC_PRESSED_N] = 'n',
  [KB_SC_PRESSED_M] = 'm',

  [KB_SC_PRESSED_COMMA] = ',',
  [KB_SC_PRESSED_DOT] = '.',
  [KB_SC_PRESSED_SLASH] = '/',
  [KB_SC_PRESSED_KEYPAD_MULT] = '*',
  [KB_SC_PRESSED_SPACE] = ' ',

  [KB_SC_PRESSED_KEYPAD_PLUS]  = '+',
  [KB_SC_PRESSED_KEYPAD_MINUS] = '-',
  [KB_SC_PRESSED_KEYPAD_SEVEN] = '7',
  [KB_SC_PRESSED_KEYPAD_EIGHT] = '8',
  [KB_SC_PRESSED_KEYPAD_NINE]  = '9',
  [KB_SC_PRESSED_KEYPAD_FOUR]  = '4',
  [KB_SC_PRESSED_KEYPAD_FIVE]  = '5',
  [KB_SC_PRESSED_KEYPAD_SIX]   = '6',
  [KB_SC_PRESSED_KEYPAD_ONE]   = '1',
  [KB_SC_PRESSED_KEYPAD_TWO]   = '2',
  [KB_SC_PRESSED_KEYPAD_THREE] = '3',
  [KB_SC_PRESSED_KEYPAD_ZERO]  = '0',
  [KB_SC_PRESSED_KEYPAD_DOT]   = '.',
};

void kb_handle_interrupt() {
    uint8_t scancode = inb(IO_KEYBOARD_DATA_PORT);
    if (scancode >= 0x80) return;

    if (scancode == KB_SC_PRESSED_BACKSPACE) {
        // vga_backspace();
        tty_puts("backspace: not implemented\n");
    } else if (scancode == KB_SC_PRESSED_ENTER) {
        tty_putchar('\n');
    } else if (scancode == KB_SC_PRESSED_NOTHING) {
        return;
    } else {
        char c = kb_map[scancode];
        if (c != 0) tty_putchar(kb_map[scancode]);
    }
    tty_flush_current_line();
}
