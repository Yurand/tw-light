/* $Id: shptaugl.cpp,v 1.12 2007/04/16 23:55:32 yurand Exp $ */
#include "../ship.h"

REGISTER_FILE

class TauGladius : public Ship
{
	public:
		double      weaponVelocity, weaponRange, weaponDamage, weaponSpread;
		bool        flashed;
		int         flash_counter;

		double      specialRange;
		double      specialVelocity, specialTurnRate;
		double      specialProxyAngle, specialDamage, specialArmour;

		int         side;

	public:
		TauGladius(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

		virtual void calculate();
		virtual int  activate_weapon();
		virtual int  activate_special();
		virtual void animate(Frame *space);
};

class TauGladiusMissile : public HomingMissile
{
	public:
		double  proxy_angle;
		int     smoke_frame;

	public:

		TauGladiusMissile(SpaceLocation *creator, Vector2 opos, double oangle,
			double ov, double orange, double odamage, double otrate,
			double oproxyangle, double oarmour,
			SpaceSprite *osprite, SAMPLE *s, SpaceObject *otarget);
		virtual void calculate();
};

class TauGladiusShot : public SpaceLine
{
	public:
		double    d, v, range;
		int       r,g,b;

	public:
		TauGladiusShot (SpaceLocation *creator, Vector2 opos, double oangle, double ov, double orange, double odamage, double olength);
		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual int  handle_damage(SpaceLocation *source, double normal, double direct = 0);
		virtual void animate(Frame *space);
};

TauGladius::TauGladius(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange         = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity      = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage        = tw_get_config_float("Weapon", "Damage", 0);
	weaponSpread        = tw_get_config_float("Weapon", "Spread", 0) * ANGLE_RATIO;

	specialRange        = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity     = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialTurnRate     = scale_turning(tw_get_config_float("Special", "TurnRate", 0));
	specialDamage       = tw_get_config_float("Special", "Damage", 0);
	specialArmour       = tw_get_config_float("Special", "Armour", 0);
	specialProxyAngle   = tw_get_config_float("Special", "TrackAngle", 0) * ANGLE_RATIO;

	side = 1;

	special_sample = -1;
	flashed = false;
	flash_counter = 0;
}


void TauGladius::calculate()
{
	STACKTRACE;
	Ship::calculate();
	if (flash_counter > 0)
		flash_counter -= frame_time;
}


int TauGladius::activate_weapon()
{
	STACKTRACE;
	double s = tw_random(-1.0, 1);
	s *= fabs(s) * weaponSpread;
	flash_counter = 25;
	flashed = false;
	game->add(new TauGladiusShot(this, Vector2(5,23), angle + s, weaponVelocity,
		weaponRange, weaponDamage, weaponVelocity*50));
	return true;
}


int TauGladius::activate_special()
{
	STACKTRACE;
	game->add(new TauGladiusMissile (this, Vector2(13*side,0), angle,
		specialVelocity, specialRange, specialDamage,
		specialTurnRate, specialProxyAngle, specialArmour,
		data->spriteSpecial, data->sampleSpecial[0], target));
	side *= -1;
	return true;
}


void TauGladius::animate(Frame *space)
{
	STACKTRACE;
	if ((flash_counter > 0) || !flashed) {
		flashed = true;
		data->more_sprites[0]->animate(pos, sprite_index, space);
	}
	else
		sprite->animate(pos, sprite_index, space);
}


TauGladiusMissile::TauGladiusMissile(SpaceLocation *creator, Vector2 opos, double oangle,
double ov, double orange, double odamage, double otrate,
double oproxyangle, double oarmour,
SpaceSprite *osprite, SAMPLE *s, SpaceObject *otarget) :
HomingMissile(creator, opos, oangle, ov, odamage, orange, oarmour,
otrate, creator,  osprite, otarget),
proxy_angle(oproxyangle)

{
	STACKTRACE;
	explosionSprite = data->spriteSpecialExplosion;
	explosionFrameCount = 10;
	explosionFrameSize = 50;
	explosionSample = data->sampleSpecial[1];
	smoke_frame = 0;
	play_sound(s);
}


void TauGladiusMissile::calculate()
{
	STACKTRACE;
	if (smoke_frame > 0) smoke_frame -= frame_time;
	else {
		while (smoke_frame <= 0)
			smoke_frame += 25;
		game->add(new Animation(this, pos, data->spriteExtra,
			0, 20, 50, LAYER_HOTSPOTS));
	}

	if (target) {
		if (target->exists() && (!target->isInvisible())) {
			double d_a = normalize(trajectory_angle(target) - (angle + turn_step), PI2);
			if (d_a > PI) d_a -= PI2;
			if (fabs(d_a) > proxy_angle)
				turn_rate = 0;
		}
		else
			target = NULL;
	}
	else
		target = NULL;

	HomingMissile::calculate();

}


TauGladiusShot::TauGladiusShot (SpaceLocation *creator, Vector2 opos, double oangle, double ov, double orange, double odamage, double olength) :
SpaceLine(creator, creator->normal_pos(), oangle, olength, tw_makecol(255,255,115)),
d(0), v(ov), range(orange)
{
	damage_factor = odamage;
	set_depth(DEPTH_SHOTS);
	opos.x *= -1;
	pos = normalize(pos + rotate(opos, -PI/2+creator->get_angle()));
	vel = (v * unit_vector(angle)) + creator->get_vel() * game->shot_relativity;
	if (range < 0) range = 99999999999999.0;
	r = 255;
	g = 255;
	b = 115;

}


void TauGladiusShot::calculate()
{
	STACKTRACE;
	SpaceLine::calculate();
	double rr = (d) / range;
	r = 255 - int(rr*150);
	g = (int)floor(255 - rr*250);
	if (g < 0) g = 0;
	b = (int)floor(115 - rr*150);
	if (b < 0) b = 0;
	d += v * frame_time;
	if (d > range)
		state = 0;
}


void TauGladiusShot::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	damage(other, damage_factor, 0);
	state = 0;
	int i = iround_down(damage_factor / 2);
	if (i >= BOOM_SAMPLES) i = BOOM_SAMPLES - 1;
	play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	game->add(new Animation( this,
		pos + edge(), meleedata.sparkSprite, 0,
		SPARK_FRAMES, 50, DEPTH_EXPLOSIONS));
}


int TauGladiusShot::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if ((normal > 0) || (direct > 0))
		state = 0;
	return 1;
}


void TauGladiusShot::animate(Frame *space)
{
	STACKTRACE;
	Vector2 position_base = pos;
	double length_base = length;
	double ic;
	pos += edge()/2 + unit_vector(angle)*10;
	Vector2 tl = unit_vector(angle)*5;
	for (int i=-3; i<=0; i++) {
		ic = (1-i)/4.0;
		length = ic * 20;
		ic *= ic*ic;
		ic = sqrt(ic);
		pos -= tl;
		color = tw_makecol(iround(r*ic), iround(g*ic), iround(b*ic));
		SpaceLine::animate(space);
	}
	pos =  position_base;
	length = length_base;
}


REGISTER_SHIP(TauGladius)
