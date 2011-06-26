/* $Id: shpsamat.cpp,v 1.22 2005/08/28 20:34:08 geomannl Exp $ */
#include "../ship.h"

/*

Sa-Matra:

Main = green blobs that deal 1 damage and push the target away.
Special = comet that does time-damage

*/

REGISTER_FILE

class SaMatra : public Ship
{
	public:

		Vector2 startpos;

		int weaponNumber, Nweapons;
		int specialNumber, Nspecials;
		double armour, Armour;

		double weaponVelocity, weaponDamage, weaponArmour, weaponTurnPerSec;
		double specialVelocity, specialDamage, specialArmour, specialTrailTimeLen,
			specialTurnPerSec, specialDamageDelay;

		SaMatra(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual void relocate();
		virtual void death();

		virtual void calculate();
		virtual int activate_weapon();
		virtual int activate_special();

		virtual int handle_damage(SpaceLocation *src, double normal, double direct=0);
		virtual void inflict_damage(SpaceObject *other);

		virtual SpaceLocation *get_ship_phaser();
};

class SaMatraFlame : public SpaceObject
{
	public:
		double armour;
		SaMatra *samatra;

		double trailTime;		 // in seconds
		int trailNum;
		Vector2 *trailPos;

		double damage_delay, damage_delay_max;

	public:

		SaMatraFlame(SaMatra* ocreator, Vector2 opos, double oangle, SpaceSprite *osprite);
		virtual ~SaMatraFlame();

		virtual void calculate();
		virtual void animate(Frame *space);
		virtual void death();

		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

class SaMatraBoxer : public SpaceObject
{
	public:
		double armour;
		SaMatra *samatra;
		double  timer, ftime;

	public:

		SaMatraBoxer(SaMatra* ocreator, Vector2 opos, double oangle, SpaceSprite *osprite);
		//	~SaMatraBoxer(void);

		virtual void calculate();
		virtual void death();
		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

SaMatra::SaMatra(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	angle = -0.5*PI;
	sprite_index = 0;

	Armour              = tw_get_config_float("Ship", "Armour", 0);

	weaponVelocity      = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage        = tw_get_config_float("Weapon", "Damage", 0);
	weaponArmour        = tw_get_config_float("Weapon", "Armour", 0);
	weaponNumber        = tw_get_config_int("Weapon", "Number", 1);
	weaponTurnPerSec    = tw_get_config_float("Weapon", "TurnPerSec", 0);

	specialVelocity     = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage       = tw_get_config_float("Special", "Damage", 0);
	specialArmour       = tw_get_config_float("Special", "Armour", 0);
	specialTrailTimeLen = tw_get_config_float("Special", "TrailTimeLen", 1.0);
	specialNumber       = tw_get_config_int("Special", "Number", 1);
	specialTurnPerSec   = tw_get_config_float("Special", "TurnPerSec", 0);
	specialDamageDelay  = tw_get_config_float("Special", "DamageDelay", 0.1);

	Nweapons = 0;
	Nspecials = 0;

	startpos = pos;
	armour = Armour;
}


int SaMatra::activate_weapon()
{
	STACKTRACE;
	if (Nweapons < weaponNumber) {
		game->add(new SaMatraBoxer(this, pos, tw_random(PI2), data->spriteWeapon));
		return true;

	} else
	return false;

}


int SaMatra::activate_special()
{
	STACKTRACE;

	if (Nspecials < specialNumber) {
		game->add(new SaMatraFlame(this, pos, random(PI2), data->spriteSpecial));
		return true;

	} else
	return false;

}


void SaMatra::calculate()
{
	STACKTRACE;

	Ship::calculate();

	if (state == 0)
		state = 1;

	// don't move !!

	angle = -0.5*PI;			 // pointing upward
	sprite_index = 0;
	vel = 0;

	// fix position
	pos = startpos;

}


int SaMatra::handle_damage(SpaceLocation *src, double normal, double direct)
{
	STACKTRACE;
	double total;
	total = normal + direct;

	while (total > 0 && crew > 0) {
		armour -= total;

		if (armour >= 0)
			total = 0;
		else
			total = -armour;

		if (armour <= 0) {
			armour = Armour;
			--crew;
		}

		if (crew <= 0) {
			state = 0;
			break;
		}

	}

	return 0;
}


void SaMatra::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (!other->isShip())
		damage(other, 100000);
}


void SaMatra::death()
{
	STACKTRACE;
	Ship::death();

	// spawn a dead body? A space object...

	SpaceSprite *spr;
	sprite->unlock();
	spr = new SpaceSprite(sprite->get_bitmap(0), SpaceSprite::MASKED);

	sprite->lock();

	SpaceObject *o;

	o = new SpaceObject(0, pos, angle, spr);
	o->set_depth(DEPTH_ASTEROIDS);
	o->layer = LAYER_CBODIES;

	o->collide_flag_anyone = ALL_LAYERS;
	o->collide_flag_sameship = ALL_LAYERS;
	o->collide_flag_sameteam = ALL_LAYERS;
	o->mass = mass;

	game->add(o);
}


SaMatraBoxer::SaMatraBoxer(SaMatra* ocreator, Vector2 opos, double oangle, SpaceSprite *osprite)
:
SpaceObject(ocreator, opos, oangle, osprite)
{
	STACKTRACE;
	samatra = ocreator;

	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);

	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;
	collide_flag_anyone = ALL_LAYERS;

