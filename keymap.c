#include "keymap.h"

struct keydesc
{
	char normal, shifted;
};

#define SPECIAL_VAL(code, description) { '\0', '\0' }
#define NORMAL_CHAR(code, normal, shifted) { normal, shifted }
#define SINGLE_CHAR(code, character, description) { character, character }
#define ONLYSHIFTED(code, character, description) { '\0', character }
#define NOT_SHIFTED(code, character, description) { character, '\0' }

static const struct keydesc const key_to_char_map[] =
{
	SPECIAL_VAL(0x00, No Event),
	SPECIAL_VAL(0x01, Overrun Error),
	SPECIAL_VAL(0x02, POST Fail),
	SPECIAL_VAL(0x03, ErrorUndefined),
	NORMAL_CHAR(0x04, 'a', 'A'),
	NORMAL_CHAR(0x05, 'b', 'B'),
	NORMAL_CHAR(0x06, 'c', 'C'),
	NORMAL_CHAR(0x07, 'd', 'D'),
	NORMAL_CHAR(0x08, 'e', 'E'),
	NORMAL_CHAR(0x09, 'f', 'F'),
	NORMAL_CHAR(0x0A, 'g', 'G'),
	NORMAL_CHAR(0x0B, 'h', 'H'),
	NORMAL_CHAR(0x0C, 'i', 'I'),
	NORMAL_CHAR(0x0D, 'j', 'J'),
	NORMAL_CHAR(0x0E, 'k', 'K'),
	NORMAL_CHAR(0x0F, 'l', 'L'),
	NORMAL_CHAR(0x10, 'm', 'M'),
	NORMAL_CHAR(0x11, 'n', 'N'),
	NORMAL_CHAR(0x12, 'o', 'O'),
	NORMAL_CHAR(0x13, 'p', 'P'),
	NORMAL_CHAR(0x14, 'q', 'Q'),
	NORMAL_CHAR(0x15, 'r', 'R'),
	NORMAL_CHAR(0x16, 's', 'S'),
	NORMAL_CHAR(0x17, 't', 'T'),
	NORMAL_CHAR(0x18, 'u', 'U'),
	NORMAL_CHAR(0x19, 'v', 'V'),
	NORMAL_CHAR(0x1A, 'w', 'W'),
	NORMAL_CHAR(0x1B, 'x', 'X'),
	NORMAL_CHAR(0x1C, 'y', 'Y'),
	NORMAL_CHAR(0x1D, 'z', 'Z'),
	NORMAL_CHAR(0x1E, '1', '!'),
	NORMAL_CHAR(0x1F, '2', '@'),
	NORMAL_CHAR(0x20, '3', '#'),
	NORMAL_CHAR(0x21, '4', '$'),
	NORMAL_CHAR(0x22, '5', '%'),
	NORMAL_CHAR(0x23, '6', '^'),
	NORMAL_CHAR(0x24, '7', '&'),
	NORMAL_CHAR(0x25, '8', '*'),
	NORMAL_CHAR(0x26, '9', '('),
	NORMAL_CHAR(0x27, '0', ')'),
	SINGLE_CHAR(0x28, '\n',   Return),
	SINGLE_CHAR(0x29, '\033', Escape),
	SINGLE_CHAR(0x2A, '\010', Backspace),
	SINGLE_CHAR(0x2B, '\t',   Tab),
	SINGLE_CHAR(0x2C, ' ',    Space),
	NORMAL_CHAR(0x2D, '-', '_'),
	NORMAL_CHAR(0x2E, '=', '+'),
	NORMAL_CHAR(0x2F, '[', '{'),
	NORMAL_CHAR(0x30, ']', '}'),
	NORMAL_CHAR(0x31, '\\', '|'),
	SPECIAL_VAL(0x32, Europe 1), // Typically next to the Enter key, AT-101: 42
	NORMAL_CHAR(0x33, ';', ':'),
	NORMAL_CHAR(0x34, '\'', '"'),
	NORMAL_CHAR(0x35, '`', '~'),
	NORMAL_CHAR(0x36, ',', '<'),
	NORMAL_CHAR(0x37, '.', '>'),
	NORMAL_CHAR(0x38, '/', '?'),
	SPECIAL_VAL(0x39, Caps Lock),
	SPECIAL_VAL(0x3A, F1),
	SPECIAL_VAL(0x3B, F2),
	SPECIAL_VAL(0x3C, F3),
	SPECIAL_VAL(0x3D, F4),
	SPECIAL_VAL(0x3E, F5),
	SPECIAL_VAL(0x3F, F6),
	SPECIAL_VAL(0x40, F7),
	SPECIAL_VAL(0x41, F8),
	SPECIAL_VAL(0x42, F9),
	SPECIAL_VAL(0x43, F10),
	SPECIAL_VAL(0x44, F11),
	SPECIAL_VAL(0x45, F12),
	SPECIAL_VAL(0x46, Print Screen),
	SPECIAL_VAL(0x47, Scroll Lock),
	SPECIAL_VAL(0x48, Pause/Break),
	SPECIAL_VAL(0x49, Insert),
	SPECIAL_VAL(0x4A, Home),
	SPECIAL_VAL(0x4B, Page Up),
	SPECIAL_VAL(0x4C, Delete),
	SPECIAL_VAL(0x4D, End),
	SPECIAL_VAL(0x4E, Page Down),
	SPECIAL_VAL(0x4F, Right Arrow),
	SPECIAL_VAL(0x50, Left Arrow),
	SPECIAL_VAL(0x51, Down Arrow),
	SPECIAL_VAL(0x52, Up Arrow),
	SPECIAL_VAL(0x53, Num Lock),
	SINGLE_CHAR(0x54, '/',  Keypad /),
	SINGLE_CHAR(0x55, '*',  Keypad *),
	SINGLE_CHAR(0x56, '-',  Keypad -),
	SINGLE_CHAR(0x57, '+',  Keypad +),
	SINGLE_CHAR(0x58, '\n', Keypad Enter),
	ONLYSHIFTED(0x59, '1',  Keypad 1 End),
	ONLYSHIFTED(0x5A, '2',  Keypad 2 Down),
	ONLYSHIFTED(0x5B, '3',  Keypad 3 PageDn),
	ONLYSHIFTED(0x5C, '4',  Keypad 4 Left),
	ONLYSHIFTED(0x5D, '5',  Keypad 5),
	ONLYSHIFTED(0x5E, '6',  Keypad 6 Right),
	ONLYSHIFTED(0x5F, '7',  Keypad 7 Home),
	ONLYSHIFTED(0x60, '8',  Keypad 8 Up),
	ONLYSHIFTED(0x61, '9',  Keypad 9 PageUp),
	ONLYSHIFTED(0x62, '0',  Keypad 0 Insert),
	ONLYSHIFTED(0x63, '.',  Keypad . Delete),
	SPECIAL_VAL(0x64, Europe 2), // Typically between LShift and Z, AT-101: 45
	SPECIAL_VAL(0x65, App),
	SPECIAL_VAL(0x66, Keyboard Power),
	SINGLE_CHAR(0x67, '=',  Keypad =), // I'm assuming behaviour for this key
};

char get_char_for_key(unsigned char modifiers, unsigned char key_code)
{
	if ( key_code >= sizeof(key_to_char_map)/sizeof(struct keydesc) )
	{
		// We don't have any characters this high up.
		return '\0';
	}
	if ( ( modifiers & MOD_SHIFT ) != modifiers )
	{
		// Non-shift modifier key pressed - no corresponding character.
		return '\0';
	}
	const struct keydesc key = key_to_char_map[key_code];
	if ( modifiers & MOD_SHIFT )
	{
		return key.shifted;
	}
	return key.normal;
}
