/* $Id: shpdeees.cpp,v 1.5 2004/03/24 23:51:40 yurand Exp $ */
#include "../ship.h"
REGISTER_FILE
#include "../melee/mcbodies.h"
#include "../other/objanim.h"

#define DEEP_SPACE_ANIM_RATE 90
#define DEEP_SPACE_WARRIOR_TRAIL_RATE 25

int makecol(RGB col)
{
	STACKTRACE;
	return makecol(col.r, col.g, col.b);
}


/*
 * created by: cyhawk@sch.bme.hu and forevian@freemail.hu
 */
class DeepSpaceEssence : public Ship
{
	// the ship

	double weaponVelocity;
	double weaponRange;
	double weaponTurnRate;
	int    weaponArmour;
	double weaponDrainRate;
	int    weaponMinCrew;		 // min crew on board to be able to use weapon

	double specialRange;
	int    specialMinCrew;		 // min crew on board to be able to use special
	int    specialEffectCount;

	int    extraFadeRate;
	double extraDrainRate;

	int sprite_step;
	int sprite_phase;
	SpaceLocation* dying;		 // mainly a boolean value but also holds pointer to our killer

	public:
		double residualDamage;

		DeepSpaceEssence( Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code );

								 // releases a DeepSpaceWarrior
		virtual int  activate_weapon();
								 // kills a member of the crew and damages all in range
		virtual int  activate_special();
		int handle_damage(SpaceLocation* source, double normal, double direct = 0);
		// virtual void handle_damage( SpaceLocation* other );
		virtual void destroyed( SpaceLocation* source );
								 // drains crew
		virtual void inflict_damage( SpaceObject* other );
		virtual void calculate();// animates graphics
								 // leaves trail instead
		virtual void calculate_hotspots();
								 //grey
		virtual int  crewPanelColor() {
			return pallete_color[8];
		}
};

class DeepSpaceWarrior : public HomingMissile
{
	// warriors that do damage on contact and pass through matter

	double drain_rate;
	int    trail_step;

	public:
		DeepSpaceWarrior( DeepSpaceEssence* creator, Vector2 opos, double oangle,
			double ov, double orange, int oarmour, double otrate, SpaceLocation* lpos,
			double odrain_rate, SpaceSprite *osprite, SpaceObject* target );

		void inflict_damage(SpaceObject *other);
		virtual void calculate();
};

DeepSpaceEssence::DeepSpaceEssence( Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code ):
Ship( opos, shipAngle, shipData, code )
{
	STACKTRACE;
	weaponVelocity      = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponRange         = scale_range(get_config_float("Weapon", "Range", 0));
	weaponTurnRate      = scale_turning(get_config_float("Weapon", "TurnRate", 0));
	weaponArmour        = get_config_int("Weapon", "Armour", 0);
	weaponDrainRate     = get_config_float("Weapon", "DrainRate", 0);
	weaponMinCrew       = get_config_int("Weapon", "MinCrew", 0);

	specialRange        = scale_range( get_config_float( "Special", "Range", 0 ));
	specialMinCrew      = get_config_int("Special", "MinCrew", 0);
	specialEffectCount  = get_config_int("Special", "EffectCount", 0);

	extraFadeRate       = scale_frames( get_config_int( "Extra", "FadeRate", 0 ));
	extraDrainRate      = get_config_float( "Extra", "DrainRate", 0 );
	residualDamage      = 0;

	set_depth( LAYER_EXPLOSIONS );
	sprite_step = 0;
	sprite_phase = 0;
	dying = NULL;
}


void DeepSpaceEssence::calculate()
{
	STACKTRACE;
	if ( dying ) {
		if ( sprite_step < 0 ) {
			sprite_step += extraFadeRate;
			sprite_phase++;
			if ( sprite_phase == 10 ) {
				sprite_phase = 9;
				state = 0;
				game->ship_died( this, dying );
			}
			sprite_index &= 63;
			sprite_index += sprite_phase * 64;
		} else {
			sprite_step -= frame_time;
		}
		return;
	}
	Ship::calculate();

	if ( sprite_step < 0 ) {
		sprite_step += (int)(DEEP_SPACE_ANIM_RATE * (1.0 + 0.3 * (double)abs( 3 - sprite_phase%7 )));
		sprite_phase++;
		if ( sprite_phase == 14 ) sprite_phase = 0;
	} else {
		sprite_step -= frame_time;
	}
	sprite_index &= 63;
	if ( sprite_phase < 7 )
		sprite_index += sprite_phase * 64;
	else
		sprite_index += (14 - sprite_phase) * 64;
}


