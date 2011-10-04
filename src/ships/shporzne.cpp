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

#include "ship.h"
REGISTER_FILE

#include "melee/mview.h"
#include "melee/mcbodies.h"

#include "shporzne.h"

OrzNemesis::OrzNemesis(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	int i;

	absorption = 0;

	collide_flag_sameship = bit(LAYER_SPECIAL);
	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);

	specialSpeedMax    = scale_velocity(tw_get_config_float("Special", "SpeedMax", 0));
	double raw_specialHotspotRate = tw_get_config_int("Special", "HotspotRate", 0);
	specialHotspotRate = scale_frames(raw_specialHotspotRate);
	specialAccelRate   = scale_acceleration(tw_get_config_float("Special", "AccelRate", 0), raw_specialHotspotRate);
	specialArmour      = tw_get_config_int("Special", "Armour", 0);

	for(i = 0; i < MAX_MARINES; i++)
		marine[i] = NULL;

	turretAngle = 0.0;
	recoil = 0;

	recoil_rate = scale_frames(tw_get_config_float("Turret", "RecoilRate",0));
	if (recoil_rate > weapon_rate) recoil_rate = weapon_rate;
	recoil_range = tw_get_config_int("Turret", "Recoil", 0);
	if (recoil_range < 0) recoil_range = 0;
	turret_turn_rate = scale_turning(tw_get_config_float("Turret", "TurnRate", 0));

	turret_turn_step = 0;
}


void OrzNemesis::calculate_turn_left()
{
	STACKTRACE;
	if (!fire_special)
		Ship::calculate_turn_left();
}


void OrzNemesis::calculate_turn_right()
{
	STACKTRACE;
	if (!fire_special)
		Ship::calculate_turn_right();
}


int OrzNemesis::activate_weapon()
{
	STACKTRACE;
	if (fire_special)
		return(FALSE);
	add(new OrzMissile(
		angle + turretAngle, weaponVelocity,
		weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon));

	recoil += recoil_rate;

	return(TRUE);
}


int OrzNemesis::activate_special()
{
	STACKTRACE;
	if (turn_left && (recoil<=0))
		turret_turn_step -= frame_time * turret_turn_rate;
	if (turn_right && (recoil<=0))
		turret_turn_step += frame_time * turret_turn_rate;

	while (fabs(turret_turn_step) > (PI2/64)/2) {
		if (turret_turn_step < 0.0 ) {
			turretAngle -= (PI2/64);
			turret_turn_step += (PI2/64);
		} else {
			turretAngle += (PI2/64);
			turret_turn_step -= (PI2/64);
		}
	}
	turretAngle = normalize(turretAngle, PI2);

	if ((fire_weapon) && (crew > 1)) {
		int i = 0;

		while((marine[i] != NULL) && (i < MAX_MARINES))
			i++;
		if (i < MAX_MARINES) {
			marine[i] = new OrzMarine(pos, this, specialAccelRate,
				specialSpeedMax, specialHotspotRate, specialArmour, i, data->spriteSpecial);
			add(marine[i]);
			crew--;
			return(TRUE);
		}
	}

	return(FALSE);
}


void OrzNemesis::calculate()
{
	STACKTRACE;
	int i;

	for(i = 0; i < MAX_MARINES; i++)
		if ((marine[i]) && (!marine[i]->exists()))
			marine[i] = NULL;

	recoil -= frame_time;
	if (recoil < 0) recoil = 0;
	Ship::calculate();
}


void OrzNemesis::animate(Frame *space)
{
	STACKTRACE;
	double rec;
	int turret_index;
	/*
	ra = normalize(angle + turretAngle, PI2);
	turret_index = get_index(ra);
	bmp = data->spriteShip->get_bitmap(64);
	clear_to_color( bmp, tw_makecol(255,0,255));
	sprite->draw(0, 0, sprite_index, bmp);
	rec = (double)recoil/recoil_rate;
	rec *= rec * recoil_range;
	data->spriteExtra->draw( -cos(ra)*rec, -sin(ra)*rec, turret_index, bmp);
	data->spriteShip->animate(x,y,64, space);
	*/

	Ship::animate(space);
	turret_index = get_index(angle + turretAngle);
	rec = (double)recoil/recoil_rate;
	rec *= rec * recoil_range;
								 //  x - cos(angle+turretAngle)*rec, y - sin(angle+turretAngle)*rec
	data->spriteExtra->animate( pos - rec*unit_vector(angle+turretAngle),
		turret_index, space);

	return;
}


OrzMissile::OrzMissile(double oangle, double ov, int odamage, double orange,
int oarmour, Ship *oship, SpaceSprite *osprite) :
Missile(oship, Vector2(0.0, 0.0), oangle, ov, odamage, orange, oarmour, oship,
osprite)
{
	STACKTRACE;
	//  x += cos(angle) * 30.0;
	//  y += sin(angle) * 30.0;
	pos += 30.0 * unit_vector(angle);

	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 10;
	explosionFrameSize  = 50;

	add(new Animation(this, pos,
		data->spriteExtraExplosion, 0, 10, 30, DEPTH_EXPLOSIONS));
}


OrzMarine::OrzMarine(Vector2 opos, OrzNemesis *oship, double oAccelRate,
double oSpeedMax, int oHotspotRate, int oArmour, int oSlot,
SpaceSprite *osprite) :
SpaceObject(oship, opos, 0.0, osprite),
accel_rate(oAccelRate),
speed_max(oSpeedMax),
hotspot_rate(oHotspotRate),
hotspot_frame(0),
armour(oArmour),
invading(NULL),
returning(FALSE),
slot(oSlot),
damage_frame(-1)
{
	STACKTRACE;
	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);
	mass = 0.001;

	isblockingweapons = true;

	orzship = oship;
}


