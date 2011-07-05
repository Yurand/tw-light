/* $Id: shpalckr.cpp,v 1.18 2005/09/15 09:01:37 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE
#include "../scp.h"
#include "../util/aastr.h"
#include "../melee/mview.h"
#include "../melee/manim.h"

// if you change the order you have to change the increment in the code too
#define ALCHERO_TRACE_START_INDEX      88
#define ALCHERO_TRACE_END_INDEX        100
#define ALCHERO_TRACE_FADE_RATE        10
// to tweak the charge effect, see the AlcheroLaser::calculate() function

/*
 * created by: cyhawk@sch.bme.hu and forevian@freemail.hu
 */
class AlcheroKronos : public Ship
{
	public:
		// the ship, dial included

		double dial_angle;
		int    dial_index;
		double fps_orig;

		double         weaponDeccel;
		double         weaponMinSpeed;
		double         weaponVelocity;
		double         weaponGrowth;
		double         weaponMinLength;
		int            weaponDamage;
		int            weaponRate;
		double         weaponsparkNfactor;
		SpaceLine     *weaponObject;

		double specialMinTime;
		double specialSpeedDown;
		double specialSpeedUp;

		double turn_rate_orig;
		double speed_max_orig;
		double accel_rate_orig;
		int    hotspot_rate_orig;
		int    recharge_rate_orig;
		int    weapon_rate_orig;
		int    special_rate_orig;
		double mass_orig;

								 // compensates for turbo change
		void compensate( double t );

		SAMPLE* copy_of_sampleSpecial;

		bool up;

	protected:
		double turbo_change;	 // this ship changed the turbo by this value

	public:
		AlcheroKronos(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

								 // starts growing a laser beam
		virtual int activate_weapon();
								 // slows down time
		virtual int activate_special();
								 // speeds up time
		virtual int deactivate_special();
		virtual void calculate();// takes note of losing the beam
								 // play sound only when starting charge
		virtual void calculate_fire_weapon() ;
								 // calls activate_special or deactivate_special
		virtual void calculate_fire_special();
								 // hotspots had to be placed closer to the ship
		virtual void calculate_hotspots();
								 // draws charge gfx
		virtual void animate( Frame* space );
		virtual void death();	 // restores turbo

		virtual ~AlcheroKronos();
};

class AlcheroLaser : public Laser
{
	public:
		// a laser beam flying by itself and slowing down with time

		bool   released;

		double  deccel;
		double  min_speed;
		double  velocity;
		double  growth;
		double  min_length;
		double  olength;
		double  sparkNfactor;
		SAMPLE* sample;

		int    step;
		int    rate;

	public:
		AlcheroLaser(SpaceLocation *creator, double langle, int lcolor, int ldamage,
			int orate, SpaceLocation *opos, double rel_x, double rel_y, double odeccel,
			double omin_speed, double ovelocity, double ogrowth, double omin_length,
			double osparkNfactor);

		virtual void calculate();
};

class AlcheroLaserTrace : public Laser
{
	public:
		// a non-colliding laser beam that stays in place and fades away

		int    step;
		int    rate;
		int    color_index;

	public:
		AlcheroLaserTrace( SpaceLocation *creator, double langle, int lcolor,
			double lrange, int orate );

