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

#define BOGEI_CENTURION_ID SPACE_SHIP+0x00FE
#define max_links 20

class BoggCenturion  :  public Ship
{
	double        weaponRange, weaponSpread;
	int           weaponDamage;

	int           specialDamage, specialArmour;
	int           specialLifetime, specialFuel;
	double        specialAccel, specialMaxspeed, specialBlastMaxspeed;
	int           specialHotspotRate, specialHotspotFrameSize;
	double        specialHotspotThrust, specialHotspotSlowdown;
	int           special_slot;
	double        specialBlastAccel;

	int           gun_phase, old_gun_phase;
	double        gun_position, gun_speed;
	int           startup_time, startup_delay, slowdown_time, delay_count;
	bool          slowing_down, gun_full_speed;
	int           flame_frame, flame_count, flame_duration;

	int           exhaust_frame, exhaust_rate, exhaust_framesize, exhaust_count;
	int           exhaust_fry_chance;
	double        exhaust_slowdown, exhaust_thrust;
	bool          draw_hotspots, exhaust_on;

	double        share_range;
	int           links_num;

	double        residual_damage;

	BoggCenturion        *link[max_links+1];

	public:

		BoggCenturion      (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

		virtual  int  activate_weapon();
		virtual void  calculate_fire_special();
		virtual void  calculate();
		virtual void  animate(Frame *space);
		virtual void  calculate_hotspots();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		//virtual void  handle_damage(SpaceLocation *source);
};

class BoggCenturionMissile : public Missile
{
	int lifetime, fuel;
	double accel, maxspeed, blast_accel, blast_maxspeed;
	int hotspot_rate, hotspot_frame_size, hotspot_frame;
	double hotspot_thrust, hotspot_slowdown;
	SpaceSprite *hotspot_sprite;

	public:

		BoggCenturionMissile (SpaceLocation *creator, Vector2 opos, double oangle, int odamage, int oarmour,
			int olifetime, int ofuel, double oaccel, double omaxspeed, double oblast_accel, double oblast_maxspeed,
			int ohotspot_rate, int ohotspot_frame_size, double ohotspot_thrust, double ohotspot_slowdown,
			SpaceSprite *osprite, SpaceSprite *hsprite, SAMPLE *s, SpaceSprite *esprite);
		virtual void  calculate();
		virtual void  inflict_damage(SpaceObject *other);
};

class BoggCenturionExhaust : public Animation
{
	double slowdown;
	public:
		BoggCenturionExhaust (SpaceLocation *creator, double oangle, double dx, double dy, SpaceSprite *osprite,
			int first_frame, int num_frames, int frame_size, double depth,
			double v, double oslowdown, int ochance, SAMPLE *os);
		virtual void  calculate();
};

class BoggCenturionExhaustShot : public Shot
{
	SpaceLocation *amt;
	SAMPLE *s;
	public:
		BoggCenturionExhaustShot (SpaceLocation *creator, SpaceSprite *osprite, SAMPLE *os);
		virtual void  calculate();
		virtual void  inflict_damage(SpaceObject *other);
		virtual void  animate(Frame *space);
};

class BoggCenturionShot : public Laser
{
	SpaceSprite *ex_sprite;
	SAMPLE *ex_sample;

