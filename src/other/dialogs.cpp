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

#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif

#include "scp.h"
#include "dialogs.h"
#include "melee.h"
#include "gui.h"
#include "melee/mcontrol.h"
#include "melee/moptions.h"
#include "melee/mgame.h"

DIALOG joyDialog[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	{ d_agup_box_proc,        40,   40,   200,  250,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ d_agup_textbox_proc,    50,   50,   180,  30,   255,  0,    0,    0,       0,    0,    (char*)"Joystick Calibration", NULL, NULL },
	{ my_d_button_proc,  50,   100,  180,  30,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[0], NULL, NULL },
	{ my_d_button_proc,  50,   140,  100,  30,   255,  0,    0,    D_EXIT,  0,    0,    (char*)"Next Joystick", NULL, NULL },
	{ my_d_button_proc,  160,  140,  70,   30,   255,  0,    0,    D_EXIT,  0,    0,    (char*)"Done", NULL, NULL },
	{ d_agup_textbox_proc,    50,   180,  180,  100,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

DIALOG keyDialog[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	{ d_agup_textbox_proc,    0,    0,    160,  80,   255,  0,    0,    0,       0,    0,    dialog_string[0], NULL, NULL },
	{ my_d_button_proc,  60,   90,   500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[1], NULL, NULL },
	{ my_d_button_proc,  60,   120,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[2], NULL, NULL },
	{ my_d_button_proc,  60,   150,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[3], NULL, NULL },
	{ my_d_button_proc,  60,   180,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[4], NULL, NULL },
	{ my_d_button_proc,  60,   210,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[5], NULL, NULL },
	{ my_d_button_proc,  60,   240,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[6], NULL, NULL },
	{ my_d_button_proc,  60,   270,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[7], NULL, NULL },
	{ my_d_button_proc,  60,   300,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[8], NULL, NULL },
	{ my_d_button_proc,  60,   330,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[9], NULL, NULL },
	{ my_d_button_proc,  60,   360,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[10], NULL, NULL },
	{ my_d_button_proc,  60,   375,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[11], NULL, NULL },
	{ my_d_button_proc,  60,   390,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[12], NULL, NULL },
	{ my_d_button_proc,  60,   405,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[13], NULL, NULL },
	{ my_d_button_proc,  60,   420,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[14], NULL, NULL },
	{ my_d_button_proc,  60,   435,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[15], NULL, NULL },
	{ my_d_button_proc,  60,   450,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[16], NULL, NULL },

	{ my_d_button_proc,  180,  20,   180,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void*)"Accept Changes", NULL, NULL },
	{ my_d_button_proc,     180,  50,   180,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void*)"Cancel", NULL, NULL },
	{ my_d_button_proc,  360,  35,   200,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void*)"Calibrate Joysticks", NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

DIALOG tw_alert_dialog1[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	{ d_agup_box_proc,        180,  170,  280,  140,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ d_agup_textbox_proc,    185,  175,  270,  95,   255,  0,    0,    0,       80,   0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     250,  280,  160,  20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

DIALOG tw_alert_dialog2[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	{ d_agup_box_proc,        180,  170,  280,  140,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ d_agup_textbox_proc,    185,  175,  270,  95,   255,  0,    0,    0,       80,   0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     190,  275,  125,  30,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     325,  275,  125,  30,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,       1,    0,    NULL, NULL, NULL }
};

DIALOG tw_alert_dialog3[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	{ d_agup_box_proc,        180,  170,  280,  140,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ d_agup_textbox_proc,    185,  175,  270,  95,   255,  0,    0,    0,       80,   0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     230,  280,  50,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     290,  280,  50,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     350,  280,  50,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

DIALOG tw_alert_dialog4[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	{ d_agup_box_proc,        180,  170,  280,  140,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ d_agup_textbox_proc,    185,  175,  270,  95,   255,  0,    0,    0,       80,   0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     190,  280,  55,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     255,  280,  55,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     320,  280,  55,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     385,  280,  55,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

DIALOG tw_alert_dialog5[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	{ d_agup_box_proc,        180,  170,  380,  140,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ d_agup_textbox_proc,    185,  175,  370,  95,   255,  0,    0,    0,       80,   0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     190,  280,  55,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     255,  280,  55,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     320,  280,  55,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     385,  280,  55,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     450,  280,  100,  20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

DIALOG selectDialog[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	{ my_list_proc,      5,     5,   280,  400,  255,  0,    0,    D_EXIT,  0,    0,    (void *)fleetListboxGetter, NULL, NULL },
	{ d_agup_textbox_proc,    300,  10,   240,  80,   255,  0,    0,    0,       0,    0,    (void *)selectTitleString, NULL, NULL },
	{ my_d_button_proc,  330, 110,   180,  35,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Select ship", NULL, NULL },
	{ my_d_button_proc,  330, 165,   180,  35,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Random selection", NULL, NULL },
	{ my_d_button_proc,  330, 220,   180,  35,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Always random", NULL, NULL },
	{ my_d_button_proc,  330, 275,   180,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Ship Info", NULL, NULL },
	{ my_bitmap_proc,    388, 330,   64,   100,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};
/*------------------------------
		HELP DIALOG
------------------------------*/
DIALOG help_dialog[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)  (d2)  (dp)
	{ d_agup_box_proc,        40,  30,    480,  420,  255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
	{ d_agup_button_proc,     50,  40,    460,  40,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Exit this screen" , NULL, NULL },
	{ d_agup_textbox_proc,    50,  90,    460,  350,  255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,          0,    0,    NULL, NULL, NULL }
};

DIALOG options_dialog[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg) (bg) (key) (flags)  (d1)  (d2)  (dp)
	{ d_agup_box_proc,        40,    30,  190,  190,   255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ my_d_button_proc,  70,    40,  110,   40,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Done", NULL, NULL },
	{ my_d_button_proc,  50,    90,  170,   30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Video Mode", NULL, NULL },
	{ my_d_button_proc,  50,   130,  170,   30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Audio Settings", NULL, NULL },
	{ my_d_button_proc,  50,   170,  170,   30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Game && Rendering", NULL, NULL },
	//  { my_d_button_proc,  50,   210,  170,   30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Physics Settings", NULL, NULL },
	//  { my_d_button_proc,  50,   250,  170,   40,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Restore Defaults", NULL, NULL },
	{ d_tw_yield_proc,        0,    0,    0,    0,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,     0,    0,    255,  0,    0,    0,       1,    0,    NULL, NULL, NULL }
};

DIALOG confirmVideoDialog[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg) (bg) (key) (flags)  (d1)  (d2)  (dp)
	{ d_agup_box_proc,       120,  140,  368,  90,   255,  0,    0,     0,       0,    0,    NULL, NULL, NULL },
	{ d_agup_text_proc,      130,  150,  348,  30,   255,  0,    0,     0,       0,    0,    (void *)"Do you want to keep these settings?", NULL, NULL },
	{ my_d_button_proc, 130,  190,  174,  30,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Yes", NULL, NULL },
	{ my_d_button_proc,    294,  190,  174,  30,   255,  0,    0,D_EXIT,       0,    0,    (void *)"No", NULL, NULL },
	{ NULL,               0,    0,    0,   0,    255,  0,    0,     0,       3,    0,    NULL, NULL, NULL }
};

DIALOG video_dialog[13] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg) (bg) (key) (flags)  (d1)  (d2)  (dp)
	{							 //DIALOG_VIDEO_BOX
		d_agup_box_proc,        20,   20,  400, 400,   255,  0,    0,     0,       0,    0,    NULL, NULL, NULL
	},

	{							 //DIALOG_VIDEO_FULLSCREEN
		d_agup_check_proc,      190,  120, 160,  30,   255,  0,    0,     0,       0,    0,    (void *)"Full-screen ", NULL, NULL
	},

	{							 //DIALOG_VIDEO_RESTEXT
		d_agup_text_proc,       30,  120,  140,  30,   255,  0,    0,     0,       0,    0,    (void *)"Resolution", NULL, NULL
	},
	{							 //DIALOG_VIDEO_RESLIST
		d_list_proc2,      30,  145,  140, 115,   255,  0,    0,D_EXIT,       0,    0,    (void *) genericListboxGetter, NULL, resolution
	},
	{							 //DIALOG_VIDEO_BPPTEXT
		d_agup_text_proc,       30,  290,  100,  30,   255,  0,    0,     0,       0,    0,    (void *)"Color Depth", NULL, NULL
	},
	{							 //DIALOG_VIDEO_BPPLIST
		d_list_proc2,      30,  310,  100, 100,   255,  0,    0,D_EXIT,       0,    0,    (void *) genericListboxGetter, NULL, color_depth
	},

	{							 //DIALOG_VIDEO_EXIT
		my_d_button_proc,      32,  30,  100,   35,   255,  0,   0,D_EXIT,  0,    0,    (void *)"Exit", NULL, NULL
	},
	{							 //DIALOG_VIDEO_GET_DEFAULT
		my_d_button_proc,  143,  30,  260,  35,   255,  0,    0,D_EXIT,  0,    0,    (void *)"Restore Default", NULL, NULL
	},
	{							 //DIALOG_VIDEO_OK
		my_d_button_proc,   32,  70,  100,   35,   255,  0,   0,D_EXIT,  0,    0,    (void *)"Ok", NULL, NULL
	},

	{							 //DIALOG_VIDEO_GAMMA_TEXT
		d_agup_text_proc,       170,  310,  160,  20,   255,  0,   0,     0,       0,    0,    (void *)"Gamma Correction", NULL, NULL
	},
	{							 //DIALOG_VIDEO_GAMMA_SLIDER
		d_agup_slider_proc,     170,  330,  160,  15,   255,  0,   0,     0,       255,  0,    NULL, (void *)handleGammaSliderChange, NULL
	},
	{ d_tw_yield_proc,        0,    0,    0,    0,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,   0,    0,    0,    255,  0,    0,    0,       3,    0,    NULL, NULL, NULL }
};

DIALOG audio_dialog[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg) (bg) (key) (flags)  (d1)  (d2)  (dp)
	{ d_box_proc,        30,   50,  410, 140,   255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ my_d_button_proc,  100,  60,  80,   40,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"OK", NULL, NULL },
	{ my_d_button_proc,     200,  60,  80,   40,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Cancel", NULL, NULL },
	{ d_agup_check_proc,      40,  110,  160,  20,   255,  0,    0,    0,       0,    0,    (void *)"Sound Volume ", NULL, NULL },
	{ d_agup_slider_proc,     205, 110,  180,  15,   255,  0,    0,    0,       255,  0,    NULL, (void *)handleSoundSliderChange, NULL },
	{ d_agup_check_proc,      40,  140,  160,  20,   255,  0,    0,    0,       0,    0,    (void *)"Music Volume ", NULL, NULL },
	{ d_agup_slider_proc,     205, 140,  180,  15,   255,  0,    0,    0,       255,  0,    NULL, (void *)handleMusicSliderChange, NULL },
	{ d_tw_yield_proc,        0,    0,    0,    0,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,   0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

DIALOG old_optionsDialog[] =
{
	/* (proc)               (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)                        (dp2) (dp3)          */
	{ d_agup_box_proc,      28,  40,  460, 325, 255, 0,   0,    0,      0,   0,   NULL,                       NULL, NULL          },
	{ d_agup_text_proc,     40,  56,  160, 20,  255, 0,   0,    0,      0,   0,   (void *)"Star Depth",       NULL, NULL          },
	{ d_agup_slider_proc,   212, 56,  160, 16,  255, 0,   0,    0,      255, 0,   NULL,                       NULL, NULL          },
	{ d_agup_text_proc,     40,  94,  160, 20,  255, 0,   0,    0,      0,   0,   (void*)"Shot Relativity",   NULL, NULL          },
	{ d_agup_slider_proc,   212, 96,  160, 16,  255, 0,   0,    0,      1000,0,   NULL,                       NULL, NULL          },
	{							 //OPTIONS_DIALOG_FRIENDLY_FIRE,
		d_agup_check_proc,    40,  144, 184, 20,  255, 0,   0,    0,      1,   0,   (void*)"Friendly Fire",     NULL, NULL
	},
	{							 //OPTIONS_DIALOG_HIDE_CLOAKERS,
		d_agup_check_proc,    40,  170, 190, 14,  255, 0,   0,    0,      1,   0,   (void*)"Camera hides cloakers",     NULL, NULL
	},
	{							 //OPTIONS_DIALOG_3DPLANET,
		d_agup_check_proc,    40,  190, 180, 14,  255, 0,   0,    0,      1,   0,   (void*)"3D Planet",         NULL, NULL
	},
	{ d_agup_text_proc,     292, 244, 130, 20,  255, 0,   0,    0,      0,   0,   (void *)"View",             NULL, NULL          },
	{ d_agup_list_proc,     284, 264, 180, 90,  255, 0,   0,    0,      0,   0,   (void *) viewListboxGetter, NULL, NULL          },
	{ my_d_button_proc,400, 60,  80,  40,  255, 0,   0,    D_EXIT, 0,   0,   (void *)"OK",               NULL, NULL          },
	{ my_d_button_proc,   400, 116, 80,  40,  255, 0,   0,    D_EXIT, 0,   0,   (void *)"Cancel",           NULL, NULL          },

	{ d_agup_text_proc,      40, 216, 120, 20,  255, 0,   0,    0,      1,   0,   (void *)"Rendering Quality:", NULL, NULL          },

	{ d_agup_check_proc,     40, 236, 130, 20,  255, 0,   0,    0,      1,   0,   (void *)"Interpolation",    NULL, NULL          },
	{ d_agup_check_proc,     40, 260, 130, 20,  255, 0,   0,    0,      1,   0,   (void *)"Anti-Aliasing",    NULL, NULL          },
	{ d_agup_check_proc,     40, 284, 130, 20,  255, 0,   0,    0,      1,   0,   (void *)"AA:Non-integer",   NULL, NULL          },
	{ d_agup_check_proc,     40, 308, 130, 20,  255, 0,   0,    0,      1,   0,   (void *)"AA:Blend",         NULL, NULL          },
	{ d_agup_check_proc,     40, 332, 130, 20,  255, 0,   0,    0,      1,   0,   (void *)"AA:Alpha",         NULL, NULL          },

	{ d_tw_yield_proc, 0,   0,   0,   0,   255, 0,   0,    0,      0,   0,   NULL,                       NULL, NULL          },
	{ NULL,            0,   0,   0,   0,   0,   0,   0,    0,      0,   0,   NULL,                       NULL, NULL          }
};

DIALOG mainDialog[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	{ d_agup_shadow_box_proc, 40,   40,   180,  220,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ my_d_button_proc,  45,   45,   170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Melee" , NULL, NULL },
	{ my_d_button_proc,  45,   75,   170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"GOB" , NULL, NULL },
	{ my_d_button_proc,  45,   105,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Extended Menu" , NULL, NULL },
	{ my_d_button_proc,  45,   135,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Teams" , NULL, NULL },
	{ my_d_button_proc,  45,   165,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Options", NULL, NULL },
	{ my_d_button_proc,  45,   195,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Help", NULL, NULL },
	{ my_d_button_proc,  45,   225,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Exit", NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,       1,    0,    NULL, NULL, NULL }
};

char title_str[80];
DIALOG fleet_titleDialog[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	{ d_agup_box_proc,        180,  210,  280,  60,   255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ d_agup_edit_proc,       190,  220,  260,  10,   255,  0,    0,    0,       80,   0,    (void *) title_str, NULL, NULL },
	{ my_d_button_proc,  255,  240,  60,   18,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"OK", NULL, NULL },
	{ d_agup_button_proc,     325,  240,  60,   18,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Cancel", NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

/*
 *** MELEE_EX dialog section - begin
 */

//  - dialog structure
DIALOG select_game_dialog[] =
{
	// (dialog proc)     (x)  (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	{ d_shadow_box_proc, 160, 120,  320,  240,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ d_agup_text_proc,       180, 135,  280,  190,  255,  0,    0,    0,       0,    0,    (void *)"Select a game", NULL, NULL},
	{							 //doesn't hold the right value until main() begins
		d_list_proc2,      180, 155,  280,  190,  255,  0,    0,    D_EXIT,  0,    0,    (void *)genericListboxGetter, NULL, game_names
	},
	{ d_tw_yield_proc,   0,   0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,   0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

// MELEE_EX - dialog structure
DIALOG melee_ex_dialog[] =
{
	// (dialog proc)     (x)  (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	{ d_shadow_box_proc, 40,  40,   240,  160,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ my_d_button_proc,  50,   50,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Play Game" , NULL, NULL },
	{ my_d_button_proc,  50,   75,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Key Tester", NULL, NULL },
	{ my_d_button_proc,  50,  100,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Ship Info", NULL, NULL },
	{ my_d_button_proc,  50,  125,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Diagnostics", NULL, NULL },
	{ my_d_button_proc,     80,  150,  190,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Main Menu", NULL, NULL },
	{ d_tw_yield_proc,   0,   0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,   0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

// TEAMS - dialog structure
DIALOG teamsDialog[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	{ d_box_proc,        35,   35,   420,  385,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ d_agup_textbox_proc,    150,  40,   200,  25,   255,  0,    0,    0,       0,    0,    (void *)"Teams Dialog", NULL, NULL },
	{ d_agup_text_proc,       40,   70,   240,  160,  255,  0,    0,    D_EXIT,  0,    0,    (void *)" Player   Team Config Type", NULL, NULL },
	{ d_list_proc,       40,   85,   240,  145,  255,  0,    0,    D_EXIT,  0,    0,    (void *)playerListboxGetter, NULL, NULL },
	{ d_list_proc,       290,  70,   160,  160,  255,  0,    0,    D_EXIT,  0,    0,    (void *)controlListboxGetter, NULL, NULL },
	{ my_d_button_proc,  295,  240,  150,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Select Controller", NULL, NULL },
	{ my_d_button_proc,  50,   255,  220,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Change Team #", NULL, NULL },
	{ my_d_button_proc,  50,   285,  220,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Change Config #", NULL, NULL },
	{ my_d_button_proc,  50,   315,  220,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Edit Config", NULL, NULL },
	{ my_d_button_proc,  50,   345,  220,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Edit Fleet", NULL, NULL },
	{ my_d_button_proc,     90,   380,  220,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Main Menu", NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

// FLEET - dialog structure
DIALOG fleetDialog[] =
{
	// (dialog proc)           (x)  (y)   (w)   (h)   (fg)(bg)(key) (flags)    (d1)   (d2)        (dp)

	//FLEET_DIALOG_AVAILABLE_SHIPS_TEXT TODO specify font here in d2 I think
	{ d_agup_textbox_proc,          10,  10,   240,  20,   255,  0,    0,     0,       0,    0,    (void *)"Available Ships", NULL, NULL },
	//FLEET_DIALOG_SHIP_CATAGORIES_TEXT
	{ d_agup_textbox_proc,          10,  35,   240,  17,   255,  0,    0,     0,       0,    0,    (void *)"Ship Catagories:", NULL, NULL },
	//FLEET_DIALOG_TW_OFFICIAL_TOGGLE
	{ d_check_proc_fleeteditor,10,  52,   240,  16,   255,  0,    0,D_EXIT | D_SELECTED,0,    0,    (void *)"TimeWarp", NULL, NULL },
	//FLEET_DIALOG_TW_EXP_TOGGLE_TOGGLE
	{ d_check_proc_fleeteditor,10,  70,   240,  16,   255,  0,    0,D_EXIT | D_SELECTED,0,    0,    (void *)"The Ur-Quan Masters", NULL, NULL },
	//FLEET_DIALOG_TW_SPECIAL_TOGGLE
	{ d_check_proc_fleeteditor,10,  88,   240,  16,   255,  0,    0,D_EXIT | D_SELECTED,0,    0,    (void *)"Special", NULL, NULL },
	//FLEET_DIALOG_TWA_TOGGLE
	{ d_check_proc_fleeteditor,10,  106,   240,  16,   255,  0,    0,D_EXIT | D_SELECTED,0,    0,   (void *)"Alpha", NULL, NULL },
	//FLEET_DIALOG_SORTBY_TEXT1
	{ d_agup_textbox_proc,     10, 121,    64,  17,   255,  0,    0,     0,       0,    0,    (void *)"Sort By:", NULL, NULL },
	//FLEET_DIALOG_SORTBY_BUTTON1
	{ my_d_button_proc,      69, 121,   128,  17,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Cost", NULL, NULL },
	//FLEET_DIALOG_SORTBY_ASCENDING1
	{ my_d_button_proc,     197, 121,    16,  17,   255,  0,    0,D_EXIT,       0,    0,    (void *)"^", NULL, NULL },
	//FLEET_DIALOG_AVAILABLE_SHIPS_LIST
	{ scp_fleet_dialog_text_list_proc, 10,  141,   240, 227,   255,  0,    0,D_EXIT, 0, 0, (void *)shippointsListboxGetter, NULL, NULL },
	//FLEET_DIALOG_FLEET_SHIPS_LIST
	{ d_list_proc2,      390, 141,   240, 227,   255,  0,    0,D_EXIT,       0,    0,    (void *)fleetpointsListboxGetter, NULL, NULL },
	//FLEET_DIALOG_PLAYER_FLEET_BUTTON
	{ my_d_button_proc,  390,  10,   240,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Player 1 Fleet", NULL, NULL },
	//FLEET_DIALOG_PLAYER_FLEET_TITLE
	{ my_d_button_proc,  390,  40,   128,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Fleet Title", NULL, NULL },
	//FLEET_DIALOG_SAVE_BUTTON
	{ my_d_button_proc,  518,  40,    56,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Save", NULL, NULL },
	//FLEET_DIALOG_LOAD_BUTTON
	{ my_d_button_proc,  574,  40,    56,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Load", NULL, NULL },
	//FLEET_DIALOG_POINT_LIMIT_TEXT
	{ d_agup_textbox_proc,    390,  60,   128,  20,   255,  0,    0,     0,       0,    0,    (void *)"Point Limit", NULL, NULL },
	//FLEET_DIALOG_POINT_LIMIT_BUTTON
	{ my_d_button_proc,  518,  60,   112,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"300\0              ", NULL, NULL },
	//FLEET_DIALOG_CURRENT_POINTS_TEXT
	{ d_agup_textbox_proc,    390,  80,   128,  20,   255,  0,    0,     0,       0,    0,    (void *)"Current Points", NULL, NULL },
	//FLEET_DIALOG_CURRENT_POINTS_VALUE
	{ d_agup_textbox_proc,    518,  80,   112,  20,   255,  0,    0,     0,       0,    0,    (void *)"100\0              ", NULL, NULL },
	//FLEET_DIALOG_SORTBY_TEXT2
	{ d_agup_textbox_proc,    390, 120,    64,  20,   255,  0,    0,     0,       0,    0,    (void *)"Sort By:", NULL, NULL },
	//FLEET_DIALOG_SORTBY_BUTTON2
	{ my_d_button_proc,     454, 120,   128,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Cost\0             ", NULL, NULL },
	//FLEET_DIALOG_SORTBY_ASCENDING2
	{ my_d_button_proc,     582, 120,    16,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"^", NULL, NULL },
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	//FLEET_DIALOG_ADD_BUTTON
	{ my_d_button_proc,  270, 210,  100,   25,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Add", NULL, NULL },
	//FLEET_DIALOG_ADD_ALL_BUTTON
	{ my_d_button_proc,  270, 235,  100,   25,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Add All", NULL, NULL },
	//FLEET_DIALOG_CLEAR
	{ my_d_button_proc,  270, 265,  100,   25,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Remove", NULL, NULL },
	//FLEET_DIALOG_CLEARALL
	{ my_d_button_proc,  270, 290,  100,   25,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Remove All", NULL, NULL },
	//FLEET_DIALOG_SHIP_PICTURE_BITMAP
	{ scp_fleet_dialog_bitmap_proc, 10, 372,   85,   85,   255,  0,    0,    0,       0,    0,    (void *)NULL, NULL, NULL },
	//FLEET_DIALOG_SHIP_SUMMARY_TEXT
	{ d_agup_textbox_proc,    325, 372,  305,   85,   255,  0,    0,     0,       0,    0,    (void *)"Summary Text", NULL, NULL },
	//FLEET_DIALOG_BACK_BUTTON
	{ my_d_button_proc,      10, 460,   64,   20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Back", NULL, NULL },
	//FLEET_DIALOG_HELP_TEXT
	{ d_agup_textbox_proc,     74, 460,  556,   20,   255,  0,    0,     0,       0,    0,    (void *)"Help Text", NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,     0,       0,    0,    NULL, NULL, NULL },
	{							 /**/
		NULL,              0,    0,    0,    0,    255,  0,    0,     0,       0,    0,    NULL, NULL, NULL
	}
};

const char *sorttypes[] = { "Name", "Cost", "Origin", "Coders", NULL };

// SHIPVIEW - dialog structure
DIALOG shipviewDialog[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	{ d_agup_textbox_proc,    5,    5,    220,  40,   255,  0,    0,    0,       0,    0,    (void *)"Select a ship to examine", NULL, NULL },
	{ my_d_button_proc,  300,  5,    120,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Done", NULL, NULL },
	{ my_d_button_proc,  300,  45,   120,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Font Size", NULL, NULL },
	{ d_list_proc2,      430,  5,    120,  60,   255,  0,    0,    D_EXIT,  0,    0,    (void *) genericListboxGetter, NULL, sorttypes },
	{ d_list_proc2,      5,    50,   220,  420,  255,  0,    0,    D_EXIT,  0,    0,    (void *) fleetListboxGetter, NULL, NULL },
	{ d_agup_textbox_proc,    230,  110,  400,  160,  255,  0,    0,    0,       0,    0,    (void *) NULL, NULL, NULL },
	{ d_agup_textbox_proc,    230,  280,  400,  190,  255,  0,    0,    0,       0,    0,    (void *) NULL, NULL, NULL },
	{ d_tw_bitmap_proc,  230,  5,    64,   100,  255,  0,    0,    0,       0,    0,    (void *) NULL, NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

// DIAGNOSTICS - dialog structure
DIALOG diagnostics_dialog[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)  (d2)  (dp)
	{ d_box_proc,        0,    0,    640,  480,  255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
	{ my_d_button_proc,     10,   10,   300,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Exit Diagnostics Screen" , NULL, NULL },
	{ d_agup_text_proc,       15,   55,   620,  15,   255,  0,    0,    0,          0,    0,    (void *)"Compile-time Options", NULL, NULL },
	{ d_agup_textbox_proc,    10,   70,   620,  90,   255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
	{ d_agup_text_proc,       15,   165,  620,  15,   255,  0,    0,    0,          0,    0,    (void *)"Compile Times", NULL, NULL },
	{ d_agup_textbox_proc,    10,   180,  620,  90,   255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
	{ d_agup_text_proc,       15,   275,  620,  15,   255,  0,    0,    0,          0,    0,    (void *)"version.txt", NULL, NULL },
	{ d_agup_textbox_proc,    10,   290,  620,  90,   255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
	//  { d_agup_text_proc,       15,   275,  620,  15,   255,  0,    0,    0,          0,    0,    (void *)"Ship Abnormalities", NULL, NULL },
	//  { d_textbox_proc,    10,   290,  620,  90,   255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,          0,    0,    NULL, NULL, NULL }
};
