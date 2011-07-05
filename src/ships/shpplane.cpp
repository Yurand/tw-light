/* $Id: shpplane.cpp,v 1.11 2004/03/24 23:51:42 yurand Exp $ */
#include "../ship.h"
#include "../melee/mship.h"
#include "../other/orbit.h"
REGISTER_FILE
#include "../melee/mcbodies.h"

/*
 * created by: cyhawk@sch.bme.hu and forevian@freemail.hu
 *
 * Yes, this is a joke.

Adapted by GeomanNL:
special = super-gravwhip (can be used to frustrate enemy attack plans)
weapon = change moon properties (distance)
*/

class PlanetShip : public Ship
{
	int     planet_index;
	double  Rmin, Rmax, R, dRplus, dRmin, v;
	double  accboost_max;

	Planet  *planet, *moon;
	OrbitHandler    *orbit;

	Planet  *nearest_other_planet();

	public:
		double  accboost;

		PlanetShip(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual Color crewPanelColor(int k = 0);
		virtual int  activate_weapon();
		virtual double isInvisible() const;
		virtual void calculate_hotspots();
		virtual void calculate();
		virtual int handle_damage( SpaceLocation* other, double normal, double direct );
		virtual void inflict_damage( SpaceObject* other );
		virtual int accelerate_gravwhip( SpaceLocation *source, double angle,
			double velocity, double max_speed );
		virtual void materialize();
		virtual void death();
};

class InvisiblePlanet : public Planet
{
	public:
		PlanetShip *mother;
		double gravity_whip_default;
		InvisiblePlanet( PlanetShip* creator, Vector2 opos,
			SpaceSprite *sprite, int index );

		virtual void animate( Frame* space );
		virtual void calculate();
};

class Moon : public Planet
{
	public:
		PlanetShip *mother;
		Moon( PlanetShip* creator, Vector2 opos, SpaceSprite *sprite, int index );
		virtual void calculate();
};

PlanetShip::PlanetShip( Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code ):
Ship( opos, shipAngle, shipData, code )
{
	STACKTRACE;
	sprite_index = planet_index = random() % 3;
	if ( sprite_index != 0 ) {
		BITMAP *bmp = spritePanel->get_bitmap( 0 );
		blit( spritePanel->get_bitmap( 6 + sprite_index ), bmp, 0, 0, 0, 0, 63, 99 );
		update_panel = TRUE;
	}

	Rmin = scale_range(get_config_float("Weapon", "Rmin", 0));
	Rmax = scale_range(get_config_float("Weapon", "Rmax", 0));
	v = scale_velocity(get_config_float("Weapon", "Velocity", 0));

	dRplus = scale_range(get_config_float("Weapon", "DRplus", 0));
	dRmin = scale_range(get_config_float("Weapon", "DRmin", 0));

	accboost_max = get_config_float("Special", "GravwhipBoost", 0);
	accboost = 0;

	R = Rmin;

	moon = new Moon (this, pos+Vector2(500,0), data->spriteWeapon, 0);

	orbit = new OrbitHandler(this, pos, 0.0,
		this, moon, R, v, 0);

	//game->add(moon);   in materialize
	//game->add(orbit);

	planet = new InvisiblePlanet( this, pos, sprite, sprite_index );

	//data->sampleSpecial = 0;	// I don't like the sound of this.
	special_sample = -1;
}


double PlanetShip::isInvisible() const
{
	return (special_recharge > 0) ? 1 : 0;
}


void PlanetShip::calculate_hotspots()
{
	STACKTRACE;
	return;
}


void PlanetShip::calculate()
{
	STACKTRACE;

	Ship::calculate();

	// super gravwhip
	if (special_recharge > 0)	 // if special is active
		accboost = accboost_max;
	else
		accboost = 0;

	// affect moon orbit:
	if (weapon_recharge > 0)
		R += dRplus * frame_time*1E-3;
	else
		R -= dRmin * frame_time*1E-3;

	if (R < Rmin)   R = Rmin;
	if (R > Rmax)   R = Rmax;

	orbit->Radius = R;

	sprite_index = planet_index;
	//  planet->x = x;
	//  planet->y = y;
	planet->pos = pos;
	//  planet->vx = vx;
	//  planet->vy = vy;
	planet->vel = vel;
}


void PlanetShip::materialize()
{
	STACKTRACE;
	planet->translate( pos );

	game->add( planet );
	game->add(moon);
	game->add(orbit);
}


void PlanetShip::death()
{
	STACKTRACE;
	planet->state = 0;
}


int PlanetShip::handle_damage(SpaceLocation *other, double normal, double direct)
{
	STACKTRACE;
	//  if ( weapon_recharge > 0 ) damage = 0;
	return Ship::handle_damage( other , normal, direct);
}


void PlanetShip::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	// planet-damage should be a property (quirk) rather than a weapon...
	//	if ( weapon_recharge <= 0 ) return;
	int i = 1;
	if (other->isShip()) {
		i = (int) ceil(((Ship*)other)->crew / 3.0);
	}
	if (other->mass == 0) other->state = 0;
	damage(other, 0, i);
	i /= 2;
	if (i >= BOOM_SAMPLES) i = BOOM_SAMPLES - 1;
	if (!other->isShot())
		play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	return;
}