	public:
		BoggCenturionShot (SpaceLocation *creator, Vector2 opos, double oangle, int odamage,
			double orange, SpaceSprite *esprite, SAMPLE *esample);
		virtual void animate(Frame *space);
		virtual void inflict_damage(SpaceObject *other);
		virtual void calculate();
};

BoggCenturion::BoggCenturion (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	id                      = BOGEI_CENTURION_ID;

	exhaust_on              = (tw_get_config_int("Engines", "ExhaustOn", 0) != 0);
	exhaust_rate            = tw_get_config_int("Engines", "ExhaustRate", 0);
	exhaust_frame           = 0;
	exhaust_slowdown        = tw_get_config_float("Engines", "ExhaustSlowdown", 0) / 1000.0;
	exhaust_framesize       = tw_get_config_int("Engines", "ExhaustFrameSize", 0);
	exhaust_thrust          = scale_velocity(tw_get_config_float("Engines", "ExhaustThrust", 0));
	draw_hotspots           = (tw_get_config_int("Engines", "DrawHotspots", 0) != 0);
	exhaust_fry_chance      = tw_get_config_int("Engines", "ExhaustFryChance", 0);

	share_range             = scale_range(tw_get_config_float("Herd", "Range", 0));
	links_num               = 0;

	weaponRange             = scale_range(tw_get_config_float("Primary", "Range", 0));
	weaponDamage            = tw_get_config_int("Primary", "Damage", 0);
	weaponSpread            = tw_get_config_float("Primary", "Spread", 0) * ANGLE_RATIO;
	flame_duration          = tw_get_config_int("Primary", "FlameDuration", 0);

	startup_time            = int(tw_get_config_float("Primary", "StartupTime", 0) * 1000);
	slowdown_time           = int(tw_get_config_float("Primary", "SlowdownTime", 0) * 1000);
	startup_delay           = int(tw_get_config_float("Primary", "StartupDelay", 0) * 1000);

	specialDamage           = tw_get_config_int("Secondary", "Damage", 0);
	specialArmour           = tw_get_config_int("Secondary", "Armour", 0);
	specialLifetime         = int(tw_get_config_float("Secondary", "Lifetime", 0) * 1000);
	specialFuel             = int(tw_get_config_float("Secondary", "Fuel", 0) * 1000);
	specialAccel            = scale_acceleration(tw_get_config_float("Secondary", "Accel", 0),0);
	specialBlastAccel       = scale_velocity(tw_get_config_float("Secondary", "BlastAccel", 0));
	specialBlastMaxspeed    = scale_velocity(tw_get_config_float("Secondary", "BlastMaxspeed", 0));
	specialMaxspeed         = scale_velocity(tw_get_config_float("Secondary", "Maxspeed", 0));
	specialHotspotRate      = tw_get_config_int("Secondary", "HotspotRate", 0);
	specialHotspotFrameSize = tw_get_config_int("Secondary", "HotspotFrameSize", 0);
	specialHotspotThrust    = scale_velocity(tw_get_config_float("Secondary", "HotspotThrust", 0));
	specialHotspotSlowdown  = tw_get_config_float("Secondary", "HotspotSlowdown", 0) / 1000.0;

	special_slot            = 2;

	gun_phase = 0; old_gun_phase = 0;
	gun_position = 0;
	gun_speed = 0;
	slowing_down = false;
	gun_full_speed = false;
	flame_count = -1;
	delay_count = 0;

	residual_damage = 0;
}


int BoggCenturion::activate_weapon()
{
	STACKTRACE;
	if ((gun_phase >= old_gun_phase) || (!gun_full_speed) || (delay_count < startup_delay))
		return false;

	double r = 1 - random(2.0);
	//        r *= r;
	//        double l = sqrt(1 - r*r) * (1 - (random()%101)/50.0);
	//        l *= l * sin(weaponSpread*ANGLE_RATIO) * weaponRange;

	double l = sqrt(random(1.0));
	game->add(new BoggCenturionShot(this, Vector2(+8 + 8*r, 21), angle + r*weaponSpread, weaponDamage, weaponRange * l,
		data->spriteWeapon, data->sampleWeapon[1]));

	flame_frame = random(4);
	flame_count = flame_duration;

	return true;
}


void BoggCenturion::calculate_fire_special()
{
	STACKTRACE;

	special_low = FALSE;

	if (fire_special) {
		if (batt < special_drain) {
			special_low = TRUE;
			return;
		}
	}
	else return;

	if (special_recharge > 0) return;

	double dx, dy;
	dx = (19 + 6.3 * special_slot);
	dy = 2 * special_slot;

	game->add(new BoggCenturionMissile(this, Vector2(dx, dy), angle,
		specialDamage, specialArmour, specialLifetime, specialFuel,
		specialAccel, specialMaxspeed, specialBlastAccel, specialBlastMaxspeed,
		specialHotspotRate, specialHotspotFrameSize, specialHotspotThrust, specialHotspotSlowdown,
		data->spriteSpecial, data->spriteExtra, data->sampleSpecial[0],
		data->spriteSpecialExplosion));
	game->add(new BoggCenturionMissile(this, Vector2(-dx, dy), angle,
		specialDamage, specialArmour, specialLifetime, specialFuel,
		specialAccel, specialMaxspeed, specialBlastAccel, specialBlastMaxspeed,
		specialHotspotRate, specialHotspotFrameSize, specialHotspotThrust, specialHotspotSlowdown,
		data->spriteSpecial, data->spriteExtra, data->sampleSpecial[0],
		data->spriteSpecialExplosion));

	special_slot--;
	if (special_slot < 0)
		special_slot = 2;

	batt -= special_drain;
	special_recharge += special_rate;
}


void BoggCenturion::calculate()
{
	STACKTRACE;
	Ship::calculate();

	if ((fire_weapon) && !( (batt < weapon_drain) || slowing_down)  ) {
		if (startup_time > 0)
			gun_speed += frame_time / (double)startup_time;
		else    gun_speed = 1.0;
	} else {
		slowing_down = true;
		gun_full_speed = false;
		delay_count = 0;
		if (slowdown_time > 0)
			gun_speed -= frame_time / (double)slowdown_time;
		else    gun_speed = 0;
	}
	if (gun_speed >= 1.0) {
		gun_full_speed = true;
		gun_speed = 1.0;
		if (delay_count < startup_delay) delay_count += frame_time;
	}
	else
	if ((gun_speed <= 0) && (slowing_down)) {
		gun_speed = 0;
		slowing_down = false;
	}
	gun_position += frame_time*gun_speed/weapon_rate;
	while (gun_position >= 1) gun_position -= 1;

	old_gun_phase = gun_phase;
	gun_phase = (int)floor(gun_position * 8);

	flame_count -= frame_time;

	Query q;
	links_num = 0;
	BoggCenturion* bro;
	for (q.begin(this, bit(LAYER_SHIPS), share_range); q.currento; q.next())
	if (q.currento->getID() == BOGEI_CENTURION_ID) {
		bro = (BoggCenturion*)q.currento;
		if (bro->exists())
			links_num += 1;
		if (links_num == 2) break;
	}

}


void BoggCenturion::animate(Frame* space)
{
	STACKTRACE;
	Surface *bmp;
	bmp = sprite->get_bitmap(64, 0);
	tw_clear_to_color( bmp, tw_makecol(255,0,255));

	double tx, ty;
	int ix, iy;

	//      prepare for the gun
	tx = sin((sprite_index+5) * 2 * PI / 64.0);
	ty = cos((sprite_index+5) * 2 * PI / 64.0);
	int ix1 = 42 + int(19*tx) - 12;
	int iy1 = 42 - int(19*ty) - 12;

	tw_blit(data->spriteSpecial->get_bitmap(64), bmp, 0, 0, ix1, iy1, 20, 20);

	//      flame

	if (flame_count >= 0) {

		tx = sin((sprite_index+3) * 2 * PI / 64.0);
		ty = cos((sprite_index+3) * 2 * PI / 64.0);
		ix = 42 + int(35*tx) - 10;
		iy = 42 - int(35*ty) - 10;
		data->spriteExtraExplosion->draw(ix, iy, 40 + sprite_index + 64*flame_frame, bmp);
	}

	//      ship itself
	sprite->draw(0, 0, sprite_index, bmp);

	//      gun

	data->spriteWeaponExplosion->draw(ix1, iy1, sprite_index+64*gun_phase, bmp);

	///     shield link indicator
	tx = sin(sprite_index * 2 * PI / 64.0);
	ty = cos(sprite_index * 2 * PI / 64.0);
	ix = 42 + int(-9*tx) - 10;
	iy = 42 - int(-9*ty) - 10;

	if (links_num)
		data->spriteExtraExplosion->draw(ix, iy, 296+sprite_index+64*(links_num-1), bmp);

	//      final
	//sprite->animate(pos, 64, space);
	animate_bmp(bmp, pos, space);
}


void BoggCenturion::calculate_hotspots()
{
	STACKTRACE;
	if (draw_hotspots)
		Ship::calculate_hotspots();

	if (exhaust_frame > 0) exhaust_frame -= frame_time;
	else    if (thrust) {
		exhaust_frame += exhaust_rate;
		if (exhaust_count < 7) exhaust_count += 1;
		game->add(new BoggCenturionExhaust(this, angle,
			-8.5, -36, data->spriteExtraExplosion, 10*(random(4)) + 10 - exhaust_count,
			exhaust_count, exhaust_framesize, LAYER_HOTSPOTS,
			exhaust_thrust, exhaust_slowdown,
			exhaust_fry_chance, data->sampleExtra[0]));
		game->add(new BoggCenturionExhaust(this, angle,
			+9.5, -36, data->spriteExtraExplosion, 10*(random(4)) + 10 - exhaust_count,
			exhaust_count, exhaust_framesize, LAYER_HOTSPOTS,
			exhaust_thrust, exhaust_slowdown,
			exhaust_fry_chance, data->sampleExtra[0]));
	}

	if (!thrust)
		exhaust_count = 0;
}


int BoggCenturion::handle_damage(SpaceLocation *source, double normal, double direct)
//void BoggCenturion::handle_damage(SpaceLocation *source)
{
	STACKTRACE;
	if (source == this) {
		return Ship::handle_damage(source, normal, direct);
	}

	double tot = normal + direct;

	Query q;
	int ln = 0;
	BoggCenturion* bro;
	for (q.begin(this, bit(LAYER_SHIPS), share_range); q.currento; q.next())
	if (q.currento->getID() == BOGEI_CENTURION_ID) {
		bro = (BoggCenturion*)q.currento;
		if (bro->exists()) {
			ln += 1;
			link[ln] = bro;
		}
		if (links_num == max_links) break;
	}

	if (ln) {
		double d = tot;

		if (ln < 2)
			d *= 0.75;
		else
			d *= 0.5;

		tot = (int)floor(d);

		d -= tot;
		residual_damage += d;

		int dx = (int)floor(residual_damage);
		tot += dx;

		residual_damage -= dx;

	}

	Ship::handle_damage(source, tot);

	return 0;
}


BoggCenturionMissile::BoggCenturionMissile (SpaceLocation *creator, Vector2 opos, double oangle, int odamage, int oarmour,
int olifetime, int ofuel, double oaccel, double omaxspeed, double oblast_accel, double oblast_maxspeed,
int ohotspot_rate, int ohotspot_frame_size, double ohotspot_thrust, double ohotspot_slowdown,
SpaceSprite *osprite, SpaceSprite *hsprite, SAMPLE *s, SpaceSprite *esprite) :
Missile (creator, opos, oangle, 0, odamage, -1, oarmour, creator, osprite, 1.0),
lifetime(olifetime), fuel(ofuel), accel(oaccel), maxspeed(omaxspeed),
hotspot_rate(ohotspot_rate), hotspot_frame_size(ohotspot_frame_size),
hotspot_thrust(ohotspot_thrust), hotspot_slowdown(ohotspot_slowdown),
hotspot_sprite(hsprite)
{
	blast_accel = oblast_accel;
	blast_maxspeed = oblast_maxspeed;
	hotspot_frame = 0;
	play_sound(s, 128);
	explosionSprite = esprite;
	explosionFrameCount = 10;
	explosionFrameSize = 50;
	explosionSample = data->sampleSpecial[1];
}


void BoggCenturionMissile::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();
	if (lifetime > 0) lifetime -= frame_time;
	else    state = 0;

