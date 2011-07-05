/* $Id: shpmontr.cpp,v 1.15 2005/08/28 20:34:08 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE
#include "../melee/mview.h"

#define MONO_TRON_SEGMENTS_PER_SPACE 6

/**
 * Mono Tron
 *
 * created by: cyhawk@sch.bme.hu and forevian@freemail.hu
 *
 * This ship moves only in four directions. It is very fast and quite small.
 * It's weapon is a unique homing missile that also moves only in four
 * directions. It is very fast, has an average range (it has to travel a longer
 * path this way) and a small damage.
 * Pressing the special button makes the ship draw a wall of laser behind itself.
 * The laser wall disappears if the button is released or the ship bumps into
 * something or battery runs low. If something touches the wall it suffers
 * a fair amount of damage and that segment of the wall (and segments not in the
 * direction of the ship) disappears. The wall can also damage the Tron.
 */

class MonoLaser;

class MonoTron : public Ship
{
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;

		int          specialDamage;

		int          turning;
		double       turn_phase;
		MonoLaser*   last_laser;
		Vector2       mo;

		SAMPLE*      copy_of_sampleSpecial;

	public:
		MonoTron(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

								 // shoots a missile
		virtual int activate_weapon();
								 // handles laser wall
		virtual void calculate_fire_special();
		virtual void calculate();// assures correct movement
								 // ignores gravity
		virtual void calculate_gravity();
								 // turns 90 degrees
		virtual void calculate_turn_left();
								 // turns 90 degrees
		virtual void calculate_turn_right();
								 // does nothing
		virtual void calculate_thrust();
								 // does nothing
		virtual void calculate_hotspots();
		virtual int accelerate(SpaceLocation *source, double angle, double velocity,
			double max_speed);	 // avoids acceleration effects
		virtual ~MonoTron();	 // frees some memory
};

class MonoMissile : public HomingMissile
{
	public:
		// this missile homes with 90 degree turns

		SpaceLocation* from_beacon;
		SpaceLocation* to_beacon;
								 // keep track of target changes
		SpaceObject*   last_target;

		void set_up_beacons();	 // sets initial beacons

	public:
		MonoMissile( SpaceLocation* creator, double ox, double oy, double oangle, double ov,
			int odamage, double orange, int oarmour, SpaceLocation *opos,
			SpaceSprite *osprite, SpaceObject *otarget );

		virtual void calculate();// home
								 // use Missile::animate_predict
		virtual void animate_predict( Frame* space, int time );

		virtual void calculate_index();

		virtual ~MonoMissile();	 // frees up beacon memory
};

class MonoLaser : public Laser
{
	public:
		// a segment of a laser wall

		friend class MonoTron;

		SpaceLocation* endpoint;
		MonoLaser* laser;		 // preceding segment

	protected:
		void stay();			 // stays in place
		void dissolve();		 // vanishes and makes all preceding segments vanish as well

	public:
		MonoLaser( SpaceLocation* source, MonoLaser* olaser, int odamage );
		void calculate();		 // grows with the movement of it's creator
								 // inflicts damage then dissolves
		void inflict_damage( SpaceObject* other );
};

MonoTron::MonoTron(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code) :
Ship(opos, (PI/2) * ((int)((shipAngle + PI/4) / (PI/2)) % 4),
shipData, code)
{
	STACKTRACE;
	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);

	specialDamage = tw_get_config_int("Special", "Damage", 0);

	turning = false;
	turn_phase = 8;
	last_laser = NULL;
	mo = pos;

	int iangle = (int)((angle + PI/4) / (PI/2)) % 4;
	sprite_index = iangle * 16 + (int)turn_phase;

	// we make a copy so that sound.stop only stops our sound
	// this requires only a small bit of memory since the data
	// itself is only referenced not copied
	copy_of_sampleSpecial = new SAMPLE;
	copy_of_sampleSpecial->bits = data->sampleSpecial[0]->bits;
	copy_of_sampleSpecial->stereo = data->sampleSpecial[0]->stereo;
	copy_of_sampleSpecial->freq = data->sampleSpecial[0]->freq;
	copy_of_sampleSpecial->priority = data->sampleSpecial[0]->priority;
	copy_of_sampleSpecial->len = data->sampleSpecial[0]->len;
	copy_of_sampleSpecial->loop_start = data->sampleSpecial[0]->loop_start;
	copy_of_sampleSpecial->loop_end = data->sampleSpecial[0]->loop_end;
	copy_of_sampleSpecial->param = data->sampleSpecial[0]->param;
	copy_of_sampleSpecial->data = data->sampleSpecial[0]->data;
}


