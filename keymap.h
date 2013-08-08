#ifndef KEYMAP_H
#define KEYMAP_H

#define MOD_CONTROL 0x11
#define MOD_SHIFT   0x22
#define MOD_ALT     0x44
#define MOD_WINDOWS 0x88

#define MOD_LEFT  0x0F
#define MOD_RIGHT 0xF0

char get_char_for_key(unsigned char modifiers, unsigned char key_code);

#endif