	mass = 50;
	armour = samatra->weaponArmour;

	++samatra->Nweapons;

	timer = 0;
	ftime = 0.3;
	sprite_index = 0;

	isblockingweapons = false;
	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void SaMatraBoxer::death()
{
	STACKTRACE;

	if (samatra)
		--samatra->Nweapons;
}


// da_ps = angle change per second

void home_in(SpaceLocation *yours, SpaceLocation *target, double *angle, double da_ps)
{
	STACKTRACE;
	if ((target && target->exists()) && (!target->isInvisible())) {
		double a, da;

		a = yours->trajectory_angle(target);

		//		//trajectory_angle(pos, l->normal_pos());
		//		a = ::trajectory_angle(pos, target->normal_pos());

		// required change in direction

		da = a - *angle;
		while (da >  PI)    da -= PI2;
		while (da < -PI)    da += PI2;

		double db;

		// max allowed change in direction
		db = da_ps * frame_time * 1E-3;

		if (da >  db)   da =  db;
		if (da < -db)   da = -db;

		*angle += da;
	}
}


void SaMatraBoxer::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();

	if (!(samatra && samatra->exists())) {
		samatra = 0;
		state = 0;
		return;
	}

	timer += frame_time * 1E-3;

	if (timer > ftime) {
		timer -= ftime;
		++sprite_index;

		if (sprite_index >= sprite->frames())
			sprite_index = 0;
	}

	//vel = Vector2(samatra->weaponVelocity, 0);
	home_in(this, ship->target, &angle, samatra->weaponTurnPerSec);

	vel = samatra->weaponVelocity * unit_vector(angle);
}


int SaMatraBoxer::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	double total = normal + direct;
	armour -= total;

	// damage points used
	if (armour < 0)
		total += armour;

	if (armour <= 0) {
		state = 0;

		// shouldn't there be a small explosion here?
	}

	return iround(total);
}


void SaMatraBoxer::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (!other->exists()) return;

	damage(other, samatra->weaponDamage);

	if (other->mass >= 1) {
		int weaponExplosionFrameCount, weaponExplosionFrameSize;

		weaponExplosionFrameCount = 6;
		weaponExplosionFrameSize = 100;

		game->add(new Animation(this, normal_pos(),
			samatra->data->spriteWeaponExplosion, 0, weaponExplosionFrameCount,
			weaponExplosionFrameSize, DEPTH_EXPLOSIONS));

		play_sound2(samatra->data->sampleWeapon[0]);

		// sampleSpecial also exists.
	}
}


class SaMatraPhaser : public Phaser
{
	public:
	public:

		SaMatraPhaser(Vector2 opos, Vector2 n, SaMatra *ship,
			SpaceSprite *sprite, int osprite_index, int *ocolors,
			int onum_colors, int ofsize, int steps, int step_time) ;

		virtual void calculate();
};

SaMatraPhaser::SaMatraPhaser(Vector2 opos, Vector2 _n, SaMatra *ship,
SpaceSprite *sprite, int osprite_index, int *ocolors,
int onum_colors, int ofsize, int steps, int step_size)
:
Phaser(ship, opos, _n, ship, sprite, osprite_index, ocolors, onum_colors, ofsize,
steps, step_size)
{
	STACKTRACE;

}


