/* $Id: shptauem.cpp,v 1.18 2005/08/28 20:34:08 geomannl Exp $ */
#include "../ship.h"
#include "../util/aastr.h"
#include "../frame.h"
#include "../melee/mview.h"

REGISTER_FILE

// allows other ships to affect control over a ship.
class OverrideControlTauEMP : public OverrideControl
{
	public:
		int jamkey;
		virtual void calculate(short *key);
};

class TauEMP : public Ship
{
	public:
		double      weaponVelocity, weaponRange, weaponDamage, weaponArmour, weaponRelativity;

		int         slot;

		double      specialRange, wave_radius, specialVelocity;
		double      specialAttenuation, specialJamTime;
		bool        specialJamFriendly;

		int         specialFlashTime, specialFlashTime2, flash_counter;
		bool        DoAlphaBlending;

	public:
		TauEMP(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual int  activate_weapon();
		virtual void calculate_fire_special();
		virtual void animate(Frame *space);
		virtual void calculate_hotspots();

};

class TauEMPVirtualTarget : public SpaceObject
{
	public:
	public:
		TauEMPVirtualTarget(SpaceLocation *creator, SpaceSprite *osprite);
		virtual void calculate();
		virtual void animate(Frame *space);
};

class TauEMPJammer : public SpaceLocation
{
	public:
		OverrideControlTauEMP *ocm;
	protected:
		Ship    *jamtarget;
		int     jamtime;
		//int		jamkey;
	public:
		TauEMPJammer(SpaceLocation *creator, Ship *tgt, int jtime);
		virtual void calculate();
};

class TauEMPMissile : public Missile
{
	public:
	public:
		TauEMPMissile(SpaceLocation *creator, Vector2 opos, double oangle, double ov, double odamage, double orange, double oarmour, SpaceSprite *osprite, double relativity);
};

TauEMP::TauEMP(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange         = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity      = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage        = tw_get_config_float("Weapon", "Damage", 0);
	weaponArmour        = tw_get_config_float("Weapon", "Armour", 0);

	slot = 0;

	specialRange        = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity     = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialAttenuation  = tw_get_config_float("Special", "Attenuation", 1);
	specialJamFriendly  = (tw_get_config_int("Special", "JamFriendly", 0) > 0);

	wave_radius         = 0;

	specialFlashTime    = (int)(tw_get_config_float("Special", "FlashTime", 0)*1000);
	specialFlashTime2   = (int)(tw_get_config_float("Special", "FlashTime2", 0)*1000);
	flash_counter       = 0;
	specialJamTime      = tw_get_config_float("Special", "JamTime", 0) * 1000;

	DoAlphaBlending     = (tw_get_config_int("Extra", "DoAlphaBlending", 1) > 0);

	if (!DoAlphaBlending)
		specialFlashTime = specialFlashTime2;

}


int TauEMP::activate_weapon()
{
	STACKTRACE;
	int rx;
	if (slot<2) rx = 8;
	else
	if (slot<4) rx = 10;
		else
			rx = 13;
	if (slot%2) rx = -rx;
	game->add(new TauEMPMissile (this, Vector2(rx, 10), angle, weaponVelocity,
		weaponDamage, weaponRange, weaponArmour, data->spriteWeapon,
		weaponRelativity));
	slot = (slot +1) % 6;
	return true;
}


void TauEMP::calculate_fire_special()
{
	STACKTRACE;
	if (wave_radius > 0.1) {
								 //propagate the wave
		wave_radius += specialVelocity * frame_time;
		if (wave_radius > specialRange)
			wave_radius = 0;
		//jam stuff
		SpaceObject *t = new TauEMPVirtualTarget(this, sprite);
		game->add(t);
		SpaceObject *o;
		Query q;
		for (q.begin(this, OBJECT_LAYERS, wave_radius); q.currento; q.next()) {
			if (!q.current->isObject())
				continue;
			o = q.currento;
			if (specialJamFriendly || !o->sameTeam(this)) {
				if (o->isShip()) {
					if (((Ship*)o)->nextkeys!=0) {
						double a = 1.0 - pow(distance(o)/specialRange, 1/specialAttenuation);

						// avoid having negative values...
						if (a < 0.1)
							a = 0.1;

						game->add(new TauEMPJammer(this, (Ship*)o, iround(a * specialJamTime)) );
					}
				}
				else
					o->target = t;
			}
		}
		q.end();

	}

	if (flash_counter > 0)		 //take care of the flash
		flash_counter -= frame_time;

	if ((fire_special) && (batt == batt_max) && (wave_radius <= 0)) {
		batt = 0;
		flash_counter = specialFlashTime;
								 //start the wave
		wave_radius = 0.75 * size.x;
		/*		SpaceObject *t = new TauEMPVirtualTarget(this, sprite);
				game->add(t);
				SpaceObject *o;
				Query q;
				for (q.begin(this, OBJECT_LAYERS, specialRange); q.currento; q.next()) {
					o = q.currento;
					if (specialJamFriendly || !o->sameTeam(this)) {
						if (o->isShip())
							game->add(new TauEMPJammer(this, (Ship*)o, (1-pow(distance(o)/specialRange, 1/specialAttenuation)) * specialJamTime));
						else
							o->target = t; } }*/
		play_sound(data->sampleSpecial[0]);
	}
}


void TauEMP::animate(Frame *space)
{
	STACKTRACE;
	//	animate wave effect

	if (wave_radius > 0.1) {
		int specialNumber = 6;
		int circle_r;
		Vector2 p0 = corner(pos,0);
		int circle_x0 = iround(p0.x);
		int circle_y0 = iround(p0.y);
		double rc = wave_radius/specialRange;
		rc = sqrt(rc);
		double r2 = pow(space_zoom, 0.33);
		if (r2 > 1) r2 = 1;
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
		int i;
		for (i=-specialNumber+1;i<=0;i++) {
			set_trans_blender(0,0,0,iround(r2*(1-rc)*(specialNumber+i)*255.0/(specialNumber)));
			circle_r = iround((wave_radius+3*i)*space_zoom);
			circle(space->surface,circle_x0,circle_y0, circle_r, tw_makecol(100,100,255));
			space->add_circle(circle_x0, circle_y0, circle_r, 0);
		}
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
	}

	//	animate ship

	if (DoAlphaBlending && (get_tw_aa_mode() & AA_BLEND) && !(get_tw_aa_mode() & AA_NO_AA) && (flash_counter > 0)) {
		int _old_trans = aa_get_trans();
		sprite->animate(pos, sprite_index+64, space);
		aa_set_trans(iround((255.0*flash_counter)/specialFlashTime));
		if (aa_get_trans() < 255)
			sprite->animate(pos, sprite_index, space);
		aa_set_trans(_old_trans);
	}
	else
	if (flash_counter > 0)
		sprite->animate(pos, sprite_index+64, space);
	else
		sprite->animate(pos, sprite_index, space);

}


void TauEMP::calculate_hotspots()
{
	STACKTRACE;
	if ((thrust) && (hotspot_frame <= 0)) {
		game->add(new Animation(this,
			normal_pos() - unit_vector(angle) * size.x / 4,
			meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));
		hotspot_frame += hotspot_rate;
	}
	if (hotspot_frame > 0)
		hotspot_frame -= frame_time;
}


TauEMPVirtualTarget::TauEMPVirtualTarget(SpaceLocation *creator, SpaceSprite *osprite) :
SpaceObject(creator, tw_random(map_size), 0, osprite)
{
	STACKTRACE;
}


void TauEMPVirtualTarget::calculate()
{
	STACKTRACE;

	if (ship && !ship->exists()) {
		ship = 0;

		state = 0;				 // it's removed
	}

	SpaceObject::calculate();
}


void TauEMPVirtualTarget::animate(Frame *space)
{
	STACKTRACE;
	return;
}


void OverrideControlTauEMP::calculate(short *key)
{
	STACKTRACE;
	*key &= ~jamkey;
}


TauEMPJammer::TauEMPJammer(SpaceLocation *creator, Ship *tgt, int jtime) :
SpaceLocation(creator, tgt->pos, tgt->get_angle())
{
	STACKTRACE;
	//jamkey = tgt->nextkeys;
	play_sound(data->sampleSpecial[1]);
	jamtime = jtime;
	jamtarget = tgt;

	ocm = new OverrideControlTauEMP();
	ocm->jamkey = tgt->nextkeys;
	jamtarget->set_override_control(ocm);

								 // it doesn't need to collide.
	attributes |= ATTRIB_UNDETECTABLE;
}


void TauEMPJammer::calculate()
{
	STACKTRACE;

	SpaceLocation::calculate();
	if (jamtarget && jamtarget->exists()) {
		pos = jamtarget->normal_pos();

		jamtime -= frame_time;

		if (jamtime <= 0)
			state = 0;
	} else {
		state = 0;
		//jamtarget = 0; no, we need this once more.
	}

	if (!exists()) {
		jamtarget->del_override_control(ocm);
	}
}


TauEMPMissile::TauEMPMissile(SpaceLocation *creator, Vector2 opos, double oangle, double ov, double odamage, double orange, double oarmour, SpaceSprite *osprite, double relativity) :
Missile(creator, opos, oangle, ov, odamage, orange, oarmour, creator, osprite)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 10;
	explosionFrameSize  = 50;
}


REGISTER_SHIP(TauEMP)