int MonoTron::activate_weapon()
{
	STACKTRACE;
	game->add(new MonoMissile(this, 0, size.y/2,
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon, target));
	return( TRUE );
}


void MonoTron::calculate_fire_special()
{
	STACKTRACE;
	if (special_recharge > 0)
		special_recharge -=  frame_time;

	special_low = FALSE;
	if ( fire_special ) {
		if ( batt < special_drain ) {
			special_low = TRUE;
		} else {
			if ( !last_laser || !last_laser->exists() ) {
				play_sound( copy_of_sampleSpecial );
				last_laser = new MonoLaser( this, NULL, specialDamage );
				game->add( last_laser );
			}else if ( last_laser->get_length() > map_size.x / MONO_TRON_SEGMENTS_PER_SPACE ||
			last_laser->get_length() > map_size.y / MONO_TRON_SEGMENTS_PER_SPACE ) {
				last_laser->stay();
				last_laser = new MonoLaser( this, last_laser, specialDamage );
				game->add( last_laser );
			}
			if ( special_recharge > 0 ) return;

			batt -= special_drain;
			special_recharge += special_rate;

			return;
		}
	}
	if ( last_laser ) {
		if ( last_laser->exists() ) {
			sound.stop( copy_of_sampleSpecial );
			last_laser->dissolve();
		}
		last_laser = NULL;
	}
}


void MonoTron::calculate_gravity()
{
	STACKTRACE;
	return;
}


void MonoTron::calculate_turn_left()
{
	STACKTRACE;
	if ( !turning && turn_left ) {
		turn_step = PI*3/2;
		turn_phase = 15.9;
		if ( last_laser ) {
			last_laser->stay();
			last_laser = new MonoLaser( this, last_laser, specialDamage );
			game->add( last_laser );
		}
	}
}


void MonoTron::calculate_turn_right()
{
	STACKTRACE;
	if ( !turning && turn_right ) {
		turn_step = PI/2;
		turn_phase = 0.1;
		if ( last_laser ) {
			last_laser->stay();
			last_laser = new MonoLaser( this, last_laser, specialDamage );
			game->add( last_laser );
		}
	}
}


void MonoTron::calculate_thrust()
{
	STACKTRACE;
	return;
}


void MonoTron::calculate_hotspots()
{
	STACKTRACE;
	return;
}


void MonoTron::calculate()
{
	STACKTRACE;
	int iangle = (int)((angle + PI/4) / (PI/2)) % 4;
	if ( last_laser &&(( pos.x != mo.x && iangle % 2 == 1 )||( pos.y != mo.y && iangle % 2 == 0 ))) {
		/* this probably means we bumped into something or something moved us */
		/* the laser wall disappears to avoid diagonal walls */
		if ( last_laser->exists() ) last_laser->dissolve();
		last_laser = NULL;
		sound.stop( copy_of_sampleSpecial );
	}

	Ship::calculate();
	turning = turn_left || turn_right;
	iangle = (int)((angle + PI/4) / (PI/2)) % 4;
	sprite_index = iangle * 16 + (int)turn_phase;
	if ( turn_phase < 7.5 ) turn_phase += 0.3;
	if ( turn_phase > 7.5 ) turn_phase -= 0.3;
	angle = iangle * PI/2;

	if ( iangle == 0 ) {
		vel.x = speed_max;
		vel.y = 0;
	}
	else if ( iangle == 1 ) {
		vel.x = 0;
		vel.y = speed_max;
	}
	else if ( iangle == 2 ) {
		vel.x = -speed_max;
		vel.y = 0;
	}
	else if ( iangle == 3 ) {
		vel.x = 0;
		vel.y = -speed_max;
	}

	mo = pos;
}


int MonoTron::accelerate( SpaceLocation *source, double angle, double velocity, double max_speed )
{
	STACKTRACE;

	if ( source!=this ) return false;
	return Ship::accelerate(source, angle, velocity, max_speed);
}


MonoTron::~MonoTron()
{
	delete copy_of_sampleSpecial;
	copy_of_sampleSpecial = NULL;
}


MonoMissile::MonoMissile( SpaceLocation* creator, double ox, double oy, double oangle,
double ov, int odamage, double orange, int oarmour,
SpaceLocation *opos, SpaceSprite *osprite, SpaceObject* otarget ) :
HomingMissile( creator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour,
0, opos, osprite, otarget )
{
	STACKTRACE;

	/*
	while (angle < 0)
	angle += PI2;
	int iangle = (int)((angle + PI/4) / (PI/2)) % 4;
	angle = iangle * PI/2;
	sprite_index = iangle;
	*/
	calculate_index();
	from_beacon = to_beacon = NULL;
	set_up_beacons();
	last_target = target;

	//attributes &= ~ATTRIB_STANDARD_INDEX;
}


