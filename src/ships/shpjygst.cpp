/* $Id: shpjygst.cpp,v 1.20 2005/08/28 20:34:08 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE
#include "../melee/mview.h"

/*
 * created by: cyhawk@sch.bme.hu and forevian@freemail.hu
 */
class JyglarStarfarer : public Ship
{
	public:
		// the ship

								 // maximum range
		double        weaponRange;
		double        weaponVelocity;
								 // does damage between zero and this value inclusive
		int           weaponDamage;
		int           weaponArmour;
								 // percentage of shots going astray
		int           weaponStray;
		double        weaponPull;// pulling force strength
		int           melody;

		double        specialMass;
		SpaceObject** bubbles;
		int           numBubbles;
		int           maxBubbles;

	public:
		JyglarStarfarer( Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code );

								 // shoots a shot
		virtual int activate_weapon();
								 // creates a bubble
		virtual int activate_special();
		virtual void calculate();// takes note of bursted bubbles
								 // blue hotspots
		virtual void calculate_hotspots();
		virtual ~JyglarStarfarer();
};

class JyglarShot : public Shot
{
	public:
		// shot that pulls the object hit towards a point before the ship

		double pull;
		SpaceLocation* beacon;	 // desired position of target

	public:
		JyglarShot( SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
			int odamage, double orange, int oarmour, SpaceLocation *opos, SpaceSprite *osprite,
			double opull, SpaceLocation* obeacon, double relativity = game->shot_relativity );

		virtual void inflict_damage( SpaceObject* other );

		virtual ~JyglarShot();	 // frees up beacon memory
};

class JyglarStrayShot : public JyglarShot
{
	public:
		// shot going astray

		int minturn;
		int maxturn;

	public:
		JyglarStrayShot( SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
			int odamage, double orange, int oarmour, SpaceLocation *opos, SpaceSprite *osprite,
			double opull, SpaceLocation* obeacon, double relativity = game->shot_relativity );

		virtual void calculate();// changes direction in a random way
};

class JyglarBubble : public SpaceObject
{
	public:
		// bubbles surrounding the ship

		double dangle;
		double dist;
		int countdown;

	public:
		JyglarBubble( SpaceLocation *creator, double odist, double odangle,
			SpaceSprite *osprite, double omass );

		virtual void calculate();// follows ship movement
								 // bursts on damage with random sound
		virtual int handle_damage( SpaceLocation* other, double normal, double direct );
};

JyglarStarfarer::JyglarStarfarer( Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code ):
Ship( opos, shipAngle, shipData, code )
{
	STACKTRACE;
	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	weaponStray    = (int)(tw_get_config_float("Weapon", "Stray", 0) * 100);
	weaponPull     = scale_velocity(tw_get_config_int("Weapon", "Pull", 0));

	specialMass = tw_get_config_float("Special", "Mass", 1);
	numBubbles  = 0;
	maxBubbles  = tw_get_config_int("Special", "Number", 10);
	bubbles     = new SpaceObject*[maxBubbles];

	melody = 0;
}


int JyglarStarfarer::activate_weapon()
{
	STACKTRACE;
	Shot *shot;
	SpaceLocation* beacon = new SpaceLocation( this,
		pos + unit_vector(angle) * weaponRange / 3, angle );
	if ( tw_random(100) < weaponStray ) {
		game->add( shot = new JyglarStrayShot( this,
			Vector2(-2.0 + 0.5 * (double)(random(5)), size.y * 0.6), angle +(- 2.0 + (double)(random(5)))*ANGLE_RATIO,
			weaponVelocity, random(weaponDamage + 1), random((int)weaponRange + 1),
			weaponArmour, this, data->spriteSpecial, weaponPull, beacon ));
		weapon_sample = 7 + random(3);
	} else {
		int shot_damage = random(weaponDamage + 1);
		game->add( shot = new JyglarShot( this,
			Vector2(-2.0 + 0.5 * (double)(random(5)), size.y * 0.5), angle +(- 2.0 + (double)(random(5)))*ANGLE_RATIO,
			weaponVelocity, shot_damage, random(int(weaponRange + 1)),
			weaponArmour, this, data->spriteSpecial, weaponPull, beacon ));
		if ( shot_damage == 0 ) {
			weapon_sample = 10;
		} else {
			melody += -1 + random(3);
			weapon_sample = abs( melody ) % 7;
		}
	}
	shot->explosionSprite = data->spriteSpecialExplosion;
	shot->explosionFrameCount = 6;
	shot->explosionSample = data->sampleSpecial[1 + random(data->num_special_samples - 1)];
	return TRUE;
}


