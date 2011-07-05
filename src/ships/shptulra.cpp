/* $Id: shptulra.cpp,v 1.14 2004/03/24 23:51:42 yurand Exp $ */
#include "../ship.h"
REGISTER_FILE
#include "../melee/mview.h"

#define TULKON_DEVICE_MIN_DIST 26
#define TULKON_DEVICE_MAX_DIST 50
#define TULKON_DEVICE_PULL 2
#define TULKON_BOMB_DROP_DIST 16

class TulkonDevice;

/*
 * created by: cyhawk@sch.bme.hu and forevian@freemail.hu
 */
class TulkonRam : public Ship
{
	// the body of the ship

	double      weaponForce;
	int         weaponDamage;
	double      weaponShieldFraction;

	int         specialDamage;
	int         specialArmour;
	double      specialMass;
	double      specialDRange;
	double      specialSRange;
	int         specialImmuneToBombs;

	TulkonDevice *ram;

	SpaceObject** bombs;
	int           numBombs;
	int           maxBombs;

	public:
		TulkonRam( Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code );

								 // furthers the ramming device
		virtual int activate_weapon();
								 // ejects a mine
		virtual int activate_special();
								 // releases the ramming device when needed
		virtual void calculate_fire_weapon();
								 // sound tweak
		virtual void calculate_fire_special();
		virtual void calculate();// keeps bomb registry up to date
								 // places two streams of blue hotspots
		virtual void calculate_hotspots();
		virtual int handle_fuel_sap( SpaceLocation* source, double normal );
								 // places the ramming device into game space
		virtual void materialize();
};

class TulkonDevice : public SpaceObject
{
	// the ramming device

	//  double px, py;
	//  double pvx, pvy;
	Vector2 P, PV;

	double force;

	void ram( bool mode );

	public:
		TulkonDevice( Ship* creator, SpaceSprite* osprite, double odist,
			int odamage, double oforce, double ofraction );

		double dist;			 // distance of ship and device
		bool   ramming;			 // if we are ramming this frame
		double ShieldFraction;	 // fractional damage of the RAM shield

		virtual void calculate();// follows ship movement
								 // prevents collision with ship
		virtual int canCollide( SpaceLocation* other );
								 // forwards kinetic energy to ship
		virtual void collide( SpaceObject* other );
								 // rams on contact and damages
		virtual void inflict_damage( SpaceObject* other );
								 // forwards directDamage and play sfx
		virtual int handle_damage( SpaceLocation* other, double normal, double direct );
};

class TulkonBomb : public AnimatedShot
{
	// bombs

	friend class TulkonDevice;

	SpaceLocation* creator;
	double damageInflicted;
	double drange;
	double srange;
	bool   exploding;
	protected:
		bool   rammed;

	public:
		int immunity;

		SpaceLocation* ram;
		TulkonBomb( SpaceLocation* ocreator, Vector2 opos, int odamage, double odrange,
			double osrange, double oarmour, double omass, SpaceSprite* osprite,
			double orelativity = game->shot_relativity );

		virtual void calculate();// checks sensory range for heat
								 // does not inflict damage
		virtual void inflict_damage( SpaceObject* other );
								 // if rammed flies straight
		virtual void collide( SpaceObject* other );
								 // if destroyed explodes
		virtual int handle_damage( SpaceLocation* source, double normal, double direct );
								 // Forevian's sound needs to be laud
		virtual void soundExplosion();
};

TulkonRam::TulkonRam( Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code ):
Ship( opos, shipAngle, shipData, code )
{
	STACKTRACE;
	weaponForce   = get_config_float( "Weapon", "Force", 0 );
	weaponDamage  = get_config_int( "Weapon", "Damage", 0 );
	weaponShieldFraction = get_config_float( "Weapon", "ShieldFraction", 0 );

	specialDamage = get_config_int( "Special", "Damage", 0 );
	specialArmour = get_config_int( "Special", "Armour", 0 );
	specialMass   = get_config_float( "Special", "Mass", 0 );
	specialDRange = scale_range( get_config_float( "Special", "DamageRange", 0 ));
	specialSRange = scale_range( get_config_float( "Special", "SensorRange", 0 ));
	specialImmuneToBombs = get_config_int("Special", "ImmuneToBombs", 0);

	ram = new TulkonDevice( this, data->spriteWeapon, TULKON_DEVICE_MAX_DIST,
		weaponDamage, weaponForce, weaponShieldFraction );

	numBombs = 0;
	maxBombs = get_config_int("Special", "Number", 10);
	bombs = new SpaceObject*[maxBombs];
}


