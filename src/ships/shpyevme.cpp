/* $Id: shpyevme.cpp,v 1.16 2004/03/24 23:51:43 yurand Exp $ */
#include "../ship.h"
#include "../melee/mview.h"
REGISTER_FILE

class YevMech : public Ship
{
	public:

		int          SaberOn;
		int          ShieldOn;
		double       ShieldAngle;
		double       DegreesMoved;
		double       DegreeTreshhold;
		bool         SaberDestroyed;
		bool         ShieldDestroyed;

		double       SaberAngle;

		int          SaberSlash;
		int          SaberSlashCurrentFrame;
		int          SaberSlashTotalFrames;

		int          FramePassedR;
		int          FramePassedL;
		int                  FrameNotDirection;

		int          DTLowerLimit;
		int          DTHigherLimit;

		int          SaberAttack;

		YevMech(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code) ;
		Color crewPanelColor(int k = 0);

		virtual void calculate_turn_left();
		virtual void calculate_turn_right();

		virtual int activate_weapon();
		virtual int activate_special();

		virtual void calculate();
};

class YevShield : public SpaceObject
{
	YevMech  *ship;

	public:
		YevShield(YevMech *oship, SpaceSprite *osprite);

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual void animate(Frame* space);
		virtual int canCollide(SpaceLocation* other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);

};

Color YevMech::crewPanelColor(int k)
{
	STACKTRACE;
	Color c = {64,64,64};
	return c;
}


class YevSaber : public SpaceObject
{
	YevMech  *ship;
	int frame;
	int frame_size;
	int frame_count;
	int frame_step;

	public:
		YevSaber(YevMech *oship, SpaceSprite *osprite);

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual void animate(Frame* space);
		virtual int canCollide(SpaceLocation* other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);

};

YevMech::YevMech(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	DegreeTreshhold =  get_config_float("Shield", "DegreeTreshhold", 0);
	DTLowerLimit  = get_config_int("Extra", "DTLowerLimit", 0);
	DTHigherLimit = get_config_int("Extra", "DTHigherLimit", 0);

	SaberOn       = false;
	ShieldOn      = false;

	ShieldAngle   = 0;
	SaberAngle    = PI/4;

	SaberSlash             = false;
	SaberSlashCurrentFrame = 0;
	SaberSlashTotalFrames  = 300;

	SaberDestroyed  = false;
	ShieldDestroyed = false;

	SaberAttack   = 0;			 //0= default - sword spin; 1 = forward thrust; 2 = right slash; 3 = left slash

	FramePassedR          = 0;
	FramePassedL          = 0;
	FrameNotDirection = 0;

	add(new YevShield(this, data->spriteSpecial));
	add(new YevSaber(this,  data->spriteWeapon));

}


void YevMech::calculate_turn_left()
{
	STACKTRACE;
	if (!SaberDestroyed) {
		if (SaberOn ) {
			if (FrameNotDirection >= DTLowerLimit
				&& FrameNotDirection <= DTHigherLimit
				&& FramePassedR == 0
			&& FramePassedL > 0) {
				if (ship->batt > special_drain ) {

					Ship::calculate_turn_right();
					Ship::calculate_turn_right();
					//					 vx = ship->get_vx() + .25 * cos(angle-PI/2);
					//					 vy = ship->get_vy() + .25 * sin(angle-PI/2);
					vel = ship->get_vel() + 0.25 * unit_vector(angle-PI/2);

					ship->batt   -= special_drain ;
					FramePassedL = 0;
				}
			}
		}

		if (!fire_special || !ShieldOn)
			Ship::calculate_turn_left();
	} if (!fire_special || !ShieldOn)
	Ship::calculate_turn_left();

}


void YevMech::calculate_turn_right()
{
	STACKTRACE;

	if (!SaberDestroyed) {
		if (SaberOn ) {
			if (FrameNotDirection >= DTLowerLimit
				&& FrameNotDirection  <= DTHigherLimit
				&& FramePassedL == 0
			&& FramePassedR > 0) {
				if (ship->batt > special_drain ) {

					Ship::calculate_turn_left();
					Ship::calculate_turn_left();

					//						vx = ship->get_vx() + .25 * cos(angle+PI/2);
					//						vy = ship->get_vy() + .25 * sin(angle+PI/2);
					vel = ship->get_vel() + 0.25 * unit_vector(angle+PI/2);

					ship->batt -= special_drain ;

					FrameNotDirection = 0;
					FramePassedR      = 0;

				}
			}
		}

		if (!fire_special || !ShieldOn)
			Ship::calculate_turn_right();
	} if (!fire_special || !ShieldOn)
	Ship::calculate_turn_right();

}


int YevMech::activate_weapon()
{
	STACKTRACE;
	if (!SaberDestroyed) {
		if (SaberOn) {
			SaberSlash = TRUE;
		} else {
			ShieldOn = FALSE;
			SaberOn  = TRUE;
		}
		return(TRUE);
	} return(FALSE);

}


int YevMech::activate_special()
{
	STACKTRACE;
	if (!ShieldDestroyed) {
		if (!ShieldOn) {
								 // Start at left side @ 45 degree angle
			ShieldAngle = 315 * ANGLE_RATIO;
			DegreesMoved = 0;
			ShieldOn = TRUE;
			SaberOn  = FALSE;
			return(TRUE);
		}
		else return(FALSE);
	} return(FALSE);
}


