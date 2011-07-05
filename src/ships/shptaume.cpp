/* $Id: shptaume.cpp,v 1.13 2004/03/24 23:51:42 yurand Exp $ */
#include "../ship.h"
#include "../melee/mview.h"
#include "../frame.h"

REGISTER_FILE

class TauMercury : public Ship
{
	double  weaponRange, weaponVelocity, weaponRelativity;
	double  weaponDamage, weaponArmour;
	double  weaponSpread, weaponAngle;
	bool    weaponSparks, weaponFrags;

	double  bank_position, bank_max, bank_time, bank_relax;
	double  lin_abs, exp_abs, thrust_limit;

	bool    turn_lag;

	public:
		TauMercury(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual int  activate_weapon();
		virtual void calculate();
		virtual void calculate_hotspots();
		virtual void calculate_thrust();
		virtual int  accelerate(SpaceLocation *source, double angle, double velocity, double max_speed);
		virtual void calculate_turn_right();
		virtual void calculate_turn_left();
		virtual void calculate_fire_special();
};

class TauMercuryShot : public AnimatedShot
{
	bool    sparks, frags;

	public:

		TauMercuryShot(SpaceLocation *creator, double ox, double oy, double oangle, double ov,
			double odamage, double orange, double oarmour, SpaceSprite *osprite, double relativity,
			bool osparks, bool ofrags);
		virtual void animateExplosion();
		virtual void calculate();
};

class TauMercurySpark : public SpaceLine
{
	int lifetime, lifetime_max, r, g, b;

	public:

		TauMercurySpark (SpaceLocation *creator, double ox, double oy, double oangle, double ov,
			int olifetime, int blah_or, int og, int ob, double od, double relativity, double odamage = 0);
		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual void animate(Frame *space);
};

TauMercury::TauMercury(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	weaponRange     = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity  = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage    = get_config_float("Weapon", "Damage", 0);
	weaponArmour    = get_config_float("Weapon", "Armour", 0);
	weaponRelativity    = get_config_float("Weapon", "Relativity", 0);

	weaponSpread    = get_config_float("Weapon", "Spread", 0);
	weaponAngle     = get_config_float("Weapon", "Angle", 0)*PI/180;

	weaponSparks    = (get_config_int("Weapon", "Sparks", 0) > 0);
	weaponFrags     = (get_config_int("Weapon", "Frags", 0) > 0);

	bank_time       = get_config_int("Special", "BankTime", 1000);
	bank_relax      = get_config_int("Special", "RelaxTime", 1000);
	exp_abs         = get_config_float("Special", "ExpAbs", 0)/1000.0;
	lin_abs         = scale_acceleration(get_config_float("Special", "LinAbs", 0),0);
	thrust_limit    = get_config_float("Special", "ThrustLimit", 1.0);

	turn_lag        = (get_config_int("Special", "TurnLag", 0) > 0);

	if (sprite->frames() == 64)
		bank_max = 0.2;
	else
		bank_max = 6.0;

	bank_position = 0;

}


int TauMercury::activate_weapon()
{
	STACKTRACE;
	double da = (1-0.002*(random()%1001));
	if (da >= 0)
		da *= da;
	else
		da *= -da;
	game->add(new TauMercuryShot(this, 0, 8, angle + weaponAngle*da,
		weaponVelocity, weaponDamage, weaponRange*(1-weaponSpread*(random()%1001)/1000.0), weaponArmour,
		data->spriteWeapon, weaponRelativity, weaponSparks, weaponFrags));

	return true;
}


void TauMercury::calculate_turn_right()
{
	STACKTRACE;
	if (turn_right) {
		bank_position += bank_max * frame_time / bank_time;
		if (!turn_lag)
			turn_step += turn_rate * frame_time;
	}
}


void TauMercury::calculate_turn_left()
{
	STACKTRACE;
	if (turn_left) {
		bank_position -= bank_max * frame_time / bank_time;
		if (!turn_lag)
			turn_step -= turn_rate * frame_time;
	}
}


void TauMercury::calculate()
{
	STACKTRACE;
	double vl;
	bool locked_rail = false;

	Vector2 tt = unit_vector(angle);
	vl = dot_product(vel, tt);

	if (!fire_special) {
		Vector2 vt = vel - vl * tt;
		double vts = magnitude(vt);

		if (vts > lin_abs * frame_time)
			vt /= vts;
		vts *= exp (- exp_abs * frame_time);
		vts -= lin_abs * frame_time;
		if (vts <= 0) {
			vts = 0;
			locked_rail = true;
		}
		else
			vel = vl*tt + vt*vts;
	}

	if ((!turn_right) && (!turn_left) && (bank_relax > 0)) {
		if (bank_position > 0) {
			bank_position -= bank_max * frame_time / bank_relax;
			if (bank_position < 0) bank_position = 0;
		} else {
			bank_position += bank_max * frame_time / bank_relax;
			if (bank_position > 0) bank_position = 0;
		}
	}

	if (turn_lag) {
		double tmp1 = bank_position / (double)bank_max;
		tmp1 *= sqrt(tmp1*tmp1);
		turn_step += turn_rate * frame_time * tmp1;
	}

	Ship::calculate();

	if (bank_position > bank_max)
		bank_position = bank_max;
	if (bank_position < -bank_max)
		bank_position = -bank_max;

	int ic = 0;
	if (fabs(bank_position)>0.5) {
		if (bank_position > 0)
			ic = iround(6 + floor(bank_position + 0.5));
		else
			ic = iround(floor(-bank_position + 0.5));
	}

	sprite_index += ic*64;

	if (sprite_index < 0 || sprite_index >= sprite->frames()) {
		tw_error("wrong number of frames!!");
		//1740
		//ic==12
	}

	if (locked_rail) {
		tt = unit_vector(angle);
		vel = vl * tt;
	}

	if ((thrust) && (vl < speed_max * thrust_limit))
		if (vl > 0)
			accelerate_gravwhip(this, angle, accel_rate * frame_time * (speed_max - vl/thrust_limit)/speed_max, speed_max);
	else
		accelerate_gravwhip(this, angle, accel_rate * frame_time, speed_max);

}


int TauMercury::accelerate(SpaceLocation *source, double angle, double velocity, double max_speed)
{
	STACKTRACE;
	double ov;
	double nvs, nvl;

	Vector2 nv, tt;

	tt = unit_vector(angle);

	ov = magnitude_sqr(vel);
	nv = vel + tt * velocity;
	nvs = magnitude_sqr(nv);

	nvl = dot_product(vel, tt) + velocity;

	if ((nvs <= max_speed * max_speed) || (nvs <= ov))
		vel = nv;
	else {
		if (ov <= max_speed * max_speed) ov = max_speed;
		else ov = sqrt(ov);
		vel = nv * ov / (ov + velocity);
	}
	return true;
}


void TauMercury::calculate_hotspots()
{
	STACKTRACE;
	if ((thrust) && (hotspot_frame <= 0)) {
		game->add(new Animation( this,
			normal_pos() - unit_vector(angle) * 14,
			meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));
		hotspot_frame += hotspot_rate;
	}
	if (hotspot_frame > 0) hotspot_frame -= frame_time;
	return;
}


void TauMercury::calculate_thrust()
{
	STACKTRACE;
}


void TauMercury::calculate_fire_special()
{
	STACKTRACE;
	special_low = FALSE;
	if (!fire_special)
		return;
	while (special_recharge <= 0) {
		special_recharge += special_rate;

		//		int i;

		//		for (i=0; i<1; i++) {
		game->add(new TauMercurySpark(this, -26, -3.5, PI2*(random(1000))/1000.0,
			scale_velocity(2)*(random(101))/100.0,
			iround(800*(1-0.5*(random(101))/100.0)),
			190+random(50), 190+random(50), 200+random()%55,
			DEPTH_EXPLOSIONS, 1.0, 1));
		game->add(new TauMercurySpark(this, 26, -3.5, PI2*(random()%1000)/1000.0,
			scale_velocity(2)*(random(101))/100.0,
			iround(800*(1-0.5*(random(101))/100.0)),
			190+random(50), 190+random(50), 200+random(55),
								 //}
			DEPTH_EXPLOSIONS, 1.0, 1));
		/*		for (i=0; i<1; i++) {
					game->add(new TauMercurySpark(this, -26, -3.5, PI2*(random()%1000)/1000.0,
								scale_velocity(5)*(random()%101)/100.0,
								800*(1-0.5*(random()%101)/100.0),
								190+random()%50, 190+random()%50, 200+random()%55,
								DEPTH_EXPLOSIONS, 0.0));
					game->add(new TauMercurySpark(this, 26, -3.5, PI2*(random()%1000)/1000.0,
								scale_velocity(5)*(random()%101)/100.0,
								800*(1-0.5*(random()%101)/100.0),
								190+random()%50, 190+random()%50, 200+random()%55,
								DEPTH_EXPLOSIONS, 0.0)); }
		*/
	}
}


TauMercuryShot::TauMercuryShot(SpaceLocation *creator, double ox, double oy, double oangle, double ov,
double odamage, double orange, double oarmour, SpaceSprite *osprite, double relativity,
bool osparks, bool ofrags) :
AnimatedShot(creator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, creator, osprite, 10, 50, relativity),
sparks(osparks), frags(ofrags)
{
	STACKTRACE;
	if (frags)
		explosionSample = data->sampleWeapon[1];
}


void TauMercuryShot::calculate()
{
	STACKTRACE;
	int si = sprite_index;
	AnimatedShot::calculate();

	if (!sparks)
		return;

	si = sprite_index - si;

	while (si < 0)
		si += 20;

	if (si > 0) {
		int i;
		for (i=0; i<2; i++)
			game->add(new TauMercurySpark(this, 0, 0, PI2*(random(1000))/1000.0,
				scale_velocity(20)*(random(101))/100.0,
				iround(800*(1-0.5*(random(101))/100.0)),
				160+random(50), 160+random(50), 200+random(55),
				DEPTH_HOTSPOTS, 1.0));
		for (i=0; i<2; i++)
			game->add(new TauMercurySpark(this, 0, 0, PI2*(random(1000))/1000.0,
				scale_velocity(20)*(random(101))/100.0,
				iround(800*(1-0.5*(random(101))/100.0)),
				160+random(50), 160+random(50), 200+random(55),
				DEPTH_HOTSPOTS, 0.0));

	}

}


void TauMercuryShot::animateExplosion()
{
	STACKTRACE;
	if (!frags) {
		Shot::animateExplosion();
		return;
	}

	Animation *a;
	int i, ff;
	double vv = magnitude_sqr(vel);
	for (i=0; i<6; i++) {
		ff = random()%10;
		a = new Animation(this, pos, data->spriteWeaponExplosion,
			ff, 20-ff, 25, DEPTH_EXPLOSIONS);
		game->add(a);
		a->accelerate(this, angle, vv*0, MAX_SPEED);
		a->accelerate(this,  PI2*(random()%1000)/1000.0, 0.01*(random()%101)*scale_velocity(25), MAX_SPEED);
		a->collide_flag_anyone = 0;
	}
}


TauMercurySpark::TauMercurySpark (SpaceLocation *creator, double ox, double oy, double oangle, double ov, int olifetime,
int blah_or, int og, int ob, double od, double relativity, double odamage) :
SpaceLine(creator, Vector2(ox,oy), oangle, 1, 0),
lifetime(olifetime), lifetime_max(olifetime),
r(blah_or), g(og), b(ob)
{
	if (odamage == 0)
		collide_flag_anyone = 0;
	else {
		damage_factor = odamage;
		collide_flag_anyone = OBJECT_LAYERS;
		collide_flag_sameship = 0;
		collide_flag_sameteam = OBJECT_LAYERS;
	}

	color = makecol(r, g, b);
	set_depth(od);

	pos = normalize(creator->normal_pos() + rotate(Vector2(-ox, oy), -PI/2+creator->get_angle()));

	vel = creator->get_vel() * relativity + ov * unit_vector(angle);

}


void TauMercurySpark::calculate()
{
	STACKTRACE;
	lifetime -= frame_time;
	if (lifetime <= 0)
		state = 0;

	SpaceLine::calculate();
}


void TauMercurySpark::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	SpaceLine::inflict_damage(other);
	state =0;
}


void TauMercurySpark::animate(Frame *space)
{
	STACKTRACE;
	drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);

	double c = lifetime/(double)lifetime_max;

	Vector2 p0 = corner(pos);
	int x0, y0;

	x0 = iround(p0.x);
	y0 = iround(p0.y);

	if (space_zoom <= 1)
		set_trans_blender(0, 0, 0, iround(space_zoom * 255 * c));
	else
		set_trans_blender(0, 0, 0, iround(1 * 255 * c));

	putpixel(space->surface, x0, y0, color);
	space->add_pixel(x0, y0);

	drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}


REGISTER_SHIP(TauMercury)
