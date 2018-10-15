/* $Id$ */ 

#include "../ship.h"
REGISTER_FILE

/* Ship-Howto. The following creates a ship class, for a ship which can just
fly around, and do nothing else. Note that the base class is Ship, which handles all I/O,
animation, and such.
*/

class HowTo_1 : public Ship
{
public:
IDENTITY(HowTo_1);
public:
	/*  constructor of a new ship.
	
	opos = a Vector2, (x,y) coordinate where the ship is located --> copied into "pos"
	shipAngle = the angle (in radian, so between -pi and pi) it faces when it's spawned --> copied into "angle"
	shipData = contains the images and sounds of the ship. These are initialized externally, using info in the data file. --> amonng this, sets the "sprite" data
	shipCollideFlag = keeps track of team ownership, and collision stuff.
	*/
	HowTo_1(Vector2 opos, double shipAngle, ShipData *shipData, int shipCollideFlag);
};

HowTo_1::HowTo_1(Vector2 opos, double shipAngle, ShipData *shipData, int shipCollideFlag)
:
Ship(opos, shipAngle, shipData, shipCollideFlag)
{
	debug_id = 3;	// for debugging purpose
};


/* The following registers a ship. The supplied identifier is used by the macro to
generate a code name-id, which can be called in ships.ini files so that the correct data
are loaded for the correct code.
*/

REGISTER_SHIP(HowTo_1);

