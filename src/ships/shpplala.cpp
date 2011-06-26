/* $Id: shpplala.cpp,v 1.6 2005/07/11 00:25:32 geomannl Exp $ */
/* todo: (in order)
	modify ship panel.
	Prevent main program from modifying ship panel (batt/crew)
	Implement 'Bio' and 'Data' on panel.
	Hot Spots/interialess selectable from INI
	differiate a between a 'planet mode' and 'space mode'
	Different configs = different point values.
	Add add-ons.  (shields etc.)
	Implement special: idea: collect resources, bio data with the lander.
		when full, unleash them. if both full then its some else.*/

/* Bugs:
	Main program panel draws over ship panel when battery empty (shouldn't be possible).
	Shot speed incorrect when start/stopping (relativity)
	Graphics display problems on rotation of ship/shot

   Disclaimer: I can't code.  At all.  I'm basically taking stuff from other
   ships and using it here. This means, theres probably a lot of usless
   garbage in here.  And a lot of other people get credit.  Some ships I've
   borrowed and modified code from: Jyglar Starfarer, Nissk Harraser,
   Bubulos Exectioner (which was based off the Xchagger), Arilou Skiff and
   Imperial Katana. Thats all that comes to mind. -rump*/

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class PlanetLander : public Ship
{
	public:
		double      weaponRange;
		double      weaponVelocity;
		int         weaponDamage;
		int         weaponArmour;

		int         shipConfig;
		int         interialessOn;
		int         hotspotsOn;
		double          shipVelocity;

		//	void show_panel( int r );

	public:
		PlanetLander (Vector2 opos, double angle, ShipData *data, unsigned int code);

	protected:
		virtual void calculate();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual int accelerate(SpaceLocation *source, double angle, double velocity, double max_speed);
		virtual int accelerate_gravwhip(SpaceLocation *source, double angle, double velocity, double max_speed);
								 // enable/disable hotspots
		virtual void calculate_hotspots();
};

								 /*Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code)*/
PlanetLander::PlanetLander(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	weaponRange             = scale_range(tw_get_config_int("Weapon", "Range", 0));
	weaponVelocity          = scale_velocity(tw_get_config_int("Weapon", "Velocity", 0));
	weaponDamage            = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour            = tw_get_config_int("Weapon", "Armour", 0);

	shipConfig              = tw_get_config_int("Configuration", "ActiveConfig",0);

	//interialessOn			= tw_get_config_int(
}


void PlanetLander::calculate()
{
	STACKTRACE;
	Ship::calculate();

	if (!thrust) vel = 0;		 //Poor man's interialess-drive

}


int PlanetLander::activate_weapon()
{
	STACKTRACE;
	add(new Missile(this, Vector2(0,
		(size.y*.25)),angle, weaponVelocity, weaponDamage,
		weaponRange, weaponArmour,this, data->spriteWeapon, 1));

	return(TRUE);
}


int PlanetLander::activate_special()
{
	STACKTRACE;
	//for future use.
	return(FALSE);
}


//This function will be used when updating the ship panel.
int PlanetLander::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;

	return Ship::handle_damage(source, normal, direct);

}


//This eliminates the hotspots.  There's probably a better way to do it... but I don't know how!
void PlanetLander::calculate_hotspots()
{
	STACKTRACE;
	// if ( thrust && hotspot_frame <= 0 ){
	/* game->add( new Animation( this,
		 normal_pos() - unit_vector(angle) * size.y / 4,
	   data->spriteExtra, 0, 12, time_ratio, LAYER_HOTSPOTS));
	   hotspot_frame += hotspot_rate;
	}
	if ( hotspot_frame > 0 ) hotspot_frame -= frame_time;*/
}


//These two funcitons negate gravity whips
int PlanetLander::accelerate(SpaceLocation *source, double angle, double velocity, double max_speed)
{
	STACKTRACE;
	if (source == this)
		return Ship::accelerate(source, angle, velocity, max_speed);
	return false;
}


int PlanetLander::accelerate_gravwhip(SpaceLocation *source, double angle, double velocity, double max_speed)
{
	STACKTRACE;
	if (source == this)
		return Ship::accelerate(source, angle, velocity, max_speed);
	return false;
}


REGISTER_SHIP(PlanetLander)