void MonoMissile::calculate_index()
{
	STACKTRACE;
	while (angle < 0)
		angle += PI2;
	int iangle = (int)((angle + PI/4) / (PI/2)) % 4;
	angle = iangle * PI/2;
	changeDirection( angle );
	sprite_index = iangle;
}


void MonoMissile::calculate()
{
	STACKTRACE;
	Missile::calculate();

	if ( target != last_target ) set_up_beacons();
	last_target = target;

	if ( !(target && target->exists()) || target->isInvisible() ) {
		target = NULL;
		return;
	}

	if ( distance( from_beacon ) > distance( to_beacon )) {
		play_sound2( data->sampleWeapon[1] );
		if ( from_beacon )
			delete from_beacon;
		from_beacon = new SpaceLocation( this, pos, 0.0 );
		double tangle = trajectory_angle( target );
		int itangle = (int)((tangle + PI/4) / (PI/2)) % 4;
		changeDirection( (PI/2) * itangle );
		sprite_index = itangle;
		if ( itangle % 2 == 0 ) {
			if ( to_beacon )
				delete to_beacon;
			to_beacon = new SpaceLocation( this, Vector2(2*target->normal_pos().x-pos.x, pos.y), 0.0 );
		}
		else if ( itangle % 2 == 1 ) {
			if ( to_beacon )
				delete to_beacon;
			to_beacon = new SpaceLocation( this, Vector2(pos.x, 2*target->normal_pos().y-pos.y), 0.0 );
		}
	}
}


void MonoMissile::animate_predict( Frame* space, int time )
{
	STACKTRACE;
	//Missile::animate_predict( space, time );
	Missile::animate( space );
}


void MonoMissile::set_up_beacons()
{
	STACKTRACE;
	if ( !target || !target->exists() || target->isInvisible() ) {
		target = NULL;
	} else {
		int iangle = (int)((angle + PI/4) / (PI/2)) % 4;
		if ( from_beacon )
			delete from_beacon;
		from_beacon = new SpaceLocation( this, pos, 0.0 );
		if ( iangle % 2 == 0 ) {
			if ( to_beacon )
				delete to_beacon;
			to_beacon = new SpaceLocation( this, Vector2(2*target->normal_pos().x-pos.x, pos.y), 0.0 );
		}
		else if ( iangle % 2 == 1 ) {
			if ( to_beacon )
				delete to_beacon;
			to_beacon = new SpaceLocation( this, Vector2(pos.x, 2*target->normal_pos().y-pos.y), 0.0 );
		}
	}
}


MonoMissile::~MonoMissile()
{
	if ( from_beacon )
		delete from_beacon;
	if ( to_beacon )
		delete to_beacon;
}


MonoLaser::MonoLaser( SpaceLocation* source, MonoLaser* olaser, int odamage )
:Laser( source, PI, pallete_color[15], 0.0, odamage, 500, source, Vector2(0,0) )
{
	STACKTRACE;
	collide_flag_sameteam = ALL_LAYERS;
	endpoint = new SpaceLocation( this, pos, 0.0 );
	laser = olaser;
	if ( laser ) if ( !laser->exists() ) laser = NULL;
}


void MonoLaser::stay()
{
	STACKTRACE;
	lpos = new SpaceLocation( this, pos, 0.0 );
	if ( laser ) {
		if ( laser->exists() ) {
			laser->collide_flag_sameship = ALL_LAYERS;
		}else laser = NULL;
	}
}


void MonoLaser::calculate()
{
	STACKTRACE;
	angle = trajectory_angle( endpoint );
	length = distance( endpoint );
	frame -= frame_time;		 /* we do not want to dissolve */
	Laser::calculate();
	if ( state == 0 ) dissolve();
	if ( laser ) if ( !laser->exists() ) laser = NULL;
}


void MonoLaser::dissolve()
{
	STACKTRACE;
	state = 0;
	if ( laser ) {
		if ( laser->exists() ) {
			laser->dissolve();
		}else laser = NULL;
	}
}


void MonoLaser::inflict_damage( SpaceObject* other )
{
	STACKTRACE;
	Laser::inflict_damage( other );
	dissolve();
}


REGISTER_SHIP(MonoTron)