int TulkonRam::activate_weapon()
{
	STACKTRACE;
	if ( ram->dist <= TULKON_DEVICE_MIN_DIST ) return FALSE;
	ram->dist -= TULKON_DEVICE_PULL;
	return TRUE;
}


int TulkonRam::activate_special()
{
	STACKTRACE;
	TulkonBomb* TB;
	if ( numBombs == maxBombs ) {
		bombs[0]->state = 0;
		numBombs--;
		for( int i = 0; i < numBombs; i++ ) {
			bombs[i] = bombs[i + 1];
		}
	}

	Vector2 droppos;
	//droppos = Vector2(-get_size().x/2-TULKON_BOMB_DROP_DIST, get_size().y/5);
	// GEO: imo it's strange that they're dropped on the side. Do it on the back
	droppos = Vector2(0, -0.45*get_size().y - TULKON_BOMB_DROP_DIST);
	TB = new TulkonBomb( this, droppos, specialDamage,
		specialDRange, specialSRange, specialArmour, specialMass, data->spriteSpecial, 1.0 );
	TB->ram = ram;
	TB->immunity = this->specialImmuneToBombs;
	bombs[numBombs] = TB;
	game->add( bombs[numBombs] );
	numBombs++;

	special_sample = random() % 3;
	return TRUE;
}


void TulkonRam::calculate_fire_special()
{
	STACKTRACE;
	special_low = FALSE;

	if (fire_special) {
		if (batt < special_drain) {
			special_low = true;
			return;
		}

		if (special_recharge > 0)
			return;

		if (!activate_special())
			return;

		batt -= special_drain;
		if (recharge_amount > 1) recharge_step = recharge_rate;
		special_recharge += special_rate;

		play_sound2(data->sampleSpecial[special_sample], 1000);
	}
	return;
}


void TulkonRam::calculate_fire_weapon()
{
	STACKTRACE;
	Ship::calculate_fire_weapon();
	if ( !fire_weapon && ram->dist != TULKON_DEVICE_MAX_DIST ) {
		ram->ramming = true;
	} else {
		ram->ramming = false;
	}
}


void TulkonRam::calculate()
{
	STACKTRACE;
	int j = 0;
	for( int i = 0; i < numBombs; i++ ) {
		if (j)
			bombs[i - j] = bombs[i];
		if ( !bombs[i]->exists() ) j++;
	}
	numBombs -= j;

	recharge_rate = scale_frames( 11 - crew / 2 );

	Ship::calculate();
}


void TulkonRam::calculate_hotspots()
{
	STACKTRACE;
	double xx, yy, tx, ty, sz;
	Vector2 vv;
	sz = (this->size).magnitude();
	if ( thrust && hotspot_frame <= 0 ) {
		tx = cos( angle );
		ty = sin( angle );
		xx = this->pos.x - tx * sz / 2.5 + ty * sz / 5;
		yy = this->pos.y - ty * sz / 2.5 - tx * sz / 5;
		vv.x = xx; vv.y = yy;
		Vector2 t = unit_vector(angle);
		game->add( new Animation( this,
			vv, data->spriteExtra, 0, 12, time_ratio, LAYER_HOTSPOTS));
		xx = this->pos.x - tx * sz / 2.5 - ty * sz / 5;
		yy = this->pos.y - ty * sz / 2.5 + tx * sz / 5;
		vv.x = xx; vv.y = yy;
		game->add( new Animation( this,
			vv, data->spriteExtra, 0, 12, time_ratio, LAYER_HOTSPOTS));
		hotspot_frame += hotspot_rate;
	}
	if ( hotspot_frame > 0 ) hotspot_frame -= frame_time;
}


int TulkonRam::handle_fuel_sap( SpaceLocation* other, double normal )
{
	STACKTRACE;
	return 0;
}


void TulkonRam::materialize()
{
	STACKTRACE;
	Ship::materialize();
	game->add( ram );
}


