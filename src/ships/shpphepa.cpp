/* $Id: shpphepa.cpp,v 1.18 2004/03/24 23:51:42 yurand Exp $ */
#include "../ship.h"
REGISTER_FILE

static const int maxcrew = 50;

class CrewPodPP;

class PhedarPatrolShip : public Ship
{
	public:

		int     weaponColor;
		double  weaponRange;
		int     weaponFrames;
		int     weaponDamage;

		double  specialVelocity1, specialVelocity2;
		int     specialFrames;

		double  energizetimer, energizetimemax;

		int     energizepersonalarmour;

								 // to keep track who's in space.
		CrewPodPP   *crewinspace[maxcrew];

		PhedarPatrolShip(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual void calculate();
		virtual Color crewPanelColor(int k = 0);
		virtual int activate_weapon();
		virtual int activate_special();
		virtual int handle_damage(SpaceLocation *src, double normal, double direct=0);
};

// THE FOLLOWING IS COPIED FROM THE SYREEN PENETRATOR CODE :

class CrewPodPP : public SpaceObject
{
	public:
		int frame_count;
		int frame_size;
		int frame_step;

		//	double velocity;
		int    life;
		int    lifetime;

		PhedarPatrolShip    *mother;

		CrewPodPP(Vector2 opos, int oLifeTime,
			PhedarPatrolShip *oship, SpaceSprite *osprite, int ofcount, int ofsize);

		virtual void calculate();
		virtual int sameTeam(SpaceLocation *other);

		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);
};

// THE FOLLOWING IS "NEW" :

PhedarPatrolShip::PhedarPatrolShip(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	weaponColor  = get_config_int("Weapon", "Color", 0);
	weaponFrames = get_config_int("Weapon", "Frames", 0);
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	energizepersonalarmour = 0;

	specialVelocity1 = scale_velocity(get_config_float("Special", "Velocity1", 0));
	specialVelocity2 = scale_velocity(get_config_float("Special", "Velocity2", 0));
	specialFrames   = get_config_int("Special", "Frames", 0);
	energizetimemax = get_config_float("Special", "Timer", 0) * 1000.0;
	energizetimer = 0;

	int i;
	for ( i = 0; i < maxcrew; ++i )
		crewinspace[i] = 0;
}


Color PhedarPatrolShip::crewPanelColor(int k)
{
	STACKTRACE;
	// change the crew color, if needed
	if ( energizepersonalarmour ) {
		Color c = {255,200,200};
		return c;
	} else {
		return Ship::crewPanelColor(k);
	}
}


int PhedarPatrolShip::activate_weapon()
{
	STACKTRACE;

	game->add(new Laser(this, get_angle(), pallete_color[weaponColor],
								 // synching=true
		weaponRange, weaponDamage, weaponFrames, this, Vector2(0, 0.5*get_size().y), true ));
	return TRUE;
}


int PhedarPatrolShip::activate_special()
{
	STACKTRACE;
	energizepersonalarmour = TRUE;
	energizetimer = energizetimemax;

	update_panel = 1;			 // update the panel!

	return TRUE;
}


void PhedarPatrolShip::calculate()
{
	STACKTRACE;

	Ship::calculate();

	if ( energizepersonalarmour ) {
		if ( energizetimer > 0 ) {
			energizetimer -= frame_time;
		} else {
			energizetimer = 0;
			energizepersonalarmour = 0;
			update_panel = 1;
		}

	}
}


int PhedarPatrolShip::handle_damage(SpaceLocation *src, double normal, double direct)
{
	STACKTRACE;

	double totdam = normal + direct;
	// you also have to deal with negative damage, i.e., crew increase - that's
	// especially important for this ship !!

	if ( totdam < 0 ) {
		crew -= totdam;

		if ( crew > crew_max )
			crew = crew_max;

		return iround(totdam);
	}

	if ( energizepersonalarmour ) {
		double evac = 0;
								 // yeah, the last one may also leave !!
		while ( evac < totdam && crew >= 1 ) {
								 // otherwise, it's hard to die ;)

			int i = int(crew+0.5);
			if ( i < 0 || i > maxcrew-1 )
				{tw_error("Too many crew in this ship !!"); }

				crewinspace[i] = new CrewPodPP(
				this->normal_pos() + (unit_vector(trajectory_angle(this) - PI) *
				this->size.x) + random(Vector2(50,50)) - Vector2(25,25),
				specialFrames, this, data->spriteSpecial, 32,
				specialFrames);

			add(crewinspace[i]);

			-- crew;

			++evac;
		}

	} else {
		Ship::handle_damage(src, normal, direct);
	}

	// don't use == 0, but use < 1, cause of possible partial damage ? I assume, 0.8 crew is also dead crew...
	if ( crew < 1 ) {
		crew = 0;
								 // use the "default" die procedure
		Ship::handle_damage(src, 0, 0);
	}

	return iround(totdam);
}


// THE FOLLOWING IS COPIED FROM THE SYREEN PENETRATOR CODE :

CrewPodPP::CrewPodPP(Vector2 opos, int oLifeTime,
PhedarPatrolShip *oship, SpaceSprite *osprite, int ofcount, int ofsize)
:
SpaceObject(oship, opos, 0.0, osprite),
frame_count(ofcount),
frame_size(ofsize),
frame_step(0),
life(0),
lifetime(oLifeTime),
mother(oship)
{
	STACKTRACE;
	collide_flag_sameship = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);

	isblockingweapons = false;
}


int CrewPodPP::sameTeam(SpaceLocation *other)
{
	STACKTRACE;
	return true;
}


void CrewPodPP::calculate()
{
	STACKTRACE;
	frame_step += frame_time;
	while (frame_step >= frame_size) {
		frame_step -= frame_size;
		sprite_index++;
		if (sprite_index == frame_count) sprite_index = 0;
	}

	life += frame_time;
	if (life >= lifetime) {
		state = 0;
		return;
	}

	if (mother && mother->exists()) {
		if ( mother->energizepersonalarmour )
			// also, when armour is activated, all crewmen in space are attracted at
			// higher than usual velocity towards the ship (like a vacuum cleaner)
			vel = unit_vector(trajectory_angle(mother)) * mother->specialVelocity2;
		else
			vel = unit_vector(trajectory_angle(mother)) * mother->specialVelocity1;
	} else {
		mother = 0;
		state = 0;
	}

	SpaceObject::calculate();

}


void CrewPodPP::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other->isShip() && other->damage_factor == 0) {
		//		sound.stop(data->sampleExtra[0]);
		//		sound.play(data->sampleExtra[0]);
		damage(other, 0, -1);
		state = 0;
	}
}


int CrewPodPP::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	state = 0;					 // this is extra; eg. if hit by a asteroid or so.
	return 0;
}


REGISTER_SHIP(PhedarPatrolShip)