//ObjectAnimation::ObjectAnimation( SpaceLocation *creator, Vector2 opos,
//  Vector2 ovel, double oangle, SpaceSprite *osprite, int first_frame,
//  int num_frames, int frame_size, double depth ):
void DeepSpaceEssence::calculate_hotspots()
{
	STACKTRACE;
	if ((thrust) && (hotspot_frame <= 0)) {

		game->add( new ObjectAnimation( this, pos, 0, angle,
			data->spriteSpecial, 0, 10, time_ratio / 2, LAYER_HOTSPOTS ));

		hotspot_frame += hotspot_rate;
	}
	if (hotspot_frame > 0) hotspot_frame -= frame_time;
}


int DeepSpaceEssence::activate_weapon()
{
	STACKTRACE;
	if ( crew < weaponMinCrew ) return FALSE;

	crew--;
	game->add( new DeepSpaceWarrior( this, Vector2(0.0, -size.y * 0.8),
		(120.0 + (double)(random() % 120))*PI/180 + angle, weaponVelocity, weaponRange,
		weaponArmour, weaponTurnRate, this, weaponDrainRate, data->spriteWeapon, target ));

	return TRUE;
}


int DeepSpaceEssence::activate_special()
{
	STACKTRACE;
	if ( crew <= specialMinCrew ) return FALSE;

	handle_damage(this, 1, 0);

	Query q;
	for( q.begin( this, OBJECT_LAYERS, specialRange ); q.currento; q.next() ) {
		if ( q.currento->isShip() && q.currento != this ) {
			Ship* s = (Ship*)q.currento;
								 //palette_color[2] ){
			if ( s->crewPanelColor().g >= 225) {
				s->handle_damage(this, 1, 0);
			}
		}
	}
	// effect
	for( int i=0; i < specialEffectCount; i++ ) {
		double rnd = random(1.0);//0.01 * (double)(rand() % 100);
		double e_range = specialRange * (1.0 - rnd*rnd);
								 //(double)(rand() % 360);
		double e_angle = random(PI2);
		double e_speed = speed_max / 3;
		game->add( new ObjectAnimation( this,

			pos + e_range*unit_vector(e_angle),
		///			x + e_range * cos( e_angle * ANGLE_RATIO ),
		//			y + e_range * sin( e_angle * ANGLE_RATIO ),

			-e_speed * unit_vector(e_angle),
		//			- e_speed * cos( e_angle * ANGLE_RATIO ),
		//			- e_speed * sin( e_angle * ANGLE_RATIO ),

			e_angle, data->spriteExtra, 0, 4, time_ratio * 2, LAYER_HOTSPOTS ));
	}
	play_sound2( data->sampleSpecial[0] );
	return TRUE;
}


void DeepSpaceEssence::inflict_damage( SpaceObject* other )
{
	STACKTRACE;
	Ship::inflict_damage( other );
	if ( other->isShip() ) {
		Ship* s = (Ship*)other;
		if ( s->crewPanelColor().g >= 225 ) {
			double d = 0;

			int drainDamage = (int)(extraDrainRate * frame_time);
			residualDamage += extraDrainRate * (double)frame_time - drainDamage;
			drainDamage += (int)residualDamage;
			residualDamage -= (int)residualDamage;
			if ( s->getCrew() - drainDamage < 0 ) drainDamage = iround(s->getCrew());
			d = drainDamage;
			drainDamage = iround(s->getCrew());
								 // so, s loses crew ...
			s->handle_damage( this, d );
			drainDamage -= iround(s->getCrew());
			//repair += drainDamage;
			handle_damage( this, -drainDamage );
			if ( drainDamage ) play_sound2( data->sampleExtra[0] );
		}
								 //>isShot() )
	}else if ( !other->isblockingweapons)
	{
		// mostly against DOGI
		int drainDamage = (int)(extraDrainRate * frame_time);
		residualDamage += extraDrainRate * (double)frame_time - drainDamage;
		drainDamage += (int)residualDamage;
		residualDamage -= (int)residualDamage;
		//other->damage += drainDamage;
		other->handle_damage( this, drainDamage );

	}
}


