/* $Id: shpartem.cpp,v 1.14 2005/08/02 00:23:40 geomannl Exp $ */
#include "../ship.h"
#include "../melee/mview.h"
REGISTER_FILE

class ArchTempest : public Ship
{
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;
		int          weaponWaves;
		int          ready_to_fire;

		double       specialRange;
		double       specialVelocity;
		int          specialDamage;
		int          specialArmour;
		double       specialTurnRate;

	public:
		ArchTempest(Vector2 pos, double angle, ShipData *data, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual void calculate_fire_weapon();
		virtual void calculate_fire_special();
		virtual void calculate_thrust();
		virtual void calculate_turn_left();
		virtual void calculate_turn_right();
		virtual void calculate_hotspots();
};

class TempestWave : public Missile
{
	public:

		ArchTempest *tempest;
		int wave_range;

		TempestWave(Vector2 pos, double oangle, double ov, int odamage,
			double orange, int oarmour, ArchTempest *oship, SpaceSprite *osprite);

		virtual void calculate();
		virtual void death();

};

ArchTempest::ArchTempest(Vector2 pos, double angle, ShipData *data, unsigned int code)
:
Ship(pos, angle, data, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	weaponWaves    = tw_get_config_int("Weapon", "Waves", 0);
	ready_to_fire = true;

	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage   = tw_get_config_int("Special", "Damage", 0);
	specialArmour   = tw_get_config_int("Special", "Armour", 0);
	specialTurnRate = scale_turning(tw_get_config_float("Special", "TurnRate", 0));
}


void ArchTempest::calculate()
{
	STACKTRACE;

	Ship::calculate();

}


int ArchTempest::activate_weapon()
{
	STACKTRACE;

	if (ready_to_fire) {
		add(new TempestWave(Vector2(0.0, (size.y / 2.0)), angle,
			weaponVelocity, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon) );
		ready_to_fire = false;
	}

	return(TRUE);
}


int ArchTempest::activate_special()
{
	STACKTRACE;

								 //&& !fire_weapon) // Requires more skill to fly
	if (target && !target->isInvisible())
		angle = trajectory_angle(target);

	return(TRUE);
}


void ArchTempest::calculate_fire_weapon()
{
	STACKTRACE;

	if (ready_to_fire)
		Ship::calculate_fire_weapon();
}


void ArchTempest::calculate_fire_special()
{
	STACKTRACE;

	// This is just to override any ini changes.  Special MUST be free with a constant rate.

	if (fire_special) activate_special();
	/*
		special_low = FALSE;

		if (fire_special)
		{

		  if (batt < special_drain) {
			special_low = TRUE;
			return;
		  }

		   if (special_recharge > 0)
			   return;

		if (!activate_special())
		  return;

		  batt -= special_drain;
		  special_recharge += special_rate;

		}
	*/

}


void ArchTempest::calculate_thrust()
{
	STACKTRACE;

	if (thrust && !fire_special) // Thrust like normal
		accelerate_gravwhip(this, angle, accel_rate * frame_time, speed_max);

	else if (thrust && fire_special) {

								 // Activate bottom-right jet
		if (turn_left && !turn_right)
			accelerate_gravwhip(this, angle - (PI/4), accel_rate * frame_time, speed_max);

								 // Activate bottom-left jet
		else if (turn_right && !turn_left)
			accelerate_gravwhip(this, angle + (PI/4), accel_rate * frame_time, speed_max);

		else					 // Thrust like normal
			accelerate_gravwhip(this, angle, accel_rate * frame_time, speed_max);

	}

	return;
}


void ArchTempest::calculate_turn_left()
{
	STACKTRACE;

	if (fire_special) {

								 // Activate right thruster
		if (turn_left && !turn_right)
			accelerate_gravwhip(this, angle - (PI/2), accel_rate * frame_time, speed_max);

		else if (turn_left && turn_right && !thrust) {
								 // Debatable
			vel = 0;
		}

	}

	else if (turn_left)
		turn_step -= turn_rate * frame_time;

}


void ArchTempest::calculate_turn_right()
{
	STACKTRACE;

	if (fire_special) {

								 // Activate left thruster
		if (turn_right && !turn_left)
			accelerate_gravwhip(this, angle + (PI/2), accel_rate * frame_time, speed_max);

		else if (turn_right && turn_left && !thrust) {
								 // Debatable
			vel = 0;
		}

	}

	else if (turn_right)
		turn_step += turn_rate * frame_time;

}


void ArchTempest::calculate_hotspots()
{
	STACKTRACE;

	// My ultra-cool hotspot code.  It 0wnz you.
	if (hotspot_frame <= 0) {

		if (fire_special) {

			if (turn_right && !turn_left) {

				game->add(new Animation(this,
					normal_pos() - (unit_vector(angle + (PI/2)) * size.y / 2.5),
					meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));

				if (thrust) {

					game->add(new Animation(this,
						normal_pos() - (unit_vector(angle + (PI/4)) * size.y / 2.5),
						meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));

				}

				hotspot_frame += hotspot_rate;

			}

			else if (turn_left && !turn_right) {

				game->add(new Animation(this,
					normal_pos() - (unit_vector(angle - (PI/2)) * size.y / 2.5),
					meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));

				if (thrust) {
					game->add(new Animation(this,
						normal_pos() - (unit_vector(angle - (PI/4)) * size.y / 2.5),
						meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));
				}

				hotspot_frame += hotspot_rate;

			}

			else if (thrust) {

				game->add(new Animation(this,
					normal_pos() - (unit_vector(angle) * size.x / 2.5),
					meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));
				hotspot_frame += hotspot_rate;
			}
		}

		else if (thrust) {
			game->add(new Animation(this,
				normal_pos() - (unit_vector(angle) * size.x / 2.5),
				meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));

			hotspot_frame += hotspot_rate;
		}

	}

	if (hotspot_frame > 0)
		hotspot_frame -= frame_time;

	return;
}


TempestWave::TempestWave(Vector2 pos, double oangle, double ov,
int odamage, double orange, int oarmour,
ArchTempest *oship, SpaceSprite *osprite) :
Missile(oship, pos, oangle, ov, odamage, orange, oarmour, oship, osprite)
{
	STACKTRACE;
	//explosionSprite     = data->spriteWeaponExplosion;
	//explosionFrameCount = 5;
	//explosionFrameSize  = 50;

	tempest = oship;
	wave_range = false;
	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void TempestWave::calculate()
{
	STACKTRACE;

	Missile::calculate();

								 // FIXME: Goddamn radians
	double shit_angle = angle * (180 / PI);

	if ( (d / range) > (.25) )
		sprite_index = iround(shit_angle / 5.625) + 64;
	if ( (d / range) > (.5) )
		sprite_index = iround(shit_angle / 5.625) + 128;
	if ( (d / range) > (.75) )
		sprite_index = iround(shit_angle / 5.625) + 192;

	sprite_index &= 255;

	if (!wave_range && tempest->exists()) {

		if (d > (range / tempest->weaponWaves) ) {
			tempest->ready_to_fire = true;
			wave_range = true;
		}

	}

}


void TempestWave::death()
{
	STACKTRACE;

	if (!wave_range && tempest->exists())
		tempest->ready_to_fire = true;

}


REGISTER_SHIP(ArchTempest)
