/* $Id: shpxxxas.cpp,v 1.13 2004/03/24 23:51:43 yurand Exp $ */
#include "../ship.h"

//#include "../melee/mview.h"

REGISTER_FILE

class XXXAssimilator : public Ship
{
	int          weaponFrames;
	int          weaponDamage;
	int          orig_Damage;
	int                  orig_WeaponDrain;
	double           orig_Speed;
	double       orig_Acceleration;
	int                  orig_HotspotRate;
	double           orig_TurnRate;
	BITMAP           *orig_bmp;

	double           weaponRange;
	double           weaponVelocity;
	int                  weaponArmour;

	bool         AssimilateReset;
	double           TheSpotAngle;
	int                  TheSpotDelay;
	int                  TheSpotDelayTimer;
	int                  TheSpotAngleDelta;
	int                  TheSpotRadius;
	int                  RelX;
	int                  RelY;

	int          BodyFrames;
	int          CurrBodyFrame;
	int          ToggleDirection;

	public:

		XXXAssimilator(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		Color crewPanelColor(int k = 0);

		virtual int activate_weapon();
		virtual int activate_special();

		virtual void calculate();

		virtual void collide(SpaceObject *other);

		virtual void animate(Frame *space);

		virtual int handle_damage(SpaceLocation *source, double normal, double direct);

		virtual void calculate_hotspots();

		virtual void reload_panel();

};

XXXAssimilator::XXXAssimilator(Vector2 opos, double shipAngle,

ShipData *shipData, unsigned int code)

:

Ship(opos, shipAngle, shipData, code)

{
	STACKTRACE;

	weaponFrames    = get_config_int("Weapon", "Frames", 0);

	weaponRange           = scale_range(get_config_float("Weapon", "Range", 0));

	weaponDamage      = get_config_int("Weapon", "Damage", 0);

	weaponVelocity  = scale_velocity(get_config_float("Weapon", "Velocity", 0));

	weaponArmour      = get_config_int("Weapon", "Armour", 0);

	orig_Damage             = weaponDamage;

	orig_WeaponDrain    = weapon_drain;

	orig_Speed        = speed_max;

	orig_Acceleration = accel_rate;

	orig_HotspotRate    = hotspot_rate;

	orig_TurnRate           = turn_rate;

	orig_bmp          = shipData->spritePanel->get_bitmap(0);

	TheSpotDelay             = get_config_int("Special", "TheSpotDelay", 0);

	TheSpotRadius            = get_config_int("Special", "TheSpotRadius", 0);

	TheSpotAngle             = 0;

	TheSpotDelayTimer  = 0;

	TheSpotAngleDelta  = get_config_int("Special", "TheSpotAngleDelta", 0);

	RelX = int(cos(-TheSpotAngle+PI/2) * TheSpotRadius);

	RelY = int(sin(-TheSpotAngle+PI/2) * TheSpotRadius);

	BodyFrames                 = weaponFrames;

	CurrBodyFrame          = 0;

	ToggleDirection        = FALSE;

}


Color XXXAssimilator::crewPanelColor(int k)

{
	STACKTRACE;

	Color c = {64,64,64};

	return c;

}


int XXXAssimilator::activate_weapon()

{
	STACKTRACE;

	double ShotAngle = normalize(TheSpotAngle + get_angle(),PI2);

	add(new AnimatedShot(this, Vector2(RelX, RelY),

		ShotAngle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,

		this, data->spriteWeapon,4,4,0));

	return(TRUE);

}


int XXXAssimilator::activate_special()

{
	STACKTRACE;

	weaponDamage = orig_Damage;

	weapon_drain = orig_WeaponDrain;

	speed_max    = orig_Speed;

	accel_rate   = orig_Acceleration;

	hotspot_rate = orig_HotspotRate;

	turn_rate    = orig_TurnRate;

	reload_panel();

	return(TRUE);

}


void XXXAssimilator::reload_panel()

{
	STACKTRACE;

	blit(orig_bmp,spritePanel->get_bitmap(0), 0, 0, 0, 0, 64, 100);

	ship->update_panel = TRUE;

	return;

}


void XXXAssimilator::calculate()

{
	STACKTRACE;

	Ship::calculate();

	TheSpotDelayTimer++;

	if (TheSpotDelayTimer > TheSpotDelay) {

		TheSpotDelayTimer   = 0;

		TheSpotAngle                += TheSpotAngleDelta;

		TheSpotAngle        = normalize(TheSpotAngle,PI2);

		RelX = iround(cos(-TheSpotAngle+PI/2) * TheSpotRadius);

		RelY = iround(sin(-TheSpotAngle+PI/2) * TheSpotRadius);

	}

	add(new PositionedAnimation(this, this, Vector2(RelX, RelY),

		data->spriteExtra, 0, 1, 25, LAYER_EXPLOSIONS));

}


void XXXAssimilator::collide(SpaceObject *other)

{
	STACKTRACE;

	Ship::collide(other);

	if (other->isShip()) {

		if (other->ship->speed_max > speed_max) {

			speed_max = other->ship->speed_max;

			reload_panel();

			play_sound(data->sampleExtra[0]);

		}

		if (other->ship->accel_rate  > accel_rate) {

			accel_rate     = other->ship->accel_rate;

			hotspot_rate = other->ship->hotspot_rate;

			reload_panel();

			play_sound(data->sampleExtra[0]);

		}

	}

}


void XXXAssimilator::animate(Frame *space)

{
	STACKTRACE;

	BodyFrames -= frame_time * 5;

	if (BodyFrames <= 0) {

		if (!( (CurrBodyFrame > 0) && (CurrBodyFrame < 2) ))

			ToggleDirection = !ToggleDirection;

		if (ToggleDirection) ++CurrBodyFrame;

		else --CurrBodyFrame;

		BodyFrames = weaponFrames;

	}

	sprite->animate(pos, sprite_index  + (64 * CurrBodyFrame) , space);

}


int XXXAssimilator::handle_damage(SpaceLocation *source, double normal, double direct)

{
	STACKTRACE;

	if (source->damage_factor > weaponDamage) {

		weaponDamage = iround(source->damage_factor);

		weapon_drain = weaponDamage;

		play_sound(data->sampleExtra[0]);

	}

	return Ship::handle_damage(source, normal, direct);

}


void XXXAssimilator::calculate_hotspots()

{
	STACKTRACE;

	if ((thrust) && (hotspot_frame <= 0)) {

		add(new Animation(this,

			normal_pos() - 11*unit_vector(angle),

			meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, LAYER_HOTSPOTS));

		add(new PositionedAnimation(this, this, Vector2(0, -10),

			data->spriteExtraExplosion ,0, random()%2, 5, LAYER_SHOTS));

		hotspot_frame += hotspot_rate;

	}

	if (hotspot_frame > 0) hotspot_frame -= frame_time;

}


REGISTER_SHIP(XXXAssimilator)
