/* $Id: shpjurcu.cpp,v 1.5 2004/03/26 17:55:49 geomannl Exp $ */

#include "../ship.h"

REGISTER_FILE

class JurgathaCutter: public Ship
{
	int batt_counter;

	double weaponRange;
	double weaponSpeed;
	int weaponDamage;
	int weaponColor;

	int specialDamage;
	int specialFrames;

	public:
		JurgathaCutter(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
	protected:
		virtual void calculate();
		virtual int activate_weapon();
		virtual int activate_special();
};

class JurgathaPortal : public Animation
{
	public:
		JurgathaPortal(SpaceLocation *creator, Vector2 opos, int damage, SpaceSprite *osprite, int ofct, int ofsz);
		virtual void calculate();
};

JurgathaCutter::JurgathaCutter(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	batt_counter = 0;

	weaponRange     = scale_range(get_config_float("Weapon", "Range", 0));
	weaponSpeed     = get_config_float("Weapon", "Speed", 1);
	weaponDamage    = get_config_int("Weapon", "Damage", 0);
	weaponColor     = get_config_int("Weapon", "Color", 2);

	specialDamage   = get_config_int("Special", "Damage", 0);
	specialFrames   = scale_frames(get_config_int("Special", "Frames", 0));
}


void JurgathaCutter::calculate()
{
	STACKTRACE;
	if (thrust) {
		if (batt_counter==3) {
			batt_counter = 0;
			batt>batt_max?batt=batt_max:batt++;
		}
		else batt_counter++;
	}
	else if (!thrust) {
		if (batt_counter>=2) {
			batt_counter = 0;
			batt<0?batt=0:batt--;
		}
		else batt_counter++;
	}

	Ship::calculate();
}


int JurgathaCutter::activate_weapon()
{
	STACKTRACE;
	int random_number = random()%10;

	game->add(new Laser(this, angle, pallete_color[weaponColor], weaponRange,
		weaponDamage, weapon_rate, this, Vector2(-(double)(random_number), 2.0), true));

	random_number = random()%10;

	game->add(new Laser(this, angle, pallete_color[weaponColor], weaponRange,
		weaponDamage, weapon_rate, this, Vector2((double)(random_number), 2.0), true));

	return(TRUE);
}


int JurgathaCutter::activate_special()
{
	STACKTRACE;
	game->add(new JurgathaPortal(this, pos, specialDamage, data->spriteSpecial,
		data->spriteSpecial->frames(), specialFrames));

	return(TRUE);
}


JurgathaPortal::JurgathaPortal(SpaceLocation *creator, Vector2 opos, int damage, SpaceSprite *osprite, int ofct, int ofsz) :
Animation(creator, opos, osprite, 0, ofct, ofsz, LAYER_HOTSPOTS)
{
	STACKTRACE;
	return;
}


void JurgathaPortal::calculate()
{
	STACKTRACE;
	Animation::calculate();

	Query a;

	for (a.begin(this, bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL),
	sprite->width()/2); a.current; a.next()) {
		if (!a.currento->sameTeam(this) && !(a.currento->isAsteroid() || a.currento->isPlanet() ) ) {
			//a.currento->directDamage++;
			a.currento->handle_damage(this, 0, 1);
		}
	}
}


REGISTER_SHIP(JurgathaCutter)