int DeepSpaceEssence::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;

	crew -= normal;

	if (crew > crew_max) crew = crew_max;

	if ( state <= 0 ) crew = 0;	 // planet kills with state=0 if mass==0

	if (crew <= 0) {
		crew  = 0;
		state = 0;

		destroyed( source );
	}
	// above could be done in Ship::handle_damage()
	if ( dying && crew > 0 ) {
		dying = NULL;
		sprite = data->spriteShip;
		sprite_phase = 0;
		sprite_index &= 64;
	}

	return 0;
}


void DeepSpaceEssence::destroyed( SpaceLocation* source )
{
	STACKTRACE;
	if ( !dying ) {
		sprite_phase = 0;
		sprite = data->spriteSpecial;
	}
	state = 1;
	dying = source;
}


DeepSpaceWarrior::DeepSpaceWarrior( DeepSpaceEssence* creator, Vector2 opos,
double oangle, double ov, double orange, int oarmour, double otrate,
SpaceLocation* lpos, double odrain_rate, SpaceSprite *osprite, SpaceObject* target ):
HomingMissile( creator, opos, oangle, ov, 0, orange, oarmour, otrate, lpos,
osprite, target ),
drain_rate( odrain_rate ),
trail_step( DEEP_SPACE_WARRIOR_TRAIL_RATE )
{
	STACKTRACE;
	collide_flag_sameship = bit(LAYER_SHIPS);
}


void DeepSpaceWarrior::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if ( !ship || !ship->exists() ) return;
	if ( other == ship ) {
		state = 0;
		//ship->repair++;
		ship->handle_damage( this, -1 );
		return;
	}
	if ( other->isShip() ) {
		Ship* victim = (Ship*)other;
		DeepSpaceEssence* mother = (DeepSpaceEssence*)ship;

		int drainDamage = (int)(drain_rate * frame_time);
		mother->residualDamage += drain_rate * (double)frame_time - drainDamage;
		drainDamage += (int)mother->residualDamage;
		mother->residualDamage -= (int)mother->residualDamage;

		//victim->damage += drainDamage;
		victim->handle_damage( this, drainDamage );
		play_sound( data->sampleWeapon[1 + random() % 4] );
		Query q;
		int c = 0;
		if (range > d) {
			for( q.begin( this, bit(LAYER_SHIPS), range - d ); q.currento; q.next() ) {
				if ( !q.currento->isShip() ) continue;
				if ( q.currento->target != ship ) continue;
				c++;
			}
		}
		if ( !c ) return;
		c = random() % c;
		for( q.begin( this, bit(LAYER_SHIPS), range - d ); q.currento; q.next() ) {
			if ( !q.currento->isShip() ) continue;
			if ( q.currento->target != ship ) continue;
			if ( !c ) break;
			c--;
		}
		target = q.currento;
		q.end();
	}
}


void DeepSpaceWarrior::calculate()
{
	STACKTRACE;
	if ( !exists() ) return;
	if ( d < range ) {
		if ( !target ) target = ship;
	} else {
		target = ship;
	}
	HomingMissile::calculate();
	if ( !ship ) state = 0;
	else        state = 1;

	if ( trail_step <= 0 ) {
		game->add( new ObjectAnimation( this, pos, 0, angle, sprite, 1, 4, time_ratio, LAYER_HOTSPOTS ));
		trail_step += DEEP_SPACE_WARRIOR_TRAIL_RATE;
	}else
	trail_step -= frame_time;
}


REGISTER_SHIP(DeepSpaceEssence)
