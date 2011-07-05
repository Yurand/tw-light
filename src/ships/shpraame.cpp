/* $Id: shpraame.cpp,v 1.22 2006/06/05 19:03:44 geomannl Exp $ */
#include "../ship.h"
#include "../util/aastr.h"

#define RAALRITH_POISON_ID     0x2233

class RaalrithMenacer : public Ship
{
	public:

		int          weaponFrames;
		int          drillFrames;

		int          suckRate;
		int          suckStart;

		int          latchOnFrames;
		int          latchOnTimer;

		int          kickDamage;

		Ship         *grabbed;
		double       grabangle;
		double       grabdistance;
		double       grabshipangle;

		int          phaseFrames;
		int          phaseTimer;
		int          phase;
		int          phaseIn;
		int          phaseOld;

		int          lock;
		int          lockFrames;
		int          lockTimer;

	public:
		RaalrithMenacer(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		int          latched;
		int          poisonDuration;

		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual int canCollide(SpaceObject *other);
		virtual void animate(Frame *space);
		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual Color crewPanelColor(int k = 0);

};

class RaalrithPoison : public SpaceObject
{
	public:
	public:

		int duration;
		int start;
		Ship *oship;
		RaalrithMenacer *owner;

		RaalrithPoison(RaalrithMenacer *creator, int duration, Ship *kill_it, SpaceSprite *osprite);
		virtual void calculate();
		virtual void animate(Frame *space);
};

RaalrithMenacer::RaalrithMenacer(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)

{
	STACKTRACE;

	weaponFrames   = tw_get_config_int("Weapon", "Frames", 0);
	drillFrames    = 0;

	suckRate       = tw_get_config_int("Weapon", "SuckRate", 0);
	suckStart      = TRUE;

	latchOnFrames  = tw_get_config_int("Weapon", "LatchFrames", 0);
	latchOnTimer   = 0;

	kickDamage     = tw_get_config_int("Weapon", "KickDamage", 0);

	latched        = FALSE;
	grabbed        = NULL;

	lockFrames     = tw_get_config_int("Special", "LockFrames", 0);
	lock           = 0;
	lockTimer      = 0;

	phaseFrames    = tw_get_config_int("Extra", "PhaseMax", 0);
	phaseTimer     = phaseFrames;
	phase          = 100;
	phaseIn        = FALSE;
	phaseOld       = 0;

	poisonDuration = tw_get_config_int("Extra", "PoisonDuration", 0);

}


int RaalrithMenacer::activate_weapon()
{
	STACKTRACE;
	if (drillFrames <= 0 && !(latched)) {
		drillFrames = weaponFrames;
		return(TRUE);
	}
	if ((latched) && (grabbed != NULL) && (latchOnTimer <= 0)) {
		damage(grabbed, kickDamage);
		latched = FALSE;
		suckStart = TRUE;
		accelerate (this, angle + PI, scale_velocity(40.0), MAX_SPEED);
		target->accelerate (this, angle, scale_velocity(20.0), MAX_SPEED);
		return(TRUE);
	}
	return(FALSE);
}


int RaalrithMenacer::activate_special()
{
	STACKTRACE;
	if (lockTimer <= 0) {
		lockTimer = lockFrames;
		lock = phase;
		return(TRUE);
	}
	return(FALSE);
}


void RaalrithMenacer::calculate()
{
	STACKTRACE;

	if (drillFrames > 0) {
		drillFrames-= frame_time;
		if ((drillFrames <= 0) && (!latched)) {
			sound.stop(data->sampleWeapon[0]);
			sound.play(data->sampleWeapon[0]);
		}
	}

	if (latched) {
		if (suckStart) {
			suckStart = FALSE;
			latchOnTimer = latchOnFrames;
			add( new RaalrithPoison(this, poisonDuration, (Ship*)grabbed, data->spriteWeapon));
		}
		if (latchOnTimer > 0)
			latchOnTimer -= frame_time;
	}

	phaseTimer -= frame_time;
	if (phaseTimer < 0) {
		if (phaseIn)
			phase++;
		else
			phase--;
		phaseTimer = phaseFrames;
	}
	if (phase <= 0) {
		phase = 0;
		phaseIn = TRUE;
	}
	if (phase >= 100) {
		phase = 100;
		phaseIn = FALSE;
	}

	// crew turns grey if phase < 30
	if ( phase == 30 || phase == 31 ) {
		update_panel = 1;
	}

	if (int((double(phase)/100)*30) != int((double(phaseOld)/100)*30)) {
		BITMAP *bmp = spritePanel->get_bitmap(0);
		rectfill(bmp, 15, 14, 48, 15, pallete_color[0]);
		rectfill(bmp, 15, 14, int(14 + (double(phase)/100)*30), 15, pallete_color[9]);
		phaseOld = phase;
		ship->update_panel = TRUE;
	}

	if (grabbed != NULL)
	if (!(grabbed ->exists())) {
		latched = FALSE;
		latchOnTimer = -50;
		grabbed = NULL;
	}
	if (latched) {
		if ((tw_random(suckRate)) <= grabbed->crew) {
			crew +=1;
			damage(grabbed, 1);
		}
		grabangle = (grabbed->get_angle() - grabshipangle) + grabangle;
		angle=grabangle;
		grabshipangle = grabbed->get_angle();
		//nextkeys &= ~(keyflag::left | keyflag::right | keyflag::thrust);
		if (control)
			control->keys &= ~(keyflag::left | keyflag::right | keyflag::thrust);

		//     x = grabbed->normal_x()-((cos(grabangle )) * grabdistance);
		//     y = grabbed->normal_y()-((sin(grabangle )) * grabdistance);
		pos = grabbed->normal_pos() - grabdistance * unit_vector(grabangle);
	}

	if (lockTimer > 0) {
		lockTimer -= frame_time;
		phase = lock;
	}

	if (phase <= 10)
		collide_flag_anyone=0;
	else
		collide_flag_anyone=ALL_LAYERS;

	Ship::calculate();
}


int RaalrithMenacer::canCollide(SpaceObject *other)
{
	STACKTRACE;
	if ((latched) && (grabbed!=NULL) && (grabbed->exists())) {
		if (grabbed == other)
			return (FALSE);
	}
	return (Ship::canCollide(other));
}


void RaalrithMenacer::animate(Frame *space)
{
	STACKTRACE;
	int a;
	// 0 = not transparent; 255 = fully transparent
	a = aa_get_trans();
								 // not, phase = 100 means its solid
	aa_set_trans(iround(0 + (150 * (100-phase)) / 100.0));
	//	Ship::animate(space);

	if (drillFrames > 0)
		data->spriteWeapon->animate( pos, sprite_index, space);
	else
		sprite->animate( pos, sprite_index, space);
	if (latched)
		data->spriteWeapon->animate( pos, sprite_index, space);

	aa_set_trans(a);
}


void RaalrithMenacer::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (drillFrames > 0)
		if (!latched)
		if ((!(sameTeam(other))) && (other->isShip())) {
			latched=TRUE;
		grabbed= (Ship *) other;
		grabangle= (trajectory_angle(other) );
		grabdistance = (distance(other) * 1.1);
		grabshipangle = (other->get_angle());
		sound.stop(data->sampleExtra[1]);
		sound.play(data->sampleExtra[1]);
	}
	Ship::inflict_damage(other);
}


