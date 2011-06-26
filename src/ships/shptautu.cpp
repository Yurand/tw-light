/* $Id: shptautu.cpp,v 1.9 2004/03/24 23:51:42 yurand Exp $ */
#include "../ship.h"
REGISTER_FILE

#define TAU_TURBO_MISSILE SPACE_HOMING_MISSILE+0x106C

class TauTurbo : public Ship
{
	double  weaponRange, weaponVelocity, weaponAccel, weaponTurnRate;
	double  weaponDamage, weaponArmour;

	double  accel_boost, current_boost, brakes, speed_boost;
	int     extra_hotspot_frame, extra_hotspot_rate;

	int     fire_frame;
	bool    hotspot_index;
	bool    shield;
	int     recharge_reserve, drain_frame;

	public:
		TauTurbo(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual int  activate_weapon();
		virtual void calculate_fire_special();
		virtual void calculate();
		virtual void animate(Frame *space);
		virtual void calculate_hotspots();
		virtual void calculate_thrust();
		virtual int  handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void inflict_damage(SpaceObject *other);
		virtual double isProtected() const;
};

class TauTurboMissile : public HomingMissile
{
	double accel;
	int fire_index, fire_frame, smoke_frame, first_frame;

	public:
		TauTurboMissile (SpaceLocation *creator, double ox, double oy, double oangle, double oaccel,
			double ov, double otr, double orange,
			SpaceSprite *osprite, double odamage, double oarmour, SpaceObject *otarget);
		virtual void calculate();
		virtual void animate(Frame *space);
};

TauTurbo::TauTurbo(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponAccel    = scale_acceleration(get_config_float("Weapon", "Accel", 0), 0);
	weaponTurnRate = scale_turning(get_config_float("Weapon", "TurnRate", 0));
	weaponDamage   = get_config_float("Weapon", "Damage", 0);
	weaponArmour   = get_config_float("Weapon", "Armour", 0);

	accel_boost    = get_config_float("Extra", "AccelBoost", 1);
	speed_boost    = get_config_float("Extra", "SpeedBoost", 1);
	brakes         = get_config_float("Extra", "Brakes", 0) / 1000.0;

	extra_hotspot_rate  = 25;
	extra_hotspot_frame = 0;

	recharge_reserve = recharge_amount;

	fire_frame = 0;
	hotspot_index = true;
	current_boost = 0;
	shield = false;
	drain_frame = 0;
}


int TauTurbo::activate_weapon()
{
	STACKTRACE;
	if (shield) return false;

	game->add(new TauTurboMissile (this, 6, 0, angle, weaponAccel,
		weaponVelocity, weaponTurnRate, weaponRange,
		data->spriteWeapon, weaponDamage, weaponArmour, target));
	game->add(new TauTurboMissile (this, -6, 0, angle, weaponAccel,
		weaponVelocity, weaponTurnRate, weaponRange,
		data->spriteWeapon, weaponDamage, weaponArmour, target));

	fire_frame = 51;
	drain_frame = 0;			 //special_rate;

	return true;
}


void TauTurbo::calculate_fire_special()
{
	STACKTRACE;
	if (fire_special && shield) {
		shield = false;
		recharge_amount = recharge_reserve;
		sprite = data->spriteShip;
	}
}


void TauTurbo::calculate()
{
	STACKTRACE;
	Ship::calculate();
	if (fire_frame > 0) fire_frame -= frame_time;
	if (shield) {
		if (drain_frame > 0) drain_frame -= frame_time;
		while (drain_frame <= 0) {
			drain_frame += special_rate;
			batt -= 1;
		}
		if (batt <= 0) {
			batt = 0;
			shield = false;
			recharge_amount = recharge_reserve;
			sprite = data->spriteShip;
		}
	}
}


void TauTurbo::animate(Frame *space)
{
	STACKTRACE;
	if ((fire_frame > 0) && !shield)
		sprite->animate(pos, sprite_index + 64, space);
	else    sprite->animate(pos, sprite_index, space);
}


void TauTurbo::calculate_hotspots()
{
	STACKTRACE;
	//	Ship::calculate_hotspots();

	if (extra_hotspot_frame > 0) {
		extra_hotspot_frame -= frame_time;
		return;
	}

	if (!thrust) return;

	while (extra_hotspot_frame <= 0)
		extra_hotspot_frame += extra_hotspot_rate;
	hotspot_index = !hotspot_index;

	int ff = (int)ceil(current_boost*20);
	if (ff == 0) return;

	double rx, ry;
	double tx = cos(angle);
	double ty = sin(angle);

	int i;
	for (i=0; i<2; i++) {
		if (hotspot_index) rx = (i-0.5)*5;
		else               rx = (i-0.5)*10;
		ry = -22;
		game->add(new Animation(this, Vector2(pos.x+ry*tx-rx*ty, pos.y+ry*ty+rx*tx),
			data->spriteExtraExplosion, 20-ff, ff, 25, LAYER_HOTSPOTS));
		//                e->vx = vx;
		//                e->vy = vy;
		//                e->accelerate(this, angle+PI, scale_velocity(30), GLOBAL_MAXSPEED);
	}
}


void TauTurbo::calculate_thrust()
{
	STACKTRACE;
	if (!thrust) return;

								 //vx*cos(angle) + vy*sin(angle);
	double vc = dot_product(vel,unit_vector(angle));
	//	if (!shield) {
	double vv;
	if (fabs(vc) > 1e-20) vv = vc / magnitude(vel) ;
	else vv = 1;
	if (vv < 0) vv = 1;
	else    vv = 1 - vv;
	vv = (sqrt(fabs(vv)));
	vv = exp(-vv*brakes*frame_time);
	vel *= vv;
	//}
	vc /= 0.99*speed_max*speed_boost;
	if (vc < 0) vc = 0;
	if (vc > 1) vc = 1;
	//	vc *= vc;
	vc *= vc;
	if (shield)
		current_boost = 0;
	else    current_boost = vc;
	Ship::calculate_thrust();
	accelerate(this, angle, accel_rate * (1 + current_boost*accel_boost) * frame_time,
		speed_max * (1+current_boost*speed_boost) );
}


int TauTurbo::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if ((!shield) && (batt > special_drain)) {
		batt -= special_drain;
		shield = true;
		recharge_amount = 0;
		play_sound(data->sampleSpecial[0]);
		sprite = data->spriteSpecial;
	}
	if (shield) {
		normal = 0;
		direct = 0;
	}
	return Ship::handle_damage(source, normal, direct);
}