		virtual void calculate();
};

AlcheroKronos::AlcheroKronos(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code ):
Ship( opos, shipAngle, shipData, code )
{
	STACKTRACE;
	weaponDeccel     = tw_get_config_float( "Weapon", "DeccelRate", 0 ) * distance_ratio / time_ratio / time_ratio;
	weaponMinSpeed   = scale_velocity( tw_get_config_float( "Weapon", "MinSpeed", 0 ));
	weaponVelocity   = scale_velocity( tw_get_config_float( "Weapon", "Velocity", 0 ));
	weaponGrowth     = scale_velocity( tw_get_config_float( "Weapon", "Growth", 0 ));
	weaponMinLength  = scale_range( tw_get_config_float( "Weapon", "MinLength", 0 ));
	weaponDamage     = tw_get_config_int( "Weapon", "Damage", 0 );
	weaponRate       = scale_frames( tw_get_config_int( "Weapon", "Rate", 0 ));
	weaponObject     = NULL;
	weaponsparkNfactor = tw_get_config_float( "Weapon", "sparkNfactor", 0 );

	specialMinTime   = tw_get_config_float( "Special", "MinTime", 0 );
	specialSpeedDown = tw_get_config_float( "Special", "SpeedDown", 0 );
	specialSpeedUp   = tw_get_config_float( "Special", "SpeedUp", 0 );

	turbo_change       = 0;
	fps_orig           = game->get_turbo() / frame_time;

	turn_rate_orig     = turn_rate;
	speed_max_orig     = speed_max;
	accel_rate_orig    = accel_rate;
	hotspot_rate_orig  = hotspot_rate;
	recharge_rate_orig = recharge_rate;
	weapon_rate_orig   = weapon_rate;
	special_rate_orig  = special_rate;
	mass_orig          = mass;

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

	dial_angle = 0;
	dial_index = get_index(angle + dial_angle);

	// for debugging purpose
	//	debug_id = 50;
}


int AlcheroKronos::activate_weapon()
{
	STACKTRACE;
	if ( !weaponObject ) {
		weaponObject = new AlcheroLaser( this, angle, pallete_color[ALCHERO_TRACE_START_INDEX], weaponDamage,
			weaponRate, this, 0, 0.5*size.y, weaponDeccel, weaponMinSpeed, weaponVelocity, weaponGrowth,
			weaponMinLength, weaponsparkNfactor );
		game->add( weaponObject );
		play_sound2( data->sampleWeapon[weapon_sample], 1000 );
	}
	return TRUE;
}


void AlcheroKronos::calculate()
{
	STACKTRACE;
	Ship::calculate();

	dial_angle += turbo_change * frame_time / (1.0 + turbo_change) * PI/180;

	while (dial_angle > PI)
		dial_angle -= PI2;
	while (dial_angle < -PI)
		dial_angle += PI2;

	dial_index = get_index(angle + dial_angle);
	if ( weaponObject ) {
		if ( !weaponObject->exists() || !fire_weapon || !batt ) {
			weaponObject = NULL;
		}
	}
}


int AlcheroKronos::activate_special()
{
	STACKTRACE;
	double t_old = game->get_turbo();
	double t = t_old;
	double tnew = t - specialSpeedDown;
	if ( tnew > specialMinTime ) {
								 //specialSpeedDown;
		turbo_change -= (t - tnew);
		t = tnew;
	} else {

		//turbo_change += specialMinTime - t;
		//t = specialMinTime;
		tnew = specialMinTime;
								 //specialSpeedDown;
		turbo_change -= (t - tnew);
		t = tnew;

	}

	if (t < 0 || t > 1.0 + 1E-6) {
		tw_error("Turbo change out of bounds");
	}

	game->set_turbo( t );
	game->set_frame_time( iround(t / fps_orig) );

	if ( up ) {
		sound.stop( data->sampleSpecial[1] );
		play_sound2( copy_of_sampleSpecial, 255, 1000 );
		up = false;
	}

	if (fabs(1.0+turbo_change) < 1E-3) {
		tw_error("Something wrong with the turbo-change");
	}

	compensate( 1.0 + turbo_change );

	return TRUE;
}


int AlcheroKronos::deactivate_special()
{
	STACKTRACE;
	double t = game->get_turbo();

	if ( turbo_change == 0 )
		return TRUE;

	if ( turbo_change + specialSpeedUp < 0 ) {
		t += specialSpeedUp;
		turbo_change += specialSpeedUp;
	} else {
		t -= turbo_change;
		turbo_change = 0;
	}

	if (t < 0 || t > 1.0 + 1E-6) {
		tw_error("Turbo change out of bounds");
	}

	game->set_turbo( t );
	game->set_frame_time( iround(t / fps_orig) );

	if ( !up ) {
		sound.stop( copy_of_sampleSpecial );
		play_sound2( data->sampleSpecial[1], 255, 1000 );
		up = true;
	}

	compensate( 1.0 + turbo_change );

	double ov = magnitude(vel);

	if ( ov > speed_max)
		vel *= speed_max/ov;

	return TRUE;
}


void AlcheroKronos::compensate( double t )
{
	STACKTRACE;
	turn_rate = turn_rate_orig / t;
	speed_max = speed_max_orig / t;
	accel_rate = accel_rate_orig / (t * t);
	hotspot_rate = (int)(hotspot_rate_orig * t);
	recharge_rate = (int)(recharge_rate_orig * t);
	weapon_rate = (int)(weapon_rate_orig * t);
	special_rate = (int)(special_rate_orig * t);
	mass = mass_orig * t;
}


void AlcheroKronos::calculate_fire_weapon()
{
	STACKTRACE;
	weapon_low = FALSE;

	if (fire_weapon) {
		if (batt < weapon_drain) {
			weapon_low = true;
			return;
		}

		if (weapon_recharge > 0)
			return;

		if (!activate_weapon())
			return;

		batt -= weapon_drain;
		if (recharge_amount > 1) recharge_step = recharge_rate;
		weapon_recharge += weapon_rate;
	}
	return;
}


void AlcheroKronos::calculate_fire_special()
{
	STACKTRACE;
	special_low = FALSE;

	if (fire_special && batt >= special_drain) {
		if (special_recharge > 0)
			return;

		if (!activate_special())
			return;

		batt -= special_drain;
		special_recharge += special_rate;
	} else {
		if (batt < special_drain)
			special_low = TRUE;

		if (special_recharge > 0)
			return;

		if (!deactivate_special())
			return;

		special_recharge += special_rate;
	}
}


void AlcheroKronos::calculate_hotspots()
{
	STACKTRACE;
	if ((thrust) && (hotspot_frame <= 0)) {
		game->add(new Animation( this,
			pos - unit_vector(angle) * size.x / 4.5,
		//      normal_x() - cos(angle) * w / 4.5,
		//      normal_y() - sin(angle) * w / 4.5,
			meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, LAYER_HOTSPOTS));
		hotspot_frame += hotspot_rate;
	}
	if (hotspot_frame > 0) hotspot_frame -= frame_time;
	return;
}


void AlcheroKronos::animate( Frame* space )
{
	STACKTRACE;
	Ship::animate( space );
	//  if ( dial_index != sprite_index )
	data->spriteWeapon->animate( pos, dial_index, space );
}


void AlcheroKronos::death()
{
	STACKTRACE;
	if ( turbo_change != 0 ) {
		double t = game->get_turbo();
		t -= turbo_change;

		if (t < 0 || t > 1.0 + 1E-6) {
			tw_error("Turbo change out of bounds");
		}

		turbo_change = 0;
		game->set_turbo( t );
		game->set_frame_time( iround(t / fps_orig) );
	}
}


AlcheroKronos::~AlcheroKronos()
{
	delete copy_of_sampleSpecial;
	copy_of_sampleSpecial = NULL;
}


AlcheroLaser::AlcheroLaser(SpaceLocation *creator, double langle, int lcolor, int ldamage,
int orate, SpaceLocation *opos, double rel_x, double rel_y, double odeccel,
double omin_speed, double ovelocity, double ogrowth, double omin_length,
double osparkNfactor):
Laser( creator, langle, lcolor, omin_length, ldamage, 1000, opos, Vector2(rel_x,rel_y), true )
{
	STACKTRACE;
	released = false;
	rate = orate;
	deccel = odeccel;
	min_speed = omin_speed * omin_speed;
	velocity = ovelocity;
	growth = ogrowth;
	min_length = omin_length;
	olength = min_length;
	collide_flag_sameship = collide_flag_sameteam = 0;
	collide_flag_anyone = ALL_LAYERS;
	step = 0;
	sparkNfactor = osparkNfactor;
}


void AlcheroLaser::calculate()
{
	STACKTRACE;
	if ( step > 0 ) step -= frame_time;
	if ( step <= 0 ) {
		step += rate;
		if ( !released ) {
			int l = (int)(1 + sparkNfactor * length/min_length);
			for( int i = 0; i < l; i++ ) {
				double alpha = tw_random(PI2);
				Animation *anim  = new Animation( this, pos + 0.5 * ship->size.x *unit_vector(alpha),
				//      x + 0.5*ship->width()*cos( alpha ), y + 0.5*ship->width()*sin( alpha ),
					data->spriteExtra, 0, data->spriteExtra->frames(), 50+(random(160)), LAYER_HOTSPOTS );

				anim->set_vel ( vel + Vector2(- 0.2*cos( alpha ) - (100+random(100))*0.0005*sin( alpha ),
					- 0.2*sin( alpha ) + (100+random(100))*0.0005*cos( alpha )) );
				game->add( anim );
			}
		}
		collide_flag_anyone = ALL_LAYERS;
		if ( released ) {
			game->add( new AlcheroLaserTrace( this, angle, ALCHERO_TRACE_START_INDEX, length, ALCHERO_TRACE_FADE_RATE ));
		}
	}

	if ( length < min_length ) {
		state = 0;
		return;
	}

	if ( !released ) {
		frame = 0;				 // don't die

		if ( !ship->exists() || !ship->fire_weapon || !ship->batt ) {
			if ( ship->exists() ) {
				angle = ship->get_angle();
				//        vx = velocity * cos( angle ) + ship->get_vx() * game->shot_relativity;
				//        vy = velocity * sin( angle ) + ship->get_vy() * game->shot_relativity;
				vel = velocity * unit_vector(angle) + ship->get_vel() * game->shot_relativity;
			} else {
				vel = velocity * unit_vector(angle);
				//        vx = velocity * cos( angle );
				//        vy = velocity * sin( angle );
			}
			released = true;
			sound.stop( data->sampleWeapon[0] );
			play_sound2( data->sampleWeapon[1] );
		} else {
			Laser::calculate();
			if ( length < olength ) {
				length /= 2;
			} else {
				length += growth * frame_time;
			}
			olength = length;
		}
	} else {
		SpaceLocation::calculate();
		//	vx *= 1.0 - deccel * frame_time;
		//	vy *= 1.0 - deccel * frame_time;
		vel *= exp(-deccel * frame_time);
								 //(vx * vx) + (vy * vy);
		double vv = magnitude_sqr(vel);
		if ( vv < min_speed ) state = 0;
		//double v = sqrt( vv );
		//accelerate( this, angle, -v * deccel * frame_time, v );
	}
}


AlcheroLaserTrace::AlcheroLaserTrace( SpaceLocation *creator, double langle,
int lcolor, double lrange, int orate ):
Laser( creator, langle, pallete_color[lcolor], lrange, 0, 1000, creator, 0, true ),
step( 0 ), rate( orate ), color_index( lcolor )
{
	STACKTRACE;
	collide_flag_anyone = collide_flag_sameship = collide_flag_sameteam = 0;
	vel = 0;					 //vx = vy = 0;

}


void AlcheroLaserTrace::calculate()
{
	STACKTRACE;
	while( step <= 0 ) {
		step += rate;
		color_index++;
		if ( color_index >= ALCHERO_TRACE_END_INDEX ) state = 0;
		color = pallete_color[color_index];
	}
	step -= frame_time;

	SpaceLocation::calculate();
}


REGISTER_SHIP(AlcheroKronos)