int RaalrithMenacer::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
								 // full phase = 2x damage, full out = 1/2 damage
	int d = iround((double)normal*((double)phase/50));
	return Ship::handle_damage(source, d, direct);
}


Color RaalrithMenacer::crewPanelColor(int k)
{
	STACKTRACE;
	// change the crew color, if needed
	if ( phase < 30.0 ) {
		Color c = {				 // some dark greyish color.
			200,200,200
		};
		return c;
	} else {
		return Ship::crewPanelColor(k);
	}
}


/******* Poison Definitions ***********************************************************/

RaalrithPoison::RaalrithPoison(RaalrithMenacer *creator, int nduration, Ship *nship, SpaceSprite *osprite) :
SpaceObject (creator, nship->normal_pos(), 0.0, osprite),
duration(nduration),
oship(nship)
{
	STACKTRACE;

	owner = creator;
	target = oship;
	id |= RAALRITH_POISON_ID;
	layer = LAYER_HOTSPOTS;
	start = TRUE;
	collide_flag_anyone = 0;
	Query q;
	for (q.begin(oship, bit(LAYER_HOTSPOTS), 10); q.current; q.next()) {
		if (!q.current->isObject())
			continue;
		if ((q.current->getID() == getID()) && (((RaalrithPoison*)q.current)->oship == oship)) {
			((RaalrithPoison*)q.current)->duration = duration;
			state = 0;
		}
	}
	q.end();
}


void RaalrithPoison::calculate()
{
	STACKTRACE;

	int chance;

	SpaceObject::calculate();

	if (!(oship && oship->exists())) {
		oship = 0;
		state = 0;
		return;
	}

	if (!(owner && owner->exists())) {
		owner = 0;
		// don't worry about the state; you just need to detect, when the owner dies...
		// otherwise, the owner&&owner->latched are not likely to be 0 ... and duration
		// is always reset.
	}

	//  x = oship->normal_x();
	//  y = oship->normal_y();
	pos = oship->normal_pos();
	//  vx = oship->get_vx();
	//  vy = oship->get_vy();
	vel = oship->get_vel();
	duration -= frame_time;

	if (duration < 0) {
		if (oship->spritePanel)
			blit(oship->data->spritePanel->get_bitmap(0), oship->spritePanel->get_bitmap(0), 16 , 18, 16, 18, 32, 30);
		oship->update_panel = TRUE;
		state = 0;
		return;
	}

	if (owner && owner->latched)
		duration = owner->poisonDuration;

	if (start) {
		start = FALSE;
		if (oship->spritePanel)
			data->spriteExtra->draw(16, 18, 0, oship->spritePanel->get_bitmap(0));
		oship->update_panel = TRUE;
	}

	chance = random(2000);
	if (chance < frame_time) {
		damage(oship, 0, 1);
		sound.play(data->sampleExtra[0]);
	}

	return;
}


void RaalrithPoison::animate(Frame *space)
{
	STACKTRACE;
	return;
}


REGISTER_SHIP(RaalrithMenacer)
