/* $Id: shpestgu.cpp,v 1.13 2004/03/24 23:51:41 yurand Exp $ */
#include "../ship.h"
REGISTER_FILE
#include <string.h>
#include "../melee/mview.h"

// platform relaying blink length [cyhawk]
#define ESTION_PLATFORM_BLINK 100

class EstionPlatform;
class EstionGunner : public Ship
{
	double weaponRange;
	double weaponVelocity;
	int    weaponDamage;
	int    weaponArmour;
	// max shot bump correction angle [cyhawk]
	double weaponAngle;
	// weapon cycle variable [cyhawk]
	int    weaponGraphics;

	int    specialArmour;
	int    specialFrameSize;

	public:
		EstionGunner(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		int num_platforms;
		int max_platforms;
		EstionPlatform **platform;

		virtual int activate_weapon();
		virtual int activate_special();
};

class EstionShot : public Shot
{
	double relayAngle;

	public:
		EstionGunner *mother_ship;
		EstionShot(Vector2 opos, double angle, double velocity, int damage,
			double range, int armour, EstionGunner *ship, int osprite_index, double oRelayAngle);
		SpaceLocation *last_shooter;
		virtual void inflict_damage(SpaceObject *other) ;
};

class EstionPlatform : public SpaceObject
{
	int health;
	int rotate_time;
	int rotate_direction;

	SpaceSprite* explosionSprite;
	int          explosionFrameCount;
	int          explosionFrameSize;
	SAMPLE*      explosionSample;
	public:
		// blink support [cyhawk]
		int blink;

		EstionPlatform (EstionGunner *ship, int health, int oFrameSize);
		virtual int handle_damage (SpaceLocation *source, double normal, double direct) ;
		virtual void ship_died();
		virtual void calculate();
		virtual void death();
};

int EstionPlatform::handle_damage (SpaceLocation *source, double normal, double direct)
{
	health -= iround(normal + direct * 2);
	if (health < 0) {
		state = 0;
		// display explosion graphics [cyhawk]
		game->add(new Animation(this, normal_pos(),
			explosionSprite, 0, explosionFrameCount,
			explosionFrameSize, LAYER_EXPLOSIONS));
		// play explosion sound [cyhawk]
		play_sound( explosionSample );
	}
	return iround(normal + 2 * direct);
}


void EstionPlatform::ship_died()
{
	STACKTRACE;
	// display explosion graphics [cyhawk]
	game->add(new Animation(this, normal_pos(),
		explosionSprite, 0, explosionFrameCount,
		explosionFrameSize, LAYER_EXPLOSIONS));
	state = 0;
}


void EstionPlatform::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();
	rotate_time -= frame_time;
	while (rotate_time < 0) {
		rotate_time += 50;
		// 64 frames for rotation [cyhawk]
		sprite_index = (sprite_index + rotate_direction) & 63;
	}

	// blink support [cyhawk]
	if (blink > 0) {
		sprite = data->spriteExtra;
		blink -= frame_time;
	}
	else
		sprite = data->spriteSpecial;

	vel *= exp(-0.0005*frame_time);
	/*	if (magnitude(vel) < 0.05)
			vel = 0;*/
	return;
}


void EstionPlatform::death()
{
	STACKTRACE;
								 //ship is already dead; don't remove platform from list
	if (!(ship && ship->exists())) return;
								 //ship is somehow not an Estion; perhaps we've been stolen
	if (ship->data != data) return;
	for (int i = 0; i < ((EstionGunner*)ship)->num_platforms; i += 1) {
		if (((EstionGunner*)ship)->platform[i] == this) {
			((EstionGunner*)ship)->platform[i] = NULL;
			((EstionGunner*)ship)->num_platforms -= 1;
			memmove(&((EstionGunner*)ship)->platform[i], &((EstionGunner*)ship)->platform[i+1],
				(((EstionGunner*)ship)->num_platforms-i) * sizeof(EstionPlatform*));
			return;
		}
	}
	return;
}


EstionPlatform::EstionPlatform (EstionGunner *ship, int health, int oFrameSize) :
SpaceObject(ship, ship->normal_pos(), ship->get_angle(),
ship->data->spriteSpecial)
{
	layer = LAYER_SPECIAL;
	mass = 3;
	collide_flag_sameship = bit(LAYER_SHOTS) | bit(LAYER_SPECIAL);
	this->health = health;
	this->ship = ship;
	vel = ship->get_vel() + 0.3 * unit_vector(angle);
	rotate_time = 0;
	// custom explosion graphics added [cyhawk]
	explosionSprite     = data->spriteSpecialExplosion;
	explosionFrameCount = data->spriteSpecialExplosion->frames();
	explosionFrameSize  = oFrameSize;
	//explosionSample     = data->sampleSpecial[1]; commented out by orz ; seems to be a bug?
	explosionSample = (SAMPLE *)(melee[MELEE_BOOM + BOOM_SAMPLES - 1].dat);
	// blinking support initialization [cyhawk]
	blink = 0;

	if (random() & 1) rotate_direction = 1; else rotate_direction = -1;

	isblockingweapons = false;
}