void YevMech::calculate()
{
	STACKTRACE;
	if (turn_left) {
		FramePassedL += frame_time;
		FramePassedR = 0;
		FrameNotDirection = 0;
	}
	if (turn_right) {
		FramePassedR += frame_time;
		FramePassedL = 0;
		FrameNotDirection = 0;
	}
	if (!turn_left && !turn_right) FrameNotDirection += frame_time;

	if (fire_special && ShieldOn ) {
		DegreesMoved += (PI2/64);

		if (DegreesMoved >= DegreeTreshhold) {
			ship->batt -= 1;
			DegreesMoved = 0;
		}
		if (ship->batt > 0)
			if (turn_left) ShieldAngle -= (PI2/64);
		else if (turn_right) ShieldAngle += (PI2/64);

	} else

	if (fire_special && SaberOn ) {
		if (turn_left) {
			//left slash
			//SaberAttack   = 3;
		}
		else if (turn_right) {
			//right slash
			//SaberAttack   = 2;
		}
		else if (thrust) {
			//forward thrust
			//SaberAttack   = 1;
		} else {
			//saber spin
			//SaberAttack   = 0;
		}

	}

	Ship::calculate();
}


YevShield::YevShield(YevMech *oship,SpaceSprite *osprite) :
SpaceObject(oship, oship->normal_pos(), 0.0, osprite),
ship(oship)
{
	STACKTRACE;
	layer = LAYER_SHOTS;

	id    = SPACE_SHOT;
	calculate();
	isblockingweapons = true;

}


void YevShield::calculate()
{
	STACKTRACE;

	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
		return;
	}

	//  x = ship->normal_x() + (cos(ship->get_angle() + ship->ShieldAngle) * width()  * .45);
	//  y = ship->normal_y() + (sin(ship->get_angle() + ship->ShieldAngle) * height() * .45);

	pos = ship->normal_pos() + 0.45 * product( unit_vector(ship->get_angle() + ship->ShieldAngle), get_size() );

	sprite_index = get_index(ship->get_angle() + ship->ShieldAngle);
}


int YevShield::canCollide(SpaceLocation* other)
{
	STACKTRACE;
	if (ship->ShieldOn) return true;
	else return false;
}


void YevShield::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (ship->ShieldOn) {
		if (!(other->isShip() || other->isAsteroid()) ) {
			damage(other, 9999);
		}
	}
	SpaceObject::inflict_damage(other);
}


void YevShield::animate(Frame* space)
{
	STACKTRACE;
	if (ship->ShieldOn) SpaceObject::animate(space);
}


int YevShield::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if (this->state==0) ship->ShieldDestroyed=true;
	return 0;
}


YevSaber::YevSaber(YevMech *oship,SpaceSprite *osprite) :
SpaceObject(oship, oship->normal_pos(), 0.0, osprite),
ship(oship)
{
	STACKTRACE;

	layer = LAYER_SPECIAL;
	isblockingweapons = true;
	calculate();
}


void YevSaber::calculate()
{
	STACKTRACE;
	int          si;

	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
		return;
	}

	//  x = ship->normal_x() + cos( ship->get_angle() ) * width()  * .40;
	//  y = ship->normal_y() + sin( ship->get_angle() ) * height() * .40;

	pos = ship->normal_pos() + 0.40 * product( unit_vector(ship->get_angle()), get_size() );

	if (ship->SaberSlash) ship->SaberSlashCurrentFrame += frame_time;

	switch (ship->SaberAttack) {
		case 0:
			if (ship->SaberSlashCurrentFrame == 0)  si = 0;
			else {
				if (ship->SaberSlashCurrentFrame <= ship->SaberSlashTotalFrames * .75) si = 1;
				else
				if (ship->SaberSlashCurrentFrame <= ship->SaberSlashTotalFrames * .81) si = 2;
					else
					if (ship->SaberSlashCurrentFrame <= ship->SaberSlashTotalFrames * .87) si = 3;
						else
						if (ship->SaberSlashCurrentFrame <= ship->SaberSlashTotalFrames * .93) si = 4;
						else {
							si = 3;
					ship->SaberSlashCurrentFrame = 0;
					ship->SaberSlash = FALSE;
				}
				break;
				default:
					si = 0;

					break;
			}

	}
	sprite_index = get_index(ship->get_angle()) + (64 * si) ;

}


int YevSaber::canCollide(SpaceLocation* other)
{
	STACKTRACE;
	if (ship->SaberOn && other != ship) return true;
	else return false;
}


void YevSaber::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (ship != NULL)
	if (ship->SaberSlash && ship->SaberOn ) {
		if (ship->batt > 1) {
			damage(other, ship->batt);
			ship->batt = 0;
			sound.play(ship->data->sampleExtra[1]);

		} else {
			damage(other, 1);
			//sound.play(ship->data->sampleExtra[0]);
			sound.play(data->sampleExtra[0]);
		}

	}
}


void YevSaber::animate(Frame* space)
{
	STACKTRACE;
	if (!ship->SaberOn) return;
	SpaceObject::animate(space);
}


int YevSaber::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if (this->state==0) ship->SaberDestroyed=true;
	return iround(normal + direct);
}


REGISTER_SHIP(YevMech)
