/* $Id: shpmoisp.cpp,v 1.11 2004/03/24 23:51:42 yurand Exp $ */

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class MoianSpeeder : public Ship
{
	double  weaponRange, weaponVelocity, weaponDamage, weaponArmour;
	double  specialRange, specialVelocity, specialDamage, specialArmour, specialTurnRate;
	double  specialN, speedFraction;
	int     specialT;
	double  bloblifetime, blobrange;

	public:
		MoianSpeeder(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

	protected:

		virtual void calculate();

		virtual int activate_weapon();
		virtual int activate_special();

		//virtual int handle_damage(SpaceLocation* source, double normal, double direct);
};

class SpeedMissile : public HomingMissile
{
	SpaceObject *blobreleaser;
	SpaceSprite *blobsprite;
	double  Trelease, Tnextreleasetime, releaseinterval;
	int     Nblobs;
	double  blobspeedFraction, bloblifetime, blobrange;

	public:
		SpeedMissile(SpaceLocation *creator, Vector2 rpos,
			double oangle, double ov, double odamage, double orange, double oarmour,
			double otrate, SpaceLocation *opos, SpaceSprite *osprite, SpaceObject *otarget,
			double oTrelease, int oNblobs, SpaceSprite *oblobsprite,
			double oblobspeedFraction, double obloblifetime, double oblobrange);

		virtual void calculate();

		// changes state from a missile to a blob-releaser
		virtual void inflict_damage(SpaceObject* other);

		virtual void animate(Frame *f);
};

class SpeedBlob : public SpaceObject
{
	Vector2 accelvel;
	double  blobtime, blobrange;
	public:
		SpeedBlob(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite,
			Vector2 oaccelvel, double olifetime, double orange);

		virtual void inflict_damage(SpaceObject* other);
		virtual void calculate();
};

MoianSpeeder::MoianSpeeder(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);

	specialRange    = scale_range(get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialDamage   = get_config_int("Special", "Damage", 0);
	specialArmour   = get_config_int("Special", "Armour", 0);
	specialTurnRate = scale_turning(get_config_float("Special", "TurnRate", 0));

	// number of blobs
	specialN   = get_config_int("Special", "N", 0);
	// releases the N blobs during this period
	specialT   = get_config_int("Special", "T", 0);

	speedFraction = get_config_float("Speedblob", "speedfraction", 0);
	bloblifetime = get_config_float("Speedblob", "lifetime", 0);
	blobrange = scale_range(get_config_float("Speedblob", "Range", 0));
}


int MoianSpeeder::activate_weapon()
{
	STACKTRACE;

	add(new Shot(this, Vector2(0,40), angle,
		weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon, 0.0) );

	return TRUE;
}


int MoianSpeeder::activate_special()
{
	STACKTRACE;
	add( new SpeedMissile(this, Vector2(0,40), angle,
		specialVelocity, specialDamage, specialRange, specialArmour, specialTurnRate,
		this, data->spriteSpecial, target,
		specialT, iround(specialN), data->spriteSpecialExplosion,
		speedFraction, bloblifetime, blobrange
		)
		);

	return TRUE;
}


void MoianSpeeder::calculate()
{
	STACKTRACE;
	Ship::calculate();

	sprite_index = get_sprite_index();
}


SpeedMissile::SpeedMissile(SpaceLocation *creator, Vector2 rpos,
double oangle, double ov, double odamage, double orange, double oarmour,
double otrate, SpaceLocation *opos, SpaceSprite *osprite, SpaceObject *otarget,
double oTrelease, int oNblobs, SpaceSprite *oblobsprite,
double oblobspeedFraction, double obloblifetime, double oblobrange)
:
HomingMissile(creator, rpos, oangle, ov, odamage, orange, oarmour, otrate,
opos, osprite, otarget)
{
	STACKTRACE;
	blobsprite = oblobsprite;
	Trelease = oTrelease;
	Nblobs = oNblobs;
	blobspeedFraction = oblobspeedFraction;
	blobrange = oblobrange;

	Tnextreleasetime = Trelease;
	releaseinterval = Trelease / Nblobs;
	blobreleaser = 0;
	bloblifetime = obloblifetime;
}


void SpeedMissile::calculate()
{
	STACKTRACE;
	// if the mothership dies, you lose the sprites...
	if (!(ship && ship->exists())) {
		ship = 0;
		ship = 0;				 // done explicitly, cause the default calculate isn't always called
		state = 0;
		return;
	}

	// all N blobs were released
	if (Trelease < 0)
		state = 0;

	// if the blobreleaser was set, and doesn't exist anymore, then make sure no
	// new blobs are released
	if ( blobreleaser && !blobreleaser->exists() ) {
		blobreleaser = 0;
		state = 0;
	}

	if (!state)
		return;

	if (blobreleaser) {
		Trelease -= frame_time * 1E-3;

		if (Trelease < Tnextreleasetime) {
			// release a new speedblob
			Tnextreleasetime -= releaseinterval;

			Vector2 dv, P;

								 // per second
			dv = blobspeedFraction * blobreleaser->vel / 1000;

			P = blobreleaser->pos + blobreleaser->get_sprite()->size(0).y *
				unit_vector(blobreleaser->vel.atan() + PI + random(2.0)-1.0);

			add(
				new SpeedBlob(this, P, 0.0,
				blobsprite, dv, bloblifetime, blobrange)
				);
		}

	}
	else
		HomingMissile::calculate();
}


// changes state from a missile to a blob-releaser
void SpeedMissile::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	blobreleaser = other;
	HomingMissile::inflict_damage(other);
	state = 1;

	collide_flag_anyone = 0;
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;
	mass = 0;

}


void SpeedMissile::animate(Frame *f)
{
	STACKTRACE;
	if (!blobreleaser)
		HomingMissile::animate(f);
}


SpeedBlob::SpeedBlob(SpaceLocation *creator, Vector2 opos, double oangle,
SpaceSprite *osprite, Vector2 ovel, double olifetime, double orange)
:
SpaceObject(creator, opos, oangle, osprite)
{
	STACKTRACE;
	accelvel = ovel;
	blobrange = orange;

	layer = LAYER_SPECIAL;
	mass = 0;

	blobtime = olifetime;
	blobrange = orange;

	collide_flag_anyone = 0;
	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;

	attributes |= ATTRIB_UNDETECTABLE;
}


void SpeedBlob::inflict_damage(SpaceObject* other)
{
	STACKTRACE;

	//	if ( other && other->exists() )
	//	{
	//		other->vel += accelvel;		// deliver the speed boost
	//		state = 0;					// and die
	//	}
}


void SpeedBlob::calculate()
{
	STACKTRACE;
	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
		return;
	}

	if (blobtime < 0)
		state = 0;

	if (!state)
		return;

	blobtime -= frame_time * 1E-3;

	// easiest, check if your ship is close ...
	double R;
	R = distance(ship);
	if (R < blobrange) {
								 // deliver the speed boost
		ship->vel += accelvel * frame_time;
	}

	SpaceObject::calculate();
}


REGISTER_SHIP(MoianSpeeder)
