/* $Id: shpkolfl.cpp,v 1.16 2005/08/28 20:34:08 geomannl Exp $ */
#include "ship.h"
REGISTER_FILE
#include "melee/mview.h"

#include "frame.h"

/*

 */

class   Flamer;
class   GravityShockWave;

class KoloryFlamer : public Ship
{
	public:
		double weaponRange;
		double weaponVelocity;
		int    weaponDamage;
		int    weaponArmour;
		double weaponAccel;

		double  specialHalfTime;

		double slowdownfactor;

	public:
		Flamer  *flamer1, *flamer2;
		int     weapon1, weapon2, flame_active;

		KoloryFlamer(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();

		virtual void calculate();
		virtual void calculate_hotspots();

};

class Flamer : public Missile
{
	public:
		KoloryFlamer    *commandship;
		int     iSide;
		double  animate_time, nextpictime;
		int     sprite_offset, Nanimatedflame;
		double  FlameAccel;

	public:
		Flamer(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, double oaccel, Ship *oship, SpaceSprite *osprite, int thisiSide,
			KoloryFlamer *commandshipref);
		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

KoloryFlamer::KoloryFlamer(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	weaponRange         = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity      = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage        = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour        = tw_get_config_int("Weapon", "Armour", 0);
	weaponAccel         = scale_velocity(tw_get_config_float("Weapon", "Accelerate", 0));

	specialHalfTime     = tw_get_config_float("Special", "HalfTime", 0);

	flamer1 = 0;
	flamer2 = 0;
	weapon1 = 0;
	weapon2 = 0;

	flame_active = weapon1 | weapon2;
}


int KoloryFlamer::activate_weapon()
{
	STACKTRACE;
	weaponVelocity = 0.0;

	// check if the weapons exist
	weapon1 = flamer1 && flamer1->exists();
	weapon2 = flamer2 && flamer2->exists();
	// (must be repeated here, instead of calculate function, to avoid eternal recursion)

	if ( weapon1 & weapon2 )
		return FALSE;			 // couldn't be used.

	if ( !weapon1 ) {
		weapon1 = 1;
		flamer1 = new Flamer(
			Vector2(0.0, 0.5*get_size().y), angle, weaponVelocity, weaponDamage, weaponRange,
			weaponArmour, weaponAccel, this, data->spriteWeapon, 1, this);
		game->add( flamer1 );

	}
	if ( !weapon2 ) {
		weapon2 = 1;
		flamer2 = new Flamer(
			Vector2(0.0, -0.5*get_size().y), angle+PI, weaponVelocity, weaponDamage, weaponRange,
			weaponArmour, weaponAccel, this, data->spriteWeapon, 2, this);
		game->add( flamer2 );
	}

	return  TRUE;
}


int KoloryFlamer::activate_special()
{
	STACKTRACE;

	return(true);
}


void KoloryFlamer :: calculate ()
{

	// check if the weapons exist
	weapon1 = flamer1 && flamer1->exists();
	weapon2 = flamer2 && flamer2->exists();

	// update the flamer pointers
	if (!weapon1)   flamer1 = 0;
	if (!weapon2)   flamer2 = 0;

	flame_active = weapon1 | weapon2;

	Ship::calculate();

	// regardless of whether the flames are activated or not:
	if ( this->fire_special ) {	 // DEFENSIVE MODE

		//		double HalfTime = 1000.0 * 0.5;	// 0.5 second to half the speed.
								 // is nearly 1
		slowdownfactor = exp(-frame_time*1E-3 / specialHalfTime);

	} else {
		slowdownfactor = 1.0;
	}

	int layers = bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL) +
		bit(LAYER_CBODIES);
	double passiveRange = 1000.0;// outside this area, gravity doesn't do anything

	Query a;
	for (a.begin(this, layers, passiveRange); a.current; a.next()) {
		if (!a.current->isObject())
			continue;
		SpaceObject *o = a.currento;
		if (!(o->isPlanet()) && o != ship ) {

			// the special introduces drag around the ship, depending on distance

			o->vel *= slowdownfactor;

			if ( o->isShot() ) { // shot/missiles/homing-missiles have a different existtime/physics
				Shot *s = ((Shot*)o);
				double timeleft = (s->range - s->d) / s->v;
				s->v *= slowdownfactor;
								 // express in lifetime instead of range
				s->range = s->d + s->v * timeleft;
			}

		}
	}

}


void KoloryFlamer::calculate_hotspots()
{
	STACKTRACE;
	if ((thrust) && (hotspot_frame <= 0)) {
		Vector2 P;
		P = Vector2(-12,20);	 // note, y is down
		P = rotate(P, angle+PI/2);

		game->add(new Animation(this,
			normal_pos() + P,
			meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));

		P = Vector2(+12,20);
		P = rotate(P, angle+PI/2);

		game->add(new Animation(this,
			normal_pos() + P,
			meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));

		hotspot_frame += hotspot_rate;
	}

	if (hotspot_frame > 0)
		hotspot_frame -= frame_time;

}


// This is an intense flame with a short range.

Flamer::Flamer(Vector2 opos, double oangle, double ov,
int odamage, double orange, int oarmour, double oaccel, Ship *oship, SpaceSprite *osprite, int thisiSide,
KoloryFlamer *commandshipref)
:
Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship,osprite) ,
commandship(commandshipref),
iSide(thisiSide),
FlameAccel(oaccel)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 20;
	explosionFrameSize  = 50;

	sprite_offset = 0;
	animate_time = 0;
	nextpictime = 250.0;
	Nanimatedflame = 4;

	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void Flamer::calculate()
{
	STACKTRACE;
	if ( !commandship || !ship || !commandship->exists() || !ship->exists() ) {
		commandship = 0;
		state = 0;
		return;
	}

	Missile::calculate();

	// fix the location and direction of the flame with respect to the ship.

	double D = ( commandship->get_size().y + get_size().y ) / 2.0;
	if ( iSide == 1 ) {
		pos = commandship->pos + D * unit_vector(commandship->angle);
		vel = commandship->vel;	 // important for prediction?
		angle = commandship->angle;
	} else {
		pos = commandship->pos - D * unit_vector(commandship->angle);
		vel = commandship->vel;
		angle = commandship->angle + PI;
	}

								 // - 16;
	sprite_index = get_index(angle, -0.5*PI);
	sprite_index += sprite_offset * 64;

	animate_time += frame_time;
	if ( animate_time > nextpictime ) {
		animate_time -= nextpictime;
		sprite_offset += 1;
								 // hard coded ?!?!?!
		if ( sprite_offset > Nanimatedflame-1 )
			sprite_offset = 0;
	}

	// the active weapon also has influence on the commandship by providing thrust:

	//	commandship->vel += FlameAccel*unit_vector(angle+PI);
	commandship->accelerate_gravwhip(this, angle+PI, FlameAccel, MAX_SPEED);

	//	double v = magnitude(commandship->vel);
	//	if ( v > commandship->speed_max )
	//		commandship->vel *= commandship->speed_max / v;

}


int Flamer::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;

								 //iround(normal + direct);
	double totdamage = normal + direct;

	if (source->isShip())
		totdamage = armour;		 //iround(armour);

	armour -= totdamage;

	if ( armour <= 0 ) {
		state = 0;
		return iround(totdamage + armour);
	}

	return totdamage;
}


void Flamer::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	SpaceObject::inflict_damage(other);

	return;
}


REGISTER_SHIP(KoloryFlamer)