TulkonDevice::TulkonDevice( Ship* creator, SpaceSprite* osprite,
double odist, int odamage, double oforce, double ofraction )
:
SpaceObject( creator, creator->normal_pos(), creator->get_angle(), osprite ),
force( oforce ), dist( odist ), ramming( false )
{
	STACKTRACE;
	ShieldFraction = ofraction;
	layer = LAYER_SPECIAL;
	collide_flag_sameship = collide_flag_sameteam = ALL_LAYERS;

	//  vx = ship->get_vx();
	//  vy = ship->get_vy();
	vel = ship->get_vel();
	angle = ship->get_angle();
	sprite_index = get_index(angle);

	//  x = ship->normal_x() + cos( angle ) * dist;
	//  y = ship->normal_y() + sin( angle ) * dist;
	pos = ship->normal_pos() + dist * unit_vector(angle);

	mass = ship->mass;
	damage_factor = odamage;
}


void TulkonDevice::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();
	if ( !(ship && ship->exists()) ) {
		ship = 0;				 // not really needed
		state = 0;
		return;
	}

	ram( ramming );

	angle = ship->get_angle();
	sprite_index = get_index(angle);

	if ( dist > TULKON_DEVICE_MAX_DIST ) dist = TULKON_DEVICE_MAX_DIST;
	if ( dist < TULKON_DEVICE_MIN_DIST ) dist = TULKON_DEVICE_MIN_DIST;

	//  x = ship->normal_x() + cos( angle  ) * dist;
	//  y = ship->normal_y() + sin( angle  ) * dist;
	pos = ship->normal_pos() + dist * unit_vector(angle);

	//  px = x; py = y; pvx = vx; pvy = vy;

	P = pos;
	PV = vel;
}


int TulkonDevice::canCollide( SpaceLocation* other )
{
	STACKTRACE;
	if ( other == ship ) return FALSE;
	return SpaceObject::canCollide( other );
}


void TulkonDevice::collide( SpaceObject* other )
{
	STACKTRACE;
	if ( ramming ) {
		// these lines of code execute
		// once a [ramming] frame
		// after all calculate()s
		// and before any inflict_damage()s
								 // play ram_nohit.wav
		play_sound2( data->sampleWeapon[1] );
	}
	SpaceObject::collide( other );
	// by now all collision calculations must have occured either on our side or on the other
	if ( ramming ) {
		// we return to the speed of the ship to avoid jumping away a bit
		ram( false );
		return;
	}
	if ( pos != P ) ship->translate( pos - P );
	if ( vel != PV ) {
		ship->vel = PV;
	}
}


void TulkonDevice::inflict_damage( SpaceObject* other )
{
	STACKTRACE;
	if ( dist < TULKON_DEVICE_MAX_DIST - TULKON_DEVICE_PULL &&
		( sameShip( other ) || !other->isShot() )) ram( true );
	if ( !ramming ) {
		int odf = iround(damage_factor);
		damage_factor = 0;
		SpaceObject::inflict_damage( other );
		damage_factor = odf;
	} else {
		SpaceObject::inflict_damage( other );
	}
	if ( ramming ) {
								 // this must be a TulkonBomb
		if ( sameShip( other ) && !other->isShip() ) {
								 // stop ram_nohit.wav
			sound.stop( data->sampleWeapon[1] );
								 // play ram_mine.wav
			play_sound( data->sampleWeapon[2], 1000 );
			TulkonBomb *tb = (TulkonBomb*)other;
			// we tell her to fly in the way we're facing
			tb->rammed = true;
			tb->angle = angle;
		} else {
								 // stop ram_nohit.wav
			sound.stop( data->sampleWeapon[1] );
								 // play ram_ship.wav
			play_sound( data->sampleWeapon[3], 1000 );
		}
	}
}


int TulkonDevice::handle_damage( SpaceLocation* other, double normal, double direct )
{
	STACKTRACE;
	//  if ( direct ) {
	//	  ship->damage(other, 0, direct);
	//  }
	// let the mothership take at least *some* damage, cause permanent armour sucks
	ship->handle_damage(other, int(ShieldFraction*(normal+direct)) );

	normal += direct;
	if ( normal > 0 && normal <= 2 ) {
								 // play ram_smallhit.wav
		play_sound( data->sampleWeapon[4], 500 );
	}
	else if ( normal > 2 ) {
								 // play ram_largehit.wav
		play_sound( data->sampleWeapon[5], 500 );
	}
	return iround(normal);
}