int PlanetShip::accelerate_gravwhip( SpaceLocation *source, double angle,
double velocity, double max_speed )
{
	STACKTRACE;

	Planet *p = nearest_other_planet();
	if ( !p ) return SpaceLocation::accelerate( source, angle, velocity, max_speed );
	double tmp;
	tmp = distance( p ) / p->gravity_range;
	if ( tmp > 1 ) return SpaceLocation::accelerate( source, angle, velocity, max_speed );
	return SpaceLocation::accelerate( source, angle, velocity,
		max_speed * (p->gravity_whip * tmp + 1) );
}


Planet *PlanetShip::nearest_other_planet()
{
	STACKTRACE;
	Planet *p = NULL;
	double r = 99999999;
	Query q;
	q.begin( this, bit(LAYER_CBODIES), 1600 );
	while( q.current ) {
		if ( q.current->isPlanet() && q.currento != planet ) {
			double t = distance(q.current);
			if ( t < r ) {
				r = t;
				p = (Planet*)q.current;
			}
		}
		q.next();
	}
	q.end();
	return p;
}


int PlanetShip::activate_weapon()
{
	STACKTRACE;
	//handle_damage(this, 5, 0); only for testing crew panel
	return true;
}


Color PlanetShip::crewPanelColor(int k)
{
	STACKTRACE;
	Color c = {255-k, k, 0};
	return c;
}


InvisiblePlanet::InvisiblePlanet( PlanetShip *creator, Vector2 opos,
SpaceSprite *sprite, int index ):Planet( opos, sprite, index)
{
	STACKTRACE;
	collide_flag_sameship = collide_flag_sameteam = collide_flag_anyone = 0;
	change_owner( creator );
	mother = creator;

	// less than the normal grav-whip, otherwise you're too helpful by default already ;)
	gravity_whip_default = 0.5 * gravity_whip;
}


void InvisiblePlanet::animate( Frame* space )
{
	STACKTRACE;
}


void InvisiblePlanet::calculate()
{
	STACKTRACE;
	// just exclude the PlanetShip from the gravity effect

	if (!(mother && mother->exists())) {
		mother = 0;
		state = 0;
	}

	// super-gravwhip, its special is active
	//if ( mother->special_recharge > 0 )
	//else
	//	gravity_whip = gravity_whip_default;

	SpaceObject::calculate();
	SpaceObject *o;
	Query a;

	a.begin(this, OBJECT_LAYERS, gravity_range);

	for (;a.currento;a.next()) {
		o = a.currento;

		if (o->mass > 0 && o != ship) {
			double r = distance(o);
			if (r < gravity_mindist) r = gravity_mindist;
			double sr = 1;

			//gravity_power truncated here
			if (gravity_power > 0) {
				r /= 40 * 4;
				for (int i = 0; i < gravity_power; i += 1) sr *= r;

				if (o->isShip() && ((Ship*)o)->thrust)
					o->accelerate(this, o->angle, mother->accboost * ((Ship*) o)->accel_rate, mother->accboost * MAX_SPEED);

				o->accelerate(this, trajectory_angle(o) + PI, frame_time * gravity_force / sr, MAX_SPEED);

			} else {
				r = 1 - r/gravity_range;
				for (int i = 0; i < -gravity_power; i += 1) sr *= r;
				o->accelerate(this, trajectory_angle(o) + PI, frame_time * gravity_force * sr, MAX_SPEED);
			}
		}
	}
}


Moon::Moon( PlanetShip* creator, Vector2 opos, SpaceSprite *sprite, int index )
:
Planet(opos, sprite, index)
{
	STACKTRACE;
	mother = creator;
	id = 0;						 // so that isPlanet() doesn't return a id_planet.
}


void Moon::calculate()
{
	STACKTRACE;
	if (!(mother && mother->exists())) {
		mother = 0;
		state = 0;
		return;
	}

	// velocity and position are handled by the orbiter
	return;
}


REGISTER_SHIP(PlanetShip)