	if (fuel > 0) {
		fuel -= frame_time;
		accelerate(this, angle, accel*frame_time, maxspeed);
		if (hotspot_frame > 0) hotspot_frame -= frame_time;
		else {
			hotspot_frame += hotspot_rate;
			game->add(new BoggCenturionExhaust(this, angle, 0, -8.5, hotspot_sprite, 0, 20,
				hotspot_frame_size, LAYER_HOTSPOTS,
				hotspot_thrust, hotspot_slowdown, 0, NULL));
		}
	}

	//sprite_index = (int(angle / 5.625 + 16)) & 63;
	sprite_index = get_index(angle);
};

void  BoggCenturionMissile::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if ((!other->isPlanet()) && (other->mass))
		other->accelerate(this, normalize(trajectory_angle(other), 360), blast_accel/other->mass, blast_maxspeed);
	Missile::inflict_damage(other);
}


BoggCenturionExhaust::BoggCenturionExhaust (SpaceLocation *creator, double oangle, double dx, double dy, SpaceSprite *osprite,
int first_frame, int num_frames, int frame_size, double depth,
double v, double oslowdown, int ochance, SAMPLE *os) :
Animation (creator, creator->normal_pos(), osprite,
first_frame, num_frames, frame_size, depth)
{
	if (random(100) < ochance)
		game->add(new BoggCenturionExhaustShot(this, osprite, os));

	//double alpha = oangle * ANGLE_RATIO;
	//double tx = cos(alpha);
	//double ty = sin(alpha);
	//x += dy * tx - dx*ty;
	//y += dy * ty + dx*tx;
	pos += rotate(Vector2(dy,dx), oangle);
	//vx = creator->get_vx() - vel * tx;
	//vy = creator->get_vy() - vel * ty;
	vel = creator->get_vel() - v * unit_vector(oangle);
	slowdown = oslowdown;
}