void TulkonDevice::ram( bool mode )
{
	STACKTRACE;
	if ( ramming = mode ) {
		mass = 1000 * (TULKON_DEVICE_MAX_DIST - dist);
		//    vx = ship->get_vx() + cos( angle  ) * (TULKON_DEVICE_MAX_DIST - dist) *
		//      0.001 * force;
		//    vy = ship->get_vy() + sin( angle  ) * (TULKON_DEVICE_MAX_DIST - dist) *
		//      0.001 * force;
		vel = ship->get_vel() + unit_vector(angle) * (TULKON_DEVICE_MAX_DIST - dist) *
			0.001 * force;
		dist = TULKON_DEVICE_MAX_DIST;
		sound.stop( data->sampleWeapon[0] );
	} else {
		//    vx = ship->get_vx();
		//    vy = ship->get_vy();
		vel = ship->get_vel();
		mass = ship->mass;
	}
}


TulkonBomb::TulkonBomb( SpaceLocation* ocreator, Vector2 opos, int odamage,
double odrange, double osrange, double oarmour, double omass, SpaceSprite* osprite,
double orelativity ):
AnimatedShot( ocreator, opos, 0, 0, odamage, -1, oarmour, ocreator, osprite, 64, time_ratio, orelativity ),
rammed( false )
{
	STACKTRACE;
	damageInflicted = odamage;
	creator=ocreator;
	collide_flag_sameship = collide_flag_sameteam = ALL_LAYERS;
	mass = omass;
	srange = osrange;
	drange = odrange;
	exploding = false;
	explosionSample = data->sampleSpecial[3];
	explosionSprite = data->spriteSpecialExplosion;
	explosionFrameCount = 12;

	sprite_index = random() % 64;
}


void TulkonBomb::calculate()
{
	STACKTRACE;
	AnimatedShot::calculate();

	/*
	Query q;
	for( q.begin( this, bit(LAYER_HOTSPOTS), srange ); q.currento; q.next() ){
	  if ( q.currento->get_sprite() == game->hotspotSprite ){
			damage(this, armour);
	  }
	}
	q.end();
	It's dangerous to depend on animations for your physics; animations are intended for eye-candy,
	*/

	Query q;
	for( q.begin( this, OBJECT_LAYERS, srange ); q.currento; q.next() ) {
		if ( q.currento->isObject() && !q.currento->sameShip(this) ) {
			damage(this, armour);
		}
	}
	q.end();
}


void TulkonBomb::collide( SpaceObject* other )
{
	STACKTRACE;
	AnimatedShot::collide( other );
	// by now all collision calculations must have occured either on our side or on the other
	if ( rammed ) {
		//    double v = sqrt( vx*vx + vy*vy );
		v = magnitude(vel);
		//    vx = cos( angle ) * v;
		//    vy = sin( angle ) * v;
		vel = v * unit_vector(angle);
		rammed = false;
	}
}


int TulkonBomb::handle_damage( SpaceLocation* other, double normal, double direct )
{
	STACKTRACE;
								 // creator can't set them off by slamming them with the ram.
	if (other==creator||other==ram)return(0);
	int s = exists();
	int d= AnimatedShot::handle_damage( other, normal, direct );
	if ( s && !exists() ) {
		Query q;
		for( q.begin( this, OBJECT_LAYERS, drange ); q.currento; q.next() ) {
			// we could use a distance dependant damage factor
			//      int dmg = (int)ceil((drange - distance(q.currento)) / drange * damage_factor);
			//q.currento->damage(this, damage_factor);
			if (immunity&&(q.currento==creator||q.currento==ram))
				;
			else
				damage(q.currento, 0, damageInflicted);

		}
		animateExplosion();
		soundExplosion();
	}
	return d;
}


void TulkonBomb::soundExplosion()
{
	STACKTRACE;
	play_sound2( explosionSample, 1000 );
}


void TulkonBomb::inflict_damage( SpaceObject* other )
{
	STACKTRACE;
	return;
}


REGISTER_SHIP(TulkonRam)
