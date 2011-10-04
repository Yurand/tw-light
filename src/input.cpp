/*
This file is part of "TW-Light"
					http://tw-light.appspot.com/
Copyright (C) 2001-2004  TimeWarp development team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif

#include "melee.h"
REGISTER_FILE

#define JOY_NUM_BITS       3
#define JOY_NUM_SHIFT     12

#define JOY_ACTION_BITS    2
#define JOY_ACTION_SHIFT  10

#define JOY_BUTTON_BITS   10
#define JOY_BUTTON_SHIFT   0

#define JOY_STICK_BITS     5
#define JOY_STICK_SHIFT    5

#define JOY_AXI_BITS       5
#define JOY_AXI_SHIFT      0

#define MAX_JOY_BUTTONS 98

#define extract_bits(value,bits,shift) ((value >> shift) & ((1 << bits)-1))

#define is_joy_key(a) (extract_bits(a, JOY_NUM_BITS, JOY_NUM_SHIFT) != 0)

#ifndef NO_ALLEGRO_4
static char *key_string[128] =
{
								 //  0 -   3
	"(none)",     "A",          "B",          "C",
								 //  4 -   7
	"D",          "E",          "F",          "G",
								 //  8 -  11
	"H",          "I",          "J",          "K",
								 // 12 -  15
	"L",          "M",          "N",          "O",
								 // 16 -  19
	"P",          "Q",          "R",          "S",
								 // 20 -  23
	"T",          "U",          "V",          "W",
								 // 24 -  27
	"X",          "Y",          "Z",          "0",
								 // 28 -  31
	"1",          "2",          "3",          "4",
								 // 32 -  35
	"5",          "6",          "7",          "8",
								 // 36 -  39
	"9",          "0_PAD",      "1_PAD",      "2_PAD",
								 // 40 -  43
	"3_PAD",      "4_PAD",      "5_PAD",      "6_PAD",
								 // 44 -  47
	"7_PAD",      "8_PAD",      "9_PAD",      "F1",
								 // 48 -  51
	"F2",         "F3",         "F4",         "F5",
								 // 52 -  55
	"F6",         "F7",         "F8",         "F9",
								 // 56 -  59
	"F10",        "F11",        "F12",        "ESC",
								 // 60 -  63
	"TILDE",      "MINUS",      "EQUALS",     "BACKSPACE",
								 // 64 -  67
	"TAB",        "OPENBRACE",  "CLOSEBRACE", "ENTER",
								 // 68 -  71
	"COLON",      "QUOTE",      "BACKSLASH",  "BACKSLASH2",
								 // 72 -  75
	"COMMA",      "STOP",       "SLASH",      "SPACE",
								 // 76 -  79
	"INSERT",     "DEL",        "HOME",       "END",
								 // 80 -  83
	"PGUP",       "PGDN",       "LEFT",       "RIGHT",
								 // 84 -  87
	"UP",         "DOWN",       "SLASH_PAD",  "ASTERISK",
								 // 88 -  91
	"MINUS_PAD",  "PLUS_PAD",   "DEL_PAD",    "ENTER_PAD",
	"PRTSCR",					 //  92
	"PAUSE",					 //  93
	"ABNT_C1",					 //  94
	"YEN",						 //  95
	"KANA",						 //  96
	"CONVERT",					 //  97
	"NOCONVERT",				 //  98
	"AT",						 //  99
	"CIRCUMFLEX",				 // 100
	"COLON2",					 // 101
	"KANJI",					 // 102
	"LSHIFT",					 // 103
	"RSHIFT",					 // 104
	"LCONTROL",					 // 105
	"RCONTROL",					 // 106
	"ALT",						 // 107
	"ALTGR",					 // 108
	"LWIN",						 // 109
	"RWIN",						 // 110
	"MENU",						 // 111
	"SCRLOCK",					 // 112
	"NUMLOCK",					 // 112
	"CAPSLOCK",					 // 114
	"",							 // 115
	"", "", "", "",				 // 116-119
	"","","","","","","",""		 //120-127
};
#else
static char *key_string[128] =
{
								 // 0-3*
	"(none)",     "ESC",        "1",          "2",
								 // 4-7
	"3",          "4",          "5",          "6",
								 // 8-11
	"7",          "8",          "9",          "0",
								 //12-15
	"MINUS",      "EQUALS",     "BACKSPACE",  "TAB",
								 //16-19*
	"Q",          "W",          "E",          "R",
								 //20-23
	"T",          "Y",          "U",          "I",
								 //24-27
	"O",          "P",          "OPENBRACE",  "CLOSEBRACE",
								 //28-31
	"ENTER",      "RCONTROL",   "A",          "S",
								 //32-35*
	"D",          "F",          "G",          "H",
								 //36-39
	"J",          "K",          "L",          "COLON",
								 //40-43
	"QUOTE",      "TILDE",     "LSHIFT",     "BACKSLASH",
								 //44-47
	"Z",          "X",          "C",          "V",
								 //48-51*
	"B",          "N",          "M",          "COMMA",
								 //52-55
	"STOP",       "SLASH",      "RSHIFT",     "ASTERISK",
								 //56-59
	"SLASH_PAD",  "SPACE",      "CAPSLOCK",   "F1",
								 //60-63
	"F2",         "F3",         "F4",         "F5",
								 //64-67*
	"F6",         "F7",         "F8",         "F9",
								 //68-71
	"F10",        "(none)",     "(none)",     "7_PAD",
								 //72-75
	"8_PAD",            "9_PAD",      "(none)",     "4_PAD",
								 //76-79
	"5_PAD",      "6_PAD",      "PLUS_PAD",   "1_PAD",
								 //80-83*
	"DOWN",       "PGDN",       "INS_PAD",    "DEL_PAD",
								 //84-87
	"(none)",     "(none)",     "(none)",     "F11",
								 //88-91
	"F12",        "(none)",     "(none)",     "(none)",
								 //92-94
	"(none)",     "(none)",     "(none)",     "(none)",
								 //96-99*
	"(none)",     "(none)",     "(none)",     "(none)",
								 //100-103
	"(none)",     "(none)",     "(none)",     "(none)",
								 //104-107
	"(none)",     "(none)",     "(none)",     "(none)",
								 //108-111
	"(none)",     "(none)",     "(none)",     "(none)",
								 //112-115*
	"(none)",     "(none)",     "(none)",     "(none)",
								 //116-119
	"(none)",     "(none)",     "(none)",     "(none)",
								 //120-123
	"(none)",     "(none)",     "(none)",     "(none)",
								 //124-127
	"(none)",     "(none)",     "(none)",     "(none)"
};
#endif

static int enabled_input_devices = 0;
static int failed_input_devices = 0;

int enable_input (int which)
{
	int failed = 0;
	if (which & 1) {
		if (install_keyboard() < 0) {
			failed |= 1;
			log_debug("keyboard failed initialization\n");
			allegro_message("Can't find keyboard.\nStart with the -nokeyboard option to avoid this message");
		} else {
			log_debug("keyboard initialized\n");
			poll_keyboard();
			set_keyboard_rate(0,0);
		}
	}

	if (which & 2) {
		if (install_mouse() < 0) {
			failed |= 2;
			log_debug("mouse failed initialization\n");
			allegro_message("Can't find mouse.\nStart with the -nomouse option to avoid this message");
		} else {
			log_debug("mouse initialized\n");
			poll_mouse();
		}
	}

	if (which & 4) {
		if (install_joystick(JOY_TYPE_AUTODETECT) < 0) {
			failed |= 4;
			log_debug("joystick failed initialization\n");
		} else {
			log_debug("joystick initialized (%d joysticks)\n", num_joysticks);
			poll_joystick();
		}
	}

	enabled_input_devices |= which &~ failed;
	failed_input_devices |= failed;
	return enabled_input_devices;
}


int disable_input(int which)
{
	STACKTRACE;
	int failed = 0;
	which &= enabled_input_devices;
	if (which & 1) {
		remove_keyboard();
		log_debug("keyboard disabled\n");
	}
	if (which & 2) {
		remove_mouse();
		log_debug("mouse disabled\n");
	}
	if (which & 4) {
		remove_joystick();
		log_debug("joystick disabled\n");
	}
	enabled_input_devices &= ~which;
	failed_input_devices |= failed;
	return enabled_input_devices;
}


void poll_input()
{
	STACKTRACE;
	if (enabled_input_devices & 1) poll_keyboard();
	if (enabled_input_devices & 2) poll_mouse();
	if (enabled_input_devices & 4) poll_joystick();
}


int key_to_description(Key which_key, char *dest_buffer)
{
	STACKTRACE;
	char _buffer[512];
	char *buffer = &_buffer[0];

	if (which_key < 0)
		return -1;
	if (which_key < 128)
		sprintf(buffer, "%s", key_string[which_key]);

	if (is_joy_key(which_key)) {
		int joy_num    = extract_bits(which_key, JOY_NUM_BITS, JOY_NUM_SHIFT) - 1;
		int joy_action = extract_bits(which_key, JOY_ACTION_BITS, JOY_ACTION_SHIFT);
		int joy_stick  = extract_bits(which_key, JOY_STICK_BITS, JOY_STICK_SHIFT);
		int joy_axi    = extract_bits(which_key, JOY_AXI_BITS, JOY_AXI_SHIFT);
		int joy_button = extract_bits(which_key, JOY_BUTTON_BITS, JOY_BUTTON_SHIFT);

		if (joy_action == 0) {
			if ((joy_num >= num_joysticks) || (joy_button >= joy[joy_num].num_buttons))
				sprintf(buffer, "??? Joystick %d | Button %d",
					joy_num, joy_button);
			else
				sprintf(buffer, "Joystick %d | Button %d (%s)",
					joy_num, joy_button, joy[joy_num].button[joy_button].name);
		} else {
			if ((joy_num >= num_joysticks) ||
				(joy_stick >= joy[joy_num].num_sticks) ||
				(joy_axi   >= joy[joy_num].stick[joy_stick].num_axis))
				buffer += sprintf(buffer, "??? Joystick %d | Stick %d | Axi %d (",
					joy_num, joy_stick, joy_axi);
			else
				buffer += sprintf(buffer, "Joystick %d | Stick %d (%s) | axi %d (%s ", joy_num,
					joy_stick, joy[joy_num].stick[joy_stick].name,
					joy_axi, joy[joy_num].stick[joy_stick].axis[joy_axi].name);
			switch (joy_action) {
				case 1: sprintf(buffer, "negative)"); break;
				case 2: sprintf(buffer, "positive)"); break;
				case 3: sprintf(buffer, "center)"); break;
				default: return -1;
			}
		}
	}

	strncpy(dest_buffer, _buffer, 64);
	return 0;
}


int key_to_name(Key which_key, char *buffer)
{
	STACKTRACE;
	if (which_key < 0)
		return -1;
	if (which_key < 128) {
		sprintf(buffer, "%s", key_string[which_key]);
		return 0;
	}

	if (which_key & (255 << 8)) {
		int joy_num    = extract_bits(which_key, JOY_NUM_BITS, JOY_NUM_SHIFT) - 1;
		int joy_action = extract_bits(which_key, JOY_ACTION_BITS, JOY_ACTION_SHIFT);
		int joy_stick  = extract_bits(which_key, JOY_STICK_BITS, JOY_STICK_SHIFT);
		int joy_axi    = extract_bits(which_key, JOY_AXI_BITS, JOY_AXI_SHIFT);
		int joy_button = extract_bits(which_key, JOY_BUTTON_BITS, JOY_BUTTON_SHIFT);

		if (joy_action == 0) {
			if (joy_button > MAX_JOY_BUTTONS)
				return -1;
			sprintf(buffer, "Joystick%d_button%d", joy_num, joy_button);
			return 0;
		}

		sprintf(buffer, "Joystick%d_stick%d_axi%d_action%d",
			joy_num, joy_stick, joy_axi, joy_action);
		return 0;
	}
	return -1;
}


Key name_to_key(const char *name)
{
	STACKTRACE;
	int i = 0;

	if (!name)
		return 0;

	if (!strncmp(name, "Joystick", 8)) {
		int n, t, b;
		n = name[8] - '0';
		if (n <= 0) return 0;
		if (n >= bit(JOY_NUM_BITS)) return 0;
		if (!strncmp(name+9, "_stick", 4)) {
			t = name[13] - '0';
			if (t < 0) return 0;
			if (t >= bit(JOY_STICK_BITS)) return 0;
			if (strncmp(name+9, "_axi", 4)) return 0;
			b = name[13] - '0';
			if (b < 0) return 0;
			if (b >= bit(JOY_AXI_BITS)) return 0;
			if (strncmp(name+14, "_action", 7)) return 0;
			if (name[22] != '\0') return 0;
			int a = name[21] - '0';
			if (a < 1) return 0;
			if (a >= bit(JOY_ACTION_BITS)) return 0;
			return ((n+1) << JOY_NUM_SHIFT) |
				(a << JOY_ACTION_SHIFT) |
				(t << JOY_STICK_SHIFT) |
				(b << JOY_AXI_SHIFT);
		}

		if (!strncmp(name+9, "_button", 7)) {
			t = name[16] - '0';
			b = name[17] - '0';
			if (t < 0) return 0;
			if (t > 9) return 0;
			if (name[17]) b = b + t * 10 - 1;
			else b = t;
			if (b < 0) return 0;
			if (b > 98) return 0;
			return ((n+1) << JOY_NUM_SHIFT) | (b << JOY_BUTTON_SHIFT);
		}
	}

	i = 0;
	while (true) {
		if (!key_string[i]) return 0;
		if (!strcmp(name, key_string[i])) return i;
		i++;
	}
}


//no overflow check, better have plenty of room
//the max possible is 2048
//typical is 6-12
int joykey_enumerate(Key *keys)
{
	STACKTRACE;
	if (!(enabled_input_devices & 4))
		return 0;

	int n = 0;
	int nb = 0;
	int ns = 0;
	int na = 0;
	int which_joystick;

	for (which_joystick = 1; which_joystick <= num_joysticks; which_joystick++) {
		nb = joy[which_joystick-1].num_buttons;
		if (nb > MAX_JOY_BUTTONS)
			nb = MAX_JOY_BUTTONS;
		ns = joy[which_joystick-1].num_sticks;

		int t, r = 0;
		int i, j;
		for (i = 0; i < nb; i += 1) {
			t = (which_joystick<<JOY_NUM_SHIFT) | (i<<JOY_BUTTON_SHIFT);
			keys[n] = t;
			n++;
		}

		for (i = 0; i < ns; i += 1) {
			na = joy[which_joystick-1].stick[i].num_axis;
			for (j = 0; j < na; j++) {
				t = (which_joystick<<JOY_NUM_SHIFT) |
					(i << JOY_STICK_SHIFT) |
					(j << JOY_AXI_SHIFT);
				r = t | (1<<JOY_ACTION_SHIFT);
				keys[n] = r;
				n++;
				r = t | (2<<JOY_ACTION_SHIFT);
				keys[n] = r;
				n++;
			}
		}
	}
	return n;
}


Key get_key()
{
	STACKTRACE;
	int n, i;
	Key joykeys[2048];
	char joykey_states[2048];
	char key_states[128];

	poll_input();
	n = joykey_enumerate(joykeys);

	// retreive keyboard key states
	for (i = 0; i < 128; i++)
		key_states[i]= key_pressed(i);

	// retrieve joystick "key" states
	for (i = 0; i < n; i++) {
		int t;
		if (key_pressed(joykeys[i])) t = 1; else t = 0;
		joykey_states[i] = t;
	}

	while (true) {
		poll_input();

		for (i = 0; i < 128; i++) {
			int t = key_pressed(i);
			if (t && !key_states[i]) return i;
			key_states[i] = t;
		}

		for (i = 0; i < n; i++) {
			int t;
			if (key_pressed(joykeys[i])) t = 1; else t = 0;
			int r = joykey_states[i];
			if (t && !r) return joykeys[i];
			joykey_states[i] = t;
		}
	}
}


int key_pressed(Key which_key)
{
	STACKTRACE;
	if (which_key & 0x8000) {
		tw_error ("key_pressed - bad key");
		return 0;
	}

	if (which_key & 0xff00) {
		int joy_num    = extract_bits(which_key, JOY_NUM_BITS, JOY_NUM_SHIFT) - 1;
		int joy_action = extract_bits(which_key, JOY_ACTION_BITS, JOY_ACTION_SHIFT);
		int joy_stick  = extract_bits(which_key, JOY_STICK_BITS, JOY_STICK_SHIFT);
		int joy_axi    = extract_bits(which_key, JOY_AXI_BITS, JOY_AXI_SHIFT);
		int joy_button = extract_bits(which_key, JOY_BUTTON_BITS, JOY_BUTTON_SHIFT);

		if ((joy_num >= num_joysticks) || (joy_num < 0)) return 0;

		if (joy_action == 0) {
			if (joy_button >= joy[joy_num].num_buttons) return 0;
			if (joy_button > MAX_JOY_BUTTONS) return 0;
			return joy[joy_num].button[joy_button].b;
		}

		if (joy_stick >= joy[joy_num].num_sticks) return 0;
		if (joy_axi >= joy[joy_num].stick[joy_stick].num_axis) return 0;

		switch (joy_action) {
			case 1: return joy[joy_num].stick[joy_stick].axis[joy_axi].d1;
			case 2: return joy[joy_num].stick[joy_stick].axis[joy_axi].d2;
			case 3: return joy[joy_num].stick[joy_stick].axis[joy_axi].pos;
			default: return 0;
		}
		tw_error ("key_pressed - bad key???");
		return 0;
	}

	if (which_key < KEY_MAX)
		return key[which_key];
	else
		return 0;
}