void BoggCenturionExhaust::calculate()
{
	STACKTRACE;
	Animation::calculate();
	double gamma = exp( - slowdown * frame_time);
	vel *= gamma;
	//vy *= gamma;
}


BoggCenturionExhaustShot::BoggCenturionExhaustShot (SpaceLocation *creator, SpaceSprite *osprite, SAMPLE *os) :
Shot(creator, 0, 0, 0, 1, 1e40, 1, creator, osprite, 1.0)
{
	amt = creator;
	s = os;
	sprite_index = 5;
}


void BoggCenturionExhaustShot::calculate()
{
	STACKTRACE;
	Shot::calculate();
	if (!amt->exists()) state = 0;
	//x = amt->normal_x();
	//y = amt->normal_y();
	pos = amt->normal_pos();
	//vx = amt->get_vx();
	//vy = amt->get_vy();
	vel = amt->get_vel();
}


void BoggCenturionExhaustShot::animate(Frame *space)
{
	STACKTRACE;
}


void BoggCenturionExhaustShot::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	//other->damage += 1;
	other->handle_damage(this, 1);
	if (s)  play_sound(s, 160);
	state = 0;
}


BoggCenturionShot::BoggCenturionShot (SpaceLocation *creator, Vector2 opos, double oangle, int odamage,
double orange, SpaceSprite *esprite, SAMPLE *esample) :
Laser(creator, oangle, 0, orange, odamage, 300, creator, opos, true)
{
	ex_sprite = esprite; ex_sample = esample;
}


void BoggCenturionShot::animate(Frame *space)
{
	STACKTRACE;
	// for testing show the lasers
	//color = tw_makecol(255,255,255); Laser::animate(space);
}


void BoggCenturionShot::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	//x += edge_x(); y += edge_y();
	pos += edge();
	//        play_sound(ex_sample, 240);
	game->add(new Animation(this, pos, ex_sprite, 0, 10, 50, LAYER_EXPLOSIONS));
	//other->damage += damage_factor;
	other->handle_damage(this, damage_factor);
	state = 0;
}


void BoggCenturionShot::calculate()
{
	STACKTRACE;
	if (frame > 0) {
		state = 0;
		return;
	}
	Laser::calculate();
}


REGISTER_SHIP(BoggCenturion)