EstionGunner::EstionGunner(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	// modified check [cyhawk]
	if (!data->spriteSpecial || (data->spriteSpecial->frames() != 64))
		tw_error("you have the wrong\n version of shpestgu.dat");

	weaponDamage = get_config_int("Weapon", "Damage", 0);
	weaponArmour = get_config_int("Weapon", "Armour", 0);
	weaponRange  = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity  = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	// max shot bump correction angle [cyhawk]
	weaponAngle  = get_config_float("Weapon", "Angle", 0);
	// weapon cycle initialization [cyhawk]
	weaponGraphics = 0;

	specialArmour = get_config_int( "Special", "Armour", 0);
	specialFrameSize = (int)(time_ratio/get_config_float( "Special", "ExplosionSpeed", 1));
	max_platforms = get_config_int( "Special", "Number", 0);
	num_platforms = 0;
	platform = new EstionPlatform*[max_platforms];
	for (int i = 0; i < max_platforms; i += 1) {
		platform[i] = NULL;
	}
}


EstionShot::EstionShot(Vector2 opos, double angle, double velocity,
int damage, double range, int armour, EstionGunner *ship, int osprite_index, double oRelayAngle) :
Shot(ship, opos, angle, velocity, damage, range, armour, ship,
ship->data->spriteWeapon, 0)
{
	STACKTRACE;
	collide_flag_sameship = bit(LAYER_SPECIAL);
	last_shooter = ship;
	mother_ship = ship;
	// custom explosion graphics added [cyhawk]
	explosionSprite = data->spriteWeaponExplosion;
	explosionFrameCount = data->spriteWeaponExplosion->frames();
	explosionFrameSize = time_ratio;
	// store bump correction angle
	relayAngle = oRelayAngle;
	sprite_index = osprite_index;
}


void EstionShot::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other == last_shooter) return;
	if (!sameShip(other) || other == mother_ship) {
		// don't explode on contact, just damage [cyhawk]
		Shot::inflict_damage(other);
		/*Removed on request of MRT - makes the ship too powerful as a camper
		SpaceObject::inflict_damage(other);
		// and get bumped back
		last_shooter = other;
		double v = magnitude_sqr(vel);
		double alpha = normalize (atan(vel) , PI2);
		double min_dist = 999999;
		int k = -1;
		for (int i = 0; i < mother_ship->num_platforms; i += 1) {
			double delta = trajectory_angle (mother_ship->platform[i]);
			if (normalize (fabs(delta - alpha), PI2) < relayAngle)
				{
				double r = distance(mother_ship->platform[i]);
				if (r < min_dist || k == -1)
					{
					min_dist = r;
					k = i;
					}
				}
			}
		if (k == -1)
			{
			double delta = trajectory_angle (other);
			vel = v * unit_vector( PI + 2 * delta - alpha );
			} else {
			double delta = trajectory_angle (mother_ship->platform[k]);
			vel = v * unit_vector(delta);
			}
			*/
		return;
	}
	if (!target || target->isInvisible()) return;
	last_shooter = other;
	d = 0.0;
	SpaceLocation *tmp = ship->target;
	if (!tmp) return;
	// we're being relayed -- play sound [cyhawk]
	play_sound( data->sampleExtra[random() % data->num_extra_samples] );
	double mr = distance(ship->target);
	for (int i = 0; i < mother_ship->num_platforms; i += 1) {
		double r = mother_ship->platform[i]->distance(ship->target);
		if ((mother_ship->platform[i] != other) && (r < mr) && (distance(mother_ship->platform[i]) < range)) {
			tmp = mother_ship->platform[i];
			mr = r;
		}
		// added blink effect [cyhawk]
		else if (mother_ship->platform[i] == other) {
			mother_ship->platform[i]->blink = ESTION_PLATFORM_BLINK;
		}
	}
	double rr;
	Vector2 tt = min_delta(pos, tmp->normal_pos());
	rr = magnitude(tt) / v;
	tt -= tmp->get_vel() * rr;
	changeDirection( PI+atan(tt));
	return;
}


int EstionGunner::activate_weapon()
{
	STACKTRACE;
	// cycle weapon graphics [cyhawk]
	weaponGraphics++;
	if (weaponGraphics == data->spriteWeapon->frames()) {
		weaponGraphics = -data->spriteWeapon->frames() + 2;
	}
	Shot *shot = new EstionShot(Vector2(0.0, (size.y / 4.0)),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, abs(weaponGraphics), weaponAngle);
	game->add(shot);
	// random firing sound [cyhawk]
	weapon_sample = random() % data->num_weapon_samples;
	return(TRUE);
}


int EstionGunner::activate_special()
{
	STACKTRACE;
	if (num_platforms == max_platforms) {
		num_platforms -= 1;
		platform[0]->state = 0;
		memcpy(&platform[0], &platform[1], sizeof(EstionPlatform*) * num_platforms);
		platform[num_platforms] = NULL;
	}
	EstionPlatform *tmp = new EstionPlatform(this, specialArmour, specialFrameSize);
	platform[num_platforms] = tmp;
	num_platforms += 1;
	game->add( tmp );
	return(TRUE);
}


REGISTER_SHIP(EstionGunner)