void OrzMarine::calculate()
{
	STACKTRACE;
	int    chance;

	if (!(orzship && orzship->exists())) {
		orzship = 0;
		// just this ... in case owner is changed, "ship" can still be used to do something
	}

	if (invading) {

		if (invading->exists()) {
			//			x = invading->normal_x();
			//			y = invading->normal_y();
			pos = invading->normal_pos();

			if (damage_frame > 0) {
				damage_frame -= frame_time;
				if (damage_frame <= 0 && invading->spritePanel) {
					sprite->draw(
						14 + ((slot % 4) * 6),  16 + ((slot / 4) * 6),
						0, invading->spritePanel->get_bitmap(0)
						);
					/*draw_sprite(invading->spritePanel->get_bitmap(0),	sprite->get_bitmap(0),
							14 + ((slot % 4) * 6), 16 + ((slot / 4) * 6));*/
					invading->update_panel = TRUE;
				}
				return;
			}

			chance = random() % 10000;
			if (chance < 9 * frame_time) {
				// the following is dangerous if eg a ploxis changes the owner (=ship pointer)
				//if (ship && ((random() & 255) < (((OrzNemesis*)ship)->absorption))) {
				if (orzship && orzship->exists() && (random() & 255) < orzship->absorption) {
					damage(orzship, 0, -1);
				}

				play_sound(data->sampleExtra[0]);
				damage(invading, 0, 1);
				damage_frame = 50;
				if ( invading->spritePanel ) {
					sprite->draw(
						14 + ((slot % 4) * 6),
						16 + ((slot / 4) * 6),
						1, invading->spritePanel->get_bitmap(0) );
					/*draw_sprite(invading->spritePanel->get_bitmap(0), sprite->get_bitmap(1),
							14 + ((slot % 4) * 6), 16 + ((slot / 4) * 6));*/
					invading->update_panel = TRUE;
				}
			}
			else    if (chance < 10 * frame_time) {
				state = 0;
				play_sound(data->sampleExtra[2]);
				if ( invading->spritePanel ) {
					tw_blit(invading->data->spritePanel->get_bitmap(0), invading->spritePanel->get_bitmap(0),
						14 + ((slot % 4) * 6), 16 + ((slot / 4) * 6),
						14 + ((slot % 4) * 6), 16 + ((slot / 4) * 6), 12, 12);
					invading->update_panel = TRUE;
				}
				return;
			}
		} else {
			invading = NULL;
			returning = TRUE;
			collide_flag_sameship = bit(LAYER_SHIPS);
			collide_flag_anyone = ALL_LAYERS;
			sprite_index = 1;
		}
	} else {
		if (!(ship && ship->exists())) {
			ship = 0;
			state = 0;
			return;
		}

		if (returning)
			angle = trajectory_angle(ship);
		else {
			if (ship->target && ship->target->exists() && (!ship->target->isInvisible()))
				angle = trajectory_angle(ship->target);
			else {
				returning = true;
				collide_flag_sameship = bit(LAYER_SHIPS);
				collide_flag_anyone = ALL_LAYERS;
				sprite_index = 1;
			}
		}

		Planet *spacePlanet = nearest_planet();
		if (spacePlanet!=NULL) {
			double r = distance(spacePlanet);
			if (r < 0.33*spacePlanet->gravity_range) {
				double t_a = trajectory_angle(spacePlanet);
				double d_a = normalize(t_a - angle, PI2);
				if (d_a > PI) d_a -= PI2;
				//                                double p_a = normalize(atan3(1.9*spacePlanet->getSprite()->width()/2.0, r), PI2);
				//                                p_a = p_a - fabs(d_a);
				//                                if (p_a > 0) {
				if (fabs(d_a)<PI/2) {
					if (d_a > 0)
						angle = normalize(t_a - PI/2, PI2);
					else
						angle = normalize(t_a + PI/2, PI2);
				}
			}
		}

		accelerate_gravwhip(this, angle, accel_rate * frame_time, speed_max);
		if (hotspot_frame <= 0) {
			add(new Animation(this,
				normal_pos() - product(unit_vector(angle), get_size()) / 2.0,
			//                  normal_x() - (cos(angle) * w / 2.5),
			//			        normal_y() - (sin(angle) * h / 2.5),
				meleedata.hotspotSprite,
				0, HOTSPOT_FRAMES, 50, DEPTH_HOTSPOTS));
			hotspot_frame += hotspot_rate;
		}
		if (hotspot_frame > 0) hotspot_frame-= frame_time;
	}

	SpaceObject::calculate();
}


void OrzMarine::animate(Frame *space)
{
	STACKTRACE;
	if (!invading) SpaceObject::animate(space);
}


void OrzMarine::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other->isShip() && (!returning) && (!other->sameTeam(this)) && (!other->isProtected())) {
		invading = (Ship *) other;

		if (invading->damage_factor < armour) {
			collide_flag_anyone = 0;
			play_sound(data->sampleExtra[1]);
			damage(invading, 0, 1);
			if (invading->spritePanel) {
				sprite->draw(14 + ((slot % 4) * 6), 16 + ((slot / 4) * 6), 0, invading->spritePanel->get_bitmap(0) );
				/*draw_sprite(invading->spritePanel->get_bitmap(0), sprite->get_bitmap(0),
				14 + ((slot % 4) * 6), 16 + ((slot / 4) * 6));*/
				invading->update_panel = TRUE;
			}
		}
	}
	if ((ship) && (other == ship) && (returning)) {
		state = 0;
		damage(ship, 0, -1);
	}
}


int OrzMarine::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	int total = iround(normal + direct);
	armour -= total;
	if (armour <= 0)
		state = 0;
	return total;
}


REGISTER_SHIP(OrzNemesis)