void SaMatraPhaser::calculate()
{
	STACKTRACE;
	Phaser::calculate();

	if (!ship) return;

	angle = ship->get_angle();
	sprite_index = get_index(angle);
	rel_pos = unit_vector(angle) * rel_pos.length();
	pos = normalize(ship->normal_pos() - rel_pos);

	return;
}


SpaceLocation *SaMatra::get_ship_phaser()
{
	STACKTRACE;

	relocate();

	return new SaMatraPhaser(
		pos - unit_vector(angle) * PHASE_MAX * size.x,
		unit_vector(angle) * PHASE_MAX * size.x,
		this, sprite, sprite_index, hot_color, HOT_COLORS,
		PHASE_DELAY, PHASE_MAX, PHASE_DELAY);
}


// locate the sa-matra far from the planet.

void SaMatra::relocate()
{
	STACKTRACE;
	// find the planet.

	int i;
	for(std::list<SpaceLocation*>::iterator i = game->item.begin();i!=game->item.end();i++) {
		if ((*i)->isPlanet()) {
			double a, R;
			a = random(PI2);
			R = map_size.x / 2;

			pos = (*i)->pos + R * unit_vector(a);
			startpos = pos;
			// for the Sa-Matra, startpos is important !!
			break;
		}
	}

	// this big ship has only 1 direction.
	angle = -0.5*PI;
	sprite_index = 0;
}


SaMatraFlame::SaMatraFlame(SaMatra* ocreator, Vector2 opos, double oangle, SpaceSprite *osprite)
:
SpaceObject(ocreator, opos, oangle, osprite)
{
	STACKTRACE;

	samatra = ocreator;

	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);

	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;
	collide_flag_anyone = ALL_LAYERS;

	isblockingweapons = false;

	mass = 0;
	armour = samatra->specialArmour;

	damage_delay = 0;
								 // this many times per second
	damage_delay_max = samatra->specialDamageDelay;

								 // in seconds
	trailTime = samatra->specialTrailTimeLen;
	trailNum = iround(1 + trailTime / (frame_time*1E-3));
	trailPos = new Vector2 [trailNum];

	int i;
	for ( i = 0; i < trailNum; ++i )
		trailPos[i] = pos;

	++samatra->Nspecials;
	attributes &= ~ATTRIB_STANDARD_INDEX;
}


SaMatraFlame::~SaMatraFlame()
{
	delete[] trailPos;
}


void SaMatraFlame::death()
{
	STACKTRACE;

	if (samatra)
		--samatra->Nspecials;
}


void SaMatraFlame::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();

	if (!(samatra && samatra->exists())) {
		samatra = 0;
		state = 0;
		return;
	}

	sprite_index = 0;

	home_in(this, ship->target, &angle, samatra->specialTurnPerSec);
	vel = samatra->specialVelocity * unit_vector(angle);

	int i;
	for ( i = trailNum-1; i > 0; --i )
		trailPos[i] = trailPos[i-1];

	trailPos[0] = pos;

	if (damage_delay > 0)
		damage_delay -= frame_time * 1E-3;
}


void SaMatraFlame::animate(Frame *space)
{
	STACKTRACE;
	//	sprite->animate(pos, 0, space);

	//	return;

	int i, N;

	N = sprite->frames();

	for ( i = N-1; i >= 0; --i ) {
		int k;
		k = ( i * (trailNum-1)) / (N-1);
		sprite->animate(trailPos[k], i, space);
	}
}


void SaMatraFlame::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (!other->exists()) return;

	if (damage_delay > 0)
		return;

	damage(other, samatra->specialDamage);

	damage_delay = damage_delay_max;

	//	play_sound2(samatra->data->sampleWeapon[0]);
	// sampleSpecial also exists.

}


int SaMatraFlame::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	double total = normal + direct;
	armour -= total;

	// damage points used
	if (armour < 0)
		total += armour;

	if (armour <= 0) {
		state = 0;
		// shouldn't there be a small explosion here?
	}

	return iround(total);
}


REGISTER_SHIP(SaMatra)
