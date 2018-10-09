/*
This file is part of "TW-Light"
					https://tw-light.appspot.com/
Copyright (C) 2001-2018 TimeWarp development team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include <cstdint>
#include <allegro.h>

#include "melee.h"

#include "melee/mframe.h"
#include "melee/mgame.h"
#include "melee/mmain.h"
#include "melee/mcbodies.h"
#include "melee/mview.h"

class AsteroidMelee : public NormalGame {
	virtual void init_objects();
	};

void AsteroidMelee::init_objects() {
	
	int i;
	size *= 1;
	prepare();
	add(new Stars());
	for (i = 0; i < 100; i += 1) add(new Asteroid());
	}

REGISTER_GAME ( AsteroidMelee, "Melee in Asteroid Field");

class DeepSpaceMelee : public NormalGame {
	virtual void init_objects();
	};

void DeepSpaceMelee::init_objects() {
	
	add ( new Stars() );
	add ( new Asteroid() );
	}

REGISTER_GAME ( DeepSpaceMelee, "Melee in Deep Space");