void TauTurbo::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if ((!shield) && (batt > special_drain)) {
		batt -= special_drain;
		shield = true;
		recharge_amount = 0;
		play_sound(data->sampleSpecial[0]);
		sprite = data->spriteSpecial;
	}
}


double TauTurbo::isProtected() const
{
	return (shield || (batt > special_drain));
}


TauTurboMissile::TauTurboMissile (SpaceLocation *creator, double ox, double oy, double oangle, double oaccel,
double ov, double otr, double orange,
SpaceSprite *osprite, double odamage, double oarmour, SpaceObject *otarget) :
HomingMissile(creator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, otr, creator,  osprite, otarget)
{
	id = TAU_TURBO_MISSILE;

	vel += creator->get_vel();

	accel = oaccel;
	fire_index = random() % 20;
	fire_frame = 50;

	explosionSprite = data->spriteWeaponExplosion;
	explosionFrameCount = 10;
	explosionFrameSize = 50;
	explosionSample = data->sampleWeapon[1];

	set_depth(DEPTH_EXPLOSIONS);

	smoke_frame = 0;
	first_frame = 8;
}


void TauTurboMissile::calculate()
{
	STACKTRACE;
	Vector2 ovv = vel;
	magnitude(vel);
	double otr = turn_rate;
	if (d < range) turn_rate *= d / range;
	HomingMissile::calculate();
	turn_rate = otr;
	vel = ovv;
	accelerate(this, angle, accel*frame_time, v);

	if (fire_frame > 0) fire_frame -= frame_time;
	else {
		while (fire_frame <= 0) {
			fire_index += 1;
			fire_frame +=50;
		}
		fire_index %= 20;
	}

	if (smoke_frame > 0) smoke_frame -= frame_time;
	else {
		while (smoke_frame <= 0)
			smoke_frame += 25;
		game->add(new Animation(this, pos, data->spriteExtra,
			first_frame, 10-first_frame, 50, LAYER_HOTSPOTS));
		if (first_frame > 0) first_frame -= 2;
	}

	Query q;
	double aa, bb;
	for (q.begin(this, bit(LAYER_SHOTS), 30); q.currento; q.next())
	if (q.currento->getID() == id) {
		aa = (trajectory_angle(q.currento)+PI);
		bb = 0.01*frame_time;	 //*(30-distance(q.currento))/30.0;
		pos += bb * unit_vector(aa);
	}
	q.end();
}


void TauTurboMissile::animate(Frame *space)
{
	STACKTRACE;
	sprite->animate(pos, sprite_index, space);
	data->spriteSpecialExplosion->animate(pos, fire_index, space);
}


REGISTER_SHIP(TauTurbo)
