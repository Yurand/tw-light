/* $Id: shptaufi.cpp,v 1.7 2005/08/14 16:14:32 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE

class TauFiend  :  public Ship
{
	public:
		double        weaponRange;
		double        weaponVelocity;
		int           weaponDamage;
		int           weaponArmour;
		double        weaponRelativity;

		double        specialRange;
		double        specialVelocity;
		int           specialSap;
		int           specialDuration;
		int           charge_time;
		int           charge_count;
		int           cooling_time, cooling_count;
		double        specialRelativity;

		double        residual_drain;

		int           engine_stage;
		int           engine_frame_count;
		int           engine_frame;

	public:

		TauFiend(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

		virtual  int  activate_weapon();
		virtual void  calculate_fire_special();
		virtual int  handle_damage(SpaceLocation* source, int normal, int direct);
		virtual void  animate(Frame *space);
		virtual void  calculate_thrust();
		virtual void  calculate_hotspots();
};

class TauFiendShot : public Shot
{
	public:
	public:
		TauFiendShot   (Vector2 opos, double oangle, double ov, double orange,
			int odamage, int oarmour, SpaceLocation *creator, SpaceSprite *osprite,
			double relativity);
		virtual void  calculate();
};

class TauFiendPWave : public Missile
{
	public:
		int emp_fs, delay;
	public:

		TauFiendPWave    (Vector2 opos, double oangle, double ov, double orange, TauFiend *creator,
			SpaceSprite *osprite, double relativity, int fs, int odelay);
		virtual  void inflict_damage(SpaceObject *other);
		virtual  int handle_damage(SpaceLocation *source, int normal, int direct);
		virtual   int canCollide(SpaceLocation *other);
};

class TauFiendPEffect : public Animation
{
	public:
		Ship* target;
		int fs;
	public:
		TauFiendPEffect(SpaceLocation *creator, Ship *target, SpaceSprite *osprite, int ofs, int oduration);
		virtual void calculate();
		virtual void animate(Frame *space);
};

TauFiend::TauFiend (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	weaponRange             = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity          = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage            = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour            = tw_get_config_int("Weapon", "Armour", 0);
	weaponRelativity        = tw_get_config_float("Weapon", "Relativity", 0.0);

	specialRange            = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity         = scale_velocity(tw_get_config_float("Special", "Velocity", 0));

	charge_time             = int(tw_get_config_float("Special", "ChargeTime", 1) * 1000);
	cooling_time            = int(tw_get_config_float("Special", "CoolingTime", 1) * 1000);
	specialRelativity       = tw_get_config_float("Special", "Relativity", 0.0);
	specialSap              = tw_get_config_int("Special", "Sap", 0);
	specialDuration         = tw_get_config_int("Special", "Duration", 0);

	engine_stage            = 0;
	engine_frame_count      = 0;

	charge_count = -1;
	cooling_count = 0;
	residual_drain = 0;
}


int TauFiend::activate_weapon()
{
	STACKTRACE;
	add(new TauFiendShot(Vector2(+14, 16), angle, weaponVelocity, weaponRange,
		weaponDamage, weaponArmour, this, data->spriteWeapon, weaponRelativity));
	add(new TauFiendShot(Vector2(-14, 16), angle, weaponVelocity, weaponRange,
		weaponDamage, weaponArmour, this, data->spriteWeapon, weaponRelativity));
	return true;
}


void TauFiend::calculate_fire_special()
{
	STACKTRACE;
	if (cooling_count > 0) cooling_count -= frame_time;
	if ((!fire_special) || (batt == 0)) {
		if (charge_count >= 0)
			charge_count -= frame_time;
	}
	else    if ((fire_special) && (cooling_count <= 0)) {
		if (charge_count < charge_time) {
			charge_count += frame_time;
			residual_drain += frame_time * (double)special_drain / charge_time;
		} else {
			cooling_count = cooling_time;
			charge_count = -1;
			play_sound(data->sampleSpecial[0]);
			add(new TauFiendPWave(Vector2(0, 30), angle, specialVelocity, specialRange, this,
				data->spriteSpecialExplosion, specialRelativity, specialSap, specialDuration));
		}
	}
	int a = (int)floor(residual_drain);
	batt -= a;
	if (batt < 0) batt = 0;
	residual_drain -= a;
	if (residual_drain < 0) residual_drain = 0;
}


int TauFiend::handle_damage(SpaceLocation *source, int normal, int direct)
{
	STACKTRACE;
	double alpha;
	if (source->isLine()) {
		Vector2 dpos =
			min_delta(source->normal_pos() + ((SpaceLine*)source)->edge(), map_size);
		//double dx = min_delta(source->normal_x() + ((SpaceLine*)source)->edge_x(), normal_x(), X_MAX);
		//double dy = min_delta(source->normal_y() + ((SpaceLine*)source)->edge_y(), normal_y(), Y_MAX);
		//alpha = fabs(normalize(atan3(dy, dx) * 180 / PI - angle, 360));
		alpha = fabs(normalize(dpos.atan() - angle, PI2));
	}
	else alpha = normalize(trajectory_angle(source) - angle, 360);
	if (alpha > 180) alpha -= 360;
	alpha = (90 - fabs(alpha)) / 90;
	alpha *= alpha;
	alpha *= alpha;
	alpha = normal * (alpha + (1-alpha)*(tw_random(1.0)) );
	int d = normal;
	normal = (int)floor(alpha);
	if (alpha - normal >= 0.5) normal ++;
	d -= normal;
	if (d > 0)
		play_sound(data->sampleExtra[random(8)]);
	return Ship::handle_damage(source, normal, direct);
}


void TauFiend::animate(Frame* space)
{
	STACKTRACE;
	if (engine_stage > 0)
		data->spriteExtraExplosion->animate(
								 //normal_x() - (cos(angle * ANGLE_RATIO) * w / 2.8),
								 //normal_y() - (sin(angle * ANGLE_RATIO) * h / 2.8),
			normal_pos() - unit_vector(angle) * size / 2.8,
			engine_frame, space);
	int si = (int)floor((charge_count * 10.0) / charge_time);
	if (si >= 0) {
		if (si>9) si = 9;
		data->spriteExtra->animate(
								 //normal_x() + (cos(angle * ANGLE_RATIO) * w / 2.8),
								 //normal_y() + (sin(angle * ANGLE_RATIO) * h / 2.8),
			normal_pos() + unit_vector(angle) * size / 2.8,
			si, space);
	}
	Ship::animate(space);
}


void TauFiend::calculate_thrust()
{
	STACKTRACE;
	if (engine_frame_count > 0) engine_frame_count -= frame_time;
	while (engine_frame_count <= 0) {
		engine_frame_count += 66;
		if (thrust) {
			if (engine_stage < 7)
				engine_stage++;
		}
		else    if (engine_stage > 0)
			engine_stage--;
		if (engine_stage < 7)
			engine_frame = 10-engine_stage;
		else {
			if (engine_frame > 3) engine_frame = 3;
			else {
				engine_frame --;
				if (engine_frame <0 )
					engine_frame = 3;
			}
		}
	}
	if (engine_stage > 0)
		accelerate(this, angle, accel_rate * frame_time * engine_stage/7.0, speed_max);
}


void TauFiend::calculate_hotspots()
{
	STACKTRACE;
	if ((engine_stage > 0) && (hotspot_frame <= 0)) {
		int f1 = int((HOTSPOT_FRAMES-1)*(7.0 - engine_stage)/6);
		add(new Animation(this,
		//normal_x() - (cos(angle * ANGLE_RATIO) * w / 2.8),
		//normal_y() - (sin(angle * ANGLE_RATIO) * h / 2.8),
			normal_pos() - unit_vector(angle) * size / 2.8,
			meleedata.hotspotSprite, f1, HOTSPOT_FRAMES -f1, time_ratio, LAYER_HOTSPOTS));
		hotspot_frame += hotspot_rate;
	}
	if (hotspot_frame > 0) hotspot_frame -= frame_time;
}


TauFiendShot::TauFiendShot (Vector2 opos, double oangle, double ov, double orange,
int odamage, int oarmour, SpaceLocation *creator, SpaceSprite *osprite,
double relativity) :
Shot (creator, opos, oangle, ov, odamage, orange, oarmour, creator, osprite, relativity)
{
	explosionSprite = data->spriteSpecial;
	explosionFrameCount = 6;
	explosionFrameSize = 50;
	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void TauFiendShot::calculate()
{
	STACKTRACE;
	Shot::calculate();
	if (state == 0) return;
	sprite_index = int(20 * d / range);
}


TauFiendPWave::TauFiendPWave (Vector2 opos, double oangle, double ov, double orange, TauFiend *creator,
SpaceSprite *osprite, double relativity, int fs, int odelay) :
Missile(creator, opos, oangle, ov, 0, orange, 999, creator, osprite, relativity)
{
	collide_flag_anyone = bit(LAYER_SHIPS);
	explosionSprite = data->spriteWeaponExplosion;
	explosionSample = data->sampleSpecial[1];
	emp_fs = fs;
	delay = odelay;
}


void TauFiendPWave::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	play_sound(explosionSample);
	add(new TauFiendPEffect(this, (Ship*)other, explosionSprite, emp_fs, delay));
	state = 0;
}


int TauFiendPWave::handle_damage(SpaceLocation *other, int normal, int direct)
{
	STACKTRACE;
	return normal + direct;
}


int TauFiendPWave::canCollide(SpaceLocation *other)
{
	STACKTRACE;
	return (other->isShip() && !other->sameShip(this));
}


TauFiendPEffect::TauFiendPEffect(SpaceLocation *creator, Ship *tgt, SpaceSprite *osprite, int ofs, int oduration) :
Animation(creator, 0, osprite, 0, osprite->frames(), oduration/*, 50*/, LAYER_EXPLOSIONS)
//xxx I inserted the sprite->frames() part, and removed the 50 part... dunno if it's correct though.
{
	STACKTRACE;
	target = tgt;
	fs = ofs;
	target->handle_fuel_sap(this, fs);
}


void TauFiendPEffect::calculate()
{
	STACKTRACE;
	if (!(target && target->exists()))
		target = 0;

	if (target) {
		//x = target->normal_x();
		//y = target->normal_y();
		pos = target->normal_pos();
		frame_step -= frame_time;
		while (frame_step < 0) {
			frame_step += frame_size;
			sprite_index ++;
			if (sprite_index < frame_count) {
				target->handle_fuel_sap(this, fs);
			}
			if ((sprite_index >= 10) && (sprite_index >= frame_count))
				state = 0;
		}

		if (exists())
			SpaceObject::calculate();
	}
	else state = 0;
}


void TauFiendPEffect::animate(Frame *space)
{
	STACKTRACE;
	if (sprite_index<10) Animation::animate(space);
}


REGISTER_SHIP(TauFiend)
