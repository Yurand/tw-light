/* $Id: shpvuvji.cpp,v 1.5 2004/03/24 23:51:43 yurand Exp $ */
#include "../ship.h"
REGISTER_FILE

class VuvJinx : public Ship
{
	public:
		double       weaponRange, weaponVelocity, weaponSpread;
		int          weaponNumber;

		VuvJinx(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

		virtual void calculate_thrust();
		virtual void calculate_hotspots();

		virtual int activate_weapon();
		virtual void calculate_fire_special();
};

class VuvJinxShot : public SpaceLine
{
	double        range, d, v;
	int           spark_time;
	public:

		VuvJinxShot (Vector2 opos, double oangle, double ov, double orange, SpaceLocation *creator);

		virtual void  calculate();
		virtual void  inflict_damage(SpaceObject *other);
		virtual void  animate(Frame *space);
};

class VuvJinxSpark : public SpaceLine
{
	int lifetime;
	public:
		VuvJinxSpark(SpaceLocation *creator);
		virtual void calculate();
};

class VuvJinxExplosion : public SpaceLine
{
	int lifetime;

	public:

		VuvJinxExplosion (SpaceLocation *creator, Vector2 opos, double oangle, double ov);
		virtual void calculate();
		virtual void animate(Frame *space);
};

VuvJinx::VuvJinx(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponNumber   = get_config_int("Weapon", "Number", 0);
	weaponSpread   = scale_range(get_config_float("Weapon", "Spread", 0));
}


void VuvJinx::calculate_thrust()
{
	STACKTRACE;
	if (fire_special)
		accelerate(this, angle+180, accel_rate * frame_time, speed_max);
	else    if (thrust)
		accelerate(this, angle, accel_rate * frame_time, speed_max);
}


void VuvJinx::calculate_hotspots()
{
	STACKTRACE;
	if (!fire_special)
		Ship::calculate_hotspots();
}


int VuvJinx::activate_weapon()
{
	STACKTRACE;
	double dx, dy, da;			 //, dv;
	for (int i=0; i < weaponNumber; i++) {
		dx = weaponSpread;
		if (weaponNumber > 1)
			dy = weaponSpread * (i/double(weaponNumber-1) - 0.5);
		else    dy = 0;
		da = normalize(angle + PI + atan3(dy, dx), PI2);
		//dv = sqrt(dx*dx + dy*dy) / weaponRange;
		dx += 32;
		game->add(new VuvJinxShot(Vector2(dx, dy), da, weaponVelocity, weaponRange, this));
	}
	return true;
}


void VuvJinx::calculate_fire_special()
{
	STACKTRACE;
	return;
}


VuvJinxShot::VuvJinxShot (Vector2 opos, double oangle, double ov, double orange, SpaceLocation *creator) :
SpaceLine(creator, creator->normal_pos(), oangle, 0, 0),
range(orange),
d(0),
v(ov)
{
	play_sound(data->sampleWeapon[1]);
	//double alpha = creator->get_angle() * ANGLE_RATIO;
	//double tx = cos(alpha), ty = sin(alpha);
	//x += oy * tx - ox * ty;
	//y += oy * ty + ox * tx;
	pos += rotate(opos, angle);
	damage_factor = 1;
	//vx = ov * cos (angle * ANGLE_RATIO) + creator->get_vx();
	//vy = ov * sin (angle * ANGLE_RATIO) + creator->get_vy();
	vel = ov * unit_vector (angle) + creator->get_vel();
	spark_time = 0;
}


void VuvJinxShot::animate(Frame *space)
{
	STACKTRACE;
	return;
}


void VuvJinxShot::calculate()
{
	STACKTRACE;
	SpaceLine::calculate();
	d += v * frame_time;
	if (d > range) state = 0;
	if (spark_time > 0) spark_time -= frame_time;
	else {
		while ( spark_time <= 0) {
			spark_time += 50;
			game->add(new VuvJinxSpark(this));
		}
	}
}


void VuvJinxShot::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	//other->damage += 1;
	other->handle_damage(this, 1);
	play_sound(data->sampleWeapon[2]);
	state = 0;
	double vv;
	for (int i=0; i < 15; i++) {
		vv = random(1.0);
		vv = 0.0+sqrt(vv);
		game->add(new VuvJinxExplosion(this, pos,
			random(360.0), scale_velocity(40)*vv));
	}
}


VuvJinxSpark::VuvJinxSpark(SpaceLocation *creator) :
SpaceLine(creator, creator->normal_pos(), creator->get_angle(), 0, makecol(255, 0, 255))
{
	STACKTRACE;
	lifetime = 255;
	collide_flag_anyone = 0;
	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;
	angle += normalize(180 + 25 * (1 - random(2.0)), 360);
	double v = scale_velocity(20) * (0.5 + random(1.0));
	//vx = creator->vx + v * cos(angle*ANGLE_RATIO);
	//vy = creator->vy + v * sin(angle*ANGLE_RATIO);
	vel = creator->vel + v * unit_vector(angle);
}


void VuvJinxSpark::calculate()
{
	STACKTRACE;
	color = makecol(lifetime, 0, lifetime);
	SpaceLine::calculate();
	if ((lifetime -= frame_time) <= 0) state = 0;
}


VuvJinxExplosion::VuvJinxExplosion (SpaceLocation *creator, Vector2 opos, double oangle, double ov) :
SpaceLine(creator, opos, oangle, 0, makecol(255, 0, 255))
{
	set_depth(LAYER_EXPLOSIONS);
	collide_flag_anyone = 0;
	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;
	lifetime = 400;
	vel = ov * unit_vector(angle);
}


void VuvJinxExplosion::calculate()
{
	STACKTRACE;
	SpaceLine::calculate();
	lifetime -= frame_time;
	if (lifetime <= 0) {
		state = 0; return;
	}
	double a = lifetime / 400.0;
	color = makecol(int(255*a),0,int(255*a));
}


void VuvJinxExplosion::animate(Frame *space)
{
	STACKTRACE;
	Vector2 oldpos, tpos;
	//double xx = x, yy = y;
	//double tx = cos(angle*ANGLE_RATIO);
	//double ty = sin(angle*ANGLE_RATIO);
	oldpos = pos;
	tpos = unit_vector(angle);
	double a = lifetime / 400.0;
	for (int i=3; i >=0; i--) {
		color = makecol(int((255*a)*(4-i)/4.0),0,int((255*a)*(4-i)/4.0));
		//x = xx - i*tx*2;
		//y = yy - i*ty*2;
		pos = oldpos - i * tpos * 2;
		SpaceLine::animate(space);
	}
	pos = oldpos;
}


REGISTER_SHIP(VuvJinx)
