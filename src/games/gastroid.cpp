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
