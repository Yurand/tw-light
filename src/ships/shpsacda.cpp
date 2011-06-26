/* $Id: shpsacda.cpp,v 1.1 2006/01/29 16:14:34 geomannl Exp $ */
#include "../ship.h"
#include "../util/aastr.h"
#include "../melee/mview.h"
#include "../frame.h"

REGISTER_FILE

class SacreeDagger : public Ship
{
	public:
		double      weaponRange, weaponDamage, weaponFrameCount;

		double      specialRange;
		double      specialVelocity;
		double      specialDamage;
		double      specialArmour;
		double      specialRecoil;

	public:
		SacreeDagger(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

		virtual int  activate_weapon();
		virtual int  activate_special();
};

class SacreeDaggerBeam : public SpaceLine
{
	public:
	protected:
		int             frame, frame_count;
		double          relative_angle;
		SpaceLocation   *lpos;
		Vector2         rel_pos;
		double          damage_shots;
		double          base_length;
		bool            got_spark;
	public:
		SacreeDaggerBeam(SpaceLocation *creator, Vector2 rpos, double lrange,
			double ldamage, int lfcount, double oangle);
		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual void animate(Frame *space);
};

class SacreeDaggerShot : public Missile
{
	public:
	protected:
		int         s_ind;
	public:
		SacreeDaggerShot(SpaceLocation *creator, Vector2 opos, double oangle,
			double ov, double odamage, double orange, double oarmour, SpaceSprite *osprite);
		virtual void animate(Frame *space);
};

SacreeDagger::SacreeDagger(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange         = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponDamage        = tw_get_config_float("Weapon", "Damage", 0);
	weaponFrameCount    = tw_get_config_int("Weapon", "FrameCount", 0);

	specialRange        = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity     = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage       = tw_get_config_float("Special", "Damage", 0);
	specialArmour       = tw_get_config_float("Special", "Armour", 0);
	specialRecoil       = scale_velocity(tw_get_config_float("Special", "Recoil", 0));

}


int SacreeDagger::activate_weapon()
{
	STACKTRACE;
	game->add(new SacreeDaggerBeam(this, Vector2(0,30), weaponRange, weaponDamage, iround(weaponFrameCount), angle));
	return true;
}


int SacreeDagger::activate_special()
{
	STACKTRACE;

	game->add(new SacreeDaggerShot(this, Vector2(0,55), angle, specialVelocity,
		specialDamage, specialRange, specialArmour, data->spriteSpecial));
	//game->add(new SacreeDaggerBeam(this, Vector2(15,9), 43.5, 0, 50, angle-19.5*ANGLE_RATIO));
	//game->add(new SacreeDaggerBeam(this, Vector2(-15,9), 43.5, 0, 50, angle+19.5*ANGLE_RATIO));

	accelerate(this, -unit_vector(angle)*specialRecoil, speed_max);

	return true;
}


SacreeDaggerBeam::SacreeDaggerBeam(SpaceLocation *creator, Vector2 rpos, double lrange,
double ldamage, int lfcount, double oangle) :
SpaceLine(creator, creator->normal_pos(), oangle, lrange, 0),
frame(0), frame_count(lfcount), lpos(creator), rel_pos(rpos)

{
	STACKTRACE;
	if (ldamage <= 0) collide_flag_anyone = 0;
	set_depth(DEPTH_SHOTS);
	base_length = length;
	rel_pos.x *= -1;
	pos = normalize(pos + rotate(rel_pos, -PI/2+lpos->get_angle()));
	vel = lpos->get_vel();
	id |= SPACE_LASER;
	damage_factor = ldamage;

	int rrr = tw_random(85);
	color = tw_makecol(100+rrr,100+rrr+tw_random(55),205+tw_random(51));

	relative_angle = angle - lpos->get_angle();

	got_spark = false;
}


void SacreeDaggerBeam::calculate()
{
	STACKTRACE;
	if ((frame < frame_count) && (lpos->exists())) {
		length = base_length;
		pos = lpos->normal_pos() + rotate(rel_pos, lpos->get_angle() - PI/2);
		vel = lpos->get_vel();
		//		angle = lpos->get_angle();
		angle = normalize(lpos->get_angle() + relative_angle, PI2);
		SpaceLine::calculate();
		frame += frame_time;
	}
	else
		state = 0;

	int rrr = tw_random(85);
	color = tw_makecol(100+rrr,100+rrr+tw_random(55),205+tw_random(51));
}


void SacreeDaggerBeam::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	damage(other, damage_factor*frame_time/frame_count);

	int aa = get_tw_aa_mode();
	if (!((aa & AA_BLEND) && (aa & AA_ALPHA) && (!got_spark) && !(aa & AA_NO_AA))) {
		physics->add(new Animation( this,
			pos + edge(), meleedata.sparkSprite, 0,
			SPARK_FRAMES, 50, DEPTH_EXPLOSIONS));
		got_spark = true;
	}

	return;
}


void SacreeDaggerBeam::animate(Frame *space)
{
	STACKTRACE;
	int aa = get_tw_aa_mode();
	SpaceLine::animate(space);
	if ((aa & AA_BLEND) && (aa & AA_ALPHA) && !(aa & AA_NO_AA) && (length < base_length*0.9999) && (target)) {
		int _old_trans = aa_get_trans();
		// changed tw_random into rand - GEO - so that physics are not affected by the animation
		aa_set_trans(rand()%136);//graphics
		data->spriteWeaponExplosion->animate(pos+edge(), 0, space);
		aa_set_trans(_old_trans);
	}
}


SacreeDaggerShot::SacreeDaggerShot(SpaceLocation *creator, Vector2 opos, double oangle,
double ov, double odamage, double orange, double oarmour, SpaceSprite *osprite) :
Missile(creator, opos, oangle, ov, odamage, orange, oarmour, creator, osprite)
{
	STACKTRACE;
	explosionSprite     = data->spriteSpecialExplosion;
	explosionFrameCount = 20;
	explosionFrameSize  = 25;
	explosionSample = data->sampleSpecial[1];
	s_ind = 0;

	add(new Animation(this, pos, data->spriteExtra, 0, data->spriteExtra->frames(), 25, DEPTH_HOTSPOTS+0.25));
}


void SacreeDaggerShot::animate(Frame *space)
{
	STACKTRACE;

	int k;
	k = sprite_index + 64 * s_ind;
	int n = sprite->frames();
	if ( k >= n) {
		tw_error("sprite index error");
	}

	sprite->animate(pos, k, space);
	s_ind = (s_ind + 1) % 2;
}


REGISTER_SHIP(SacreeDagger)
