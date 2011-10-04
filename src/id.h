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

#ifndef __ID_H__
#define __ID_H__

#define SPACE_LOCATION 0x00000000

#define BASE_MASK1            0xF0000000
#define BASE_MASK2            0xFF000000
#define BASE_MASK3            0xFFF00000
#define SPACE_MASK            0xFFFF0000

#define SPACE_OBJECT          0x10000000
#define SPACE_LINE            0x20000000

#define ID_CONTROL            0x01100000
#define ID_SHIP_PANEL         0x01200000

#define SPACE_SHIP            0x11000000
#define SPACE_SHOT            0x12000000
#define SPACE_HOMING_MISSILE  0x12100000

#define SPACE_LASER           0x21000000

#define ID_PLANET          0x10000001
#define ID_ASTEROID        0x10000002

// Ship ID masks
#define ANDROSYNTH_SHIP 0x0001
#define ARILOU_SHIP     0x0002
#define CHENJESU_SHIP   0x0003
#define CHMMR_SHIP      0x0004
#define DRUUGE_SHIP     0x0005
#define EARTHLING_SHIP  0x0006
#define ILWRATH_SHIP    0x0007
#define KOHRAH_SHIP     0x0008
#define KZERZA_SHIP     0x0009
#define MELNORME_SHIP   0x000A
#define MMRNMHRMX_SHIP  0x000B
#define MMRNMHRMY_SHIP  0x000C
#define MYCON_SHIP      0x000D
#define ORZ_SHIP        0x000E
#define PKUNK_SHIP      0x000F
#define SHOFIXTI_SHIP   0x0010
#define SLYLANDRO_SHIP  0x0011
#define SPATHI_SHIP     0x0012
#define SUPOX_SHIP      0x0013
#define SYREEN_SHIP     0x0014
#define THRADDASH_SHIP  0x0015
#define UMGAH_SHIP      0x0016
#define UTWIG_SHIP      0x0017
#define VUX_SHIP        0x0018
#define YEHAT_SHIP      0x0019
#define ZOQFOTPIK_SHIP  0x001A

// Shot ID masks
#define ANDROSYNTH_SHOT  0x1020
#define CHENJESU_SHOT    0x1021
#define DRUUGE_SHOT      0x1022
#define EARTHLING_SHOT   0x1023
#define KOHRAHBLADE_SHOT 0x1024
#define KOHRAHFRIED_SHOT 0x1025
#define KZERZA_SHOT      0x1026
#define MELNORME_SHOT    0x1027
#define MELNORMEDIS_SHOT 0x1028
#define MYCON_SHOT       0x1029
#define ORZ_SHOT         0x102A
#define THRADDASH_SHOT   0x102B
#define THRADFLAME_SHOT  0x102C
#define ZOQFOTPIK_SHOT   0x102D

// Special masks
#define CHENDOGI_SPEC    0x2031
#define CHMMR_SPEC       0x2030
#define KZERZAFIGHT_SPEC 0x2032
#define ORZMARINE_SPEC   0x2033
#define SYREENCREW_SPEC  0x2034
#define VUXLIMPET_SPEC   0x2035

// Laser ID masks
#define ARILOU_LASER     0x3040
#define CHMMR_LASER      0x3041
#define SLYLANDRO_LASER  0x3042
#endif							 // __ID_H__
