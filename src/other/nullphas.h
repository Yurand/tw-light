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

#ifndef __NULLPHAS_H__
#define __NULLPHAS_H__

#include "melee/mship.h"

class NullPhaser : public Phaser
{
	// this phaser instantly calls the materialize function

	public:
		NullPhaser( Ship* oship );

								 // it does not show
		virtual void animate( Frame* space );
		virtual void calculate();// it instantly dies and adds the ship
};
#endif							 // __NULLPHAS_H__