int JyglarStarfarer::activate_special()
{
	STACKTRACE;
	if ( numBubbles >= maxBubbles )
		return FALSE;

	bubbles[numBubbles] = new JyglarBubble( this, size.x / 5 + random(size.x / 3),
		random(PI2), data->spriteWeapon, specialMass);
	game->add( bubbles[numBubbles] );
	numBubbles++;
	return TRUE;
}


void JyglarStarfarer::calculate()
{
	STACKTRACE;

	int j = 0;
	for( int i = 0; i < numBubbles; i++ ) {
		bubbles[i - j] = bubbles[i];
		if ( !bubbles[i]->exists() )
			j++;
	}
	numBubbles -= j;

	Ship::calculate();
}


void JyglarStarfarer::calculate_hotspots()
{
	STACKTRACE;

	if ( thrust && hotspot_frame <= 0 ) {
		game->add( new Animation( this,
			normal_pos() - unit_vector(angle) * size.y / 4,
			data->spriteExtra, 0, data->spriteExtra->frames(), time_ratio, LAYER_HOTSPOTS));
		hotspot_frame += hotspot_rate;
	}

	if ( hotspot_frame > 0 )
		hotspot_frame -= frame_time;
}


JyglarStarfarer::~JyglarStarfarer()
{
	delete[] bubbles;
}


JyglarShot::JyglarShot( SpaceLocation *creator, Vector2 rpos,
double oangle, double ov, int odamage, double orange, int oarmour,
SpaceLocation *opos, SpaceSprite *osprite, double opull, SpaceLocation* obeacon,
double relativity ):
Shot( creator, rpos, oangle, ov, odamage, orange, oarmour, opos, osprite,
relativity ), pull( opull ), beacon( obeacon )
{
	STACKTRACE;
}


void JyglarShot::inflict_damage( SpaceObject* other )
{
	STACKTRACE;
	if ( other->mass > 0 && !other->isPlanet() ) {
		other->accelerate( this, other->trajectory_angle( beacon ), pull / other->mass, MAX_SPEED );
	}
	Shot::inflict_damage( other );
}


JyglarShot::~JyglarShot()
{
	if ( beacon )
		delete beacon;
}


JyglarStrayShot::JyglarStrayShot( SpaceLocation *creator, Vector2 rpos,
double oangle, double ov, int odamage, double orange, int oarmour,
SpaceLocation *opos, SpaceSprite *osprite, double opull, SpaceLocation* obeacon,
double relativity ):
JyglarShot( creator, rpos, oangle, ov, odamage, orange, oarmour, opos, osprite,
opull, obeacon, relativity )
{
	STACKTRACE;
	//  collide_flag_sameteam = collide_flag_sameship = ALL_LAYERS;
	minturn = -(random(2));
	maxturn = random(2);
}


void JyglarStrayShot::calculate()
{
	STACKTRACE;
	Shot::calculate();
	double r = random(minturn* ANGLE_RATIO, maxturn* ANGLE_RATIO);
	changeDirection( angle + r * frame_time );
}


JyglarBubble::JyglarBubble( SpaceLocation *creator, double odist, double odangle,
SpaceSprite *osprite, double omass ):
SpaceObject( creator,
creator->normal_pos() + odist * unit_vector( (creator->get_angle() + odangle) ),
creator->get_angle() + odangle, osprite )
{
	STACKTRACE;
	layer = LAYER_SPECIAL;
	mass = omass;
	dist = odist;
	dangle = odangle;
	vel = ship->get_vel();
	sprite_index = get_index(angle);
	countdown = 1000 + random(2000);

	isblockingweapons = true;
	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void JyglarBubble::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();
	if ( !ship ) {
		countdown -= frame_time;
		if ( countdown <= 0 ) {
			state = 0;
			play_sound( data->sampleExtra[random(data->num_extra_samples)] );
		}
		return;
	}
	if ( !(ship && ship->exists()) ) {
		ship = 0;
		return;
	}
	angle = ship->get_angle() + dangle;
	pos = ship->normal_pos() + dist * unit_vector(angle);
	vel = ship->get_vel();
	sprite_index = get_index(angle);
}


int JyglarBubble::handle_damage( SpaceLocation* source, double normal, double direct )
{
	STACKTRACE;
	if ( normal + direct ) {
		state = 0;
		play_sound( data->sampleExtra[random(data->num_extra_samples)], 1000 );
	}
	return iround(normal + direct);
}


REGISTER_SHIP(JyglarStarfarer)
