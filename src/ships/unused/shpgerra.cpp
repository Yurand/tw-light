/* $Id: shpgerra.cpp,v 1.4 2004/03/24 23:51:41 yurand Exp $ */
#include "../ship.h"
#include "../melee/mview.h"

REGISTER_FILE

#define GEARS 5

class JadRacer : public Ship
{
	public:
		JadRacer(Vector2 opos, double angle, ShipData *data, unsigned int code);

	private:
		double gearMaxSpeed[GEARS];
		double gearMinSpeed[GEARS];

		int curGear;

								 //	for non-linear acceleration
		double getAccel(double step);
								 //  for non-linear braking
		double getBrake(double step);

		double drift_rate;
		double shift_rate, shift_frame;
		double constant;
		double cur_gear_rel;
		double cur_speed_real;
		double cur_speed_rel;
		double upshiftOK;
		double downshiftOK;
		double thrust_angle;

		double swingDecline;
		Vector2 swingVec;

		bool    hotspot_index, bChangeSpeed, bChangeTach, bAngle;
		int     extra_hotspot_frame, extra_hotspot_rate;

		int     speed, speedMax, last_pitch, thrustMode;
		int     tach, tachMax;

		void calculate();
		void calculate_thrust();
		void calculate_turn_left();
		void calculate_turn_right();
		void calculate_hotspots();

		void animate(Frame *space);
		int activate_weapon();
		int activate_special();
		bool custom_panel_update(BITMAP* panel, int display_type);
};

JadRacer::JadRacer(Vector2 opos, double angle, ShipData *data, unsigned int code)
:   Ship(opos, angle, data, code)
{
	STACKTRACE;
	/*
		int i;

		char *curRead;
		for(i = 0; i < GEARS;i++)
		{
			curRead = "MaxSpeed(#)";
			curRead[9] = char(i+48);
			gearMaxSpeed[i] = scale_velocity(get_config_float("Ship", curRead, 0));
			curRead = "MinSpeed(#)";
			curRead[9] = char(i+48);
			gearMinSpeed[i] = scale_velocity(get_config_float("Ship", curRead, 0));
		}
	*/

	gearMaxSpeed[0] = scale_velocity(get_config_float("Ship", "MaxSpeed(0)", 0));
	gearMaxSpeed[1] = scale_velocity(get_config_float("Ship", "MaxSpeed(1)", 0));
	gearMaxSpeed[2] = scale_velocity(get_config_float("Ship", "MaxSpeed(2)", 0));
	gearMaxSpeed[3] = scale_velocity(get_config_float("Ship", "MaxSpeed(3)", 0));
	gearMaxSpeed[4] = scale_velocity(get_config_float("Ship", "MaxSpeed(4)", 0));

	gearMinSpeed[0] = scale_velocity(get_config_float("Ship", "MinSpeed(0)", 0));
	gearMinSpeed[1] = scale_velocity(get_config_float("Ship", "MinSpeed(1)", 0));
	gearMinSpeed[2] = scale_velocity(get_config_float("Ship", "MinSpeed(2)", 0));
	gearMinSpeed[3] = scale_velocity(get_config_float("Ship", "MinSpeed(3)", 0));
	gearMinSpeed[4] = scale_velocity(get_config_float("Ship", "MinSpeed(4)", 0));

	drift_rate = get_config_float("Ship", "Drift", 0);
	shift_rate = scale_frames(get_config_float("Ship", "ShiftDelay",0));
	shift_frame = 0;
	constant = get_config_float("Ship", "Constant", 0);

	upshiftOK = get_config_float("Ship", "UpshiftOK",0);
	downshiftOK = get_config_float("Ship", "DownshiftOK",0);

	speedMax = get_config_int("Ship", "SpeedPanelMax", 0);
	tachMax = get_config_int("Ship", "TachPanelMax", 0);
	thrustMode = get_config_int("Ship", "ThrustMode", 0);

	swingDecline = get_config_float("Ship", "swingDecline", 0);

	tach = 0;
	speed = 0;
	curGear = 0;
	cur_gear_rel = 0;
	cur_speed_real = 0;
	last_pitch = 0;

	swingVec = vel;

	thrust_angle = angle;

	hotspot_index = true;
	extra_hotspot_rate  = 25;
	extra_hotspot_frame = 0;
	bChangeSpeed = true;
	bChangeTach = true;
	bAngle = false;
}


void JadRacer::animate(Frame *space)
{
	STACKTRACE;
	sprite->animate(pos, sprite_index + (64*int(thrust_backwards)), space);
}


void JadRacer::calculate_turn_left()
{
	STACKTRACE;
	if (turn_left)
		bAngle = true;
	Ship::calculate_turn_left();
}


void JadRacer::calculate_turn_right()
{
	STACKTRACE;
	if (turn_right)
		bAngle = true;
	Ship::calculate_turn_right();
}


void JadRacer::calculate()
{
	STACKTRACE;

	//	double diff;

	if (shift_frame > 0)
		shift_frame -= frame_time;

	if (!(thrust_backwards && thrust) && thrustMode > 0)
		thrust_angle = angle;

	//-->	diff = fabs(vel.angle() - angle);
	//-->	if (diff > 0.001)
	//		swingVec = (vel + (unit_vector(angle) * cur_speed_real))/2.0;
	//	if (bAngle)
	//	{
	//-->	swingVec *= 1 - swingDecline;
	//		swingVec = unit_vector((swingVec.angle()+angle)/2) * swingVec.magnitude();
	//		if (swingVec.magnitude() < 0.001)
	//			bAngle = false;
	//	}
	//	if (!bAngle)
	//	{
	//		swingVec = vel;
	//	}

	game->add(new Laser(this, vel.angle(), makecol(255,255,255), magnitude(vel)*80, 0, 1, this, Vector2(size.x/-10,size.y/2)));

	if (thrustMode > 0) {
		//		cur_speed_real *= exp(drift_rate*frame_time);
		if (shift_frame <= 0 && !thrust)
			cur_speed_real *= 1 - fabs(drift_rate) * frame_time;

		if (cur_speed_real < gearMinSpeed[curGear]) {
			cur_speed_real += getAccel(cur_speed_real / gearMinSpeed[curGear])*accel_rate*frame_time;
		}

		if (cur_speed_real < gearMinSpeed[0]) {
			cur_speed_real += getAccel(cur_speed_real / gearMinSpeed[0])*accel_rate*frame_time;
		}
	} else {
		//		vel *= exp(drift_rate*frame_time);
		if (shift_frame <= 0 && !thrust)
			vel *= 1 - fabs(drift_rate) * frame_time;
		cur_speed_real = dot_product(vel, unit_vector(vel));
	}

	//	cur_speed_rel = (cur_speed_real -gearMinSpeed[0]) / (gearMaxSpeed[GEARS-1]-gearMinSpeed[0]);
	cur_speed_rel = cur_speed_real / gearMaxSpeed[GEARS-1];

	cur_gear_rel = (cur_speed_real-gearMinSpeed[curGear]) / (gearMaxSpeed[curGear]-gearMinSpeed[curGear]);
	//	cur_gear_rel = (cur_speed_real-gearMinSpeed[0] ) / (gearMaxSpeed[curGear]-gearMinSpeed[0]);
	//	cur_gear_rel = cur_speed_real / gearMaxSpeed[curGear];

	Ship::calculate();

	if (speed != ceil(cur_speed_rel * speedMax)) {
		speed = iround(ceil(cur_speed_rel * speedMax));
		bChangeSpeed = true;
		ship->update_panel = TRUE;
	}
	if (speed < 0)
		speed = 0;

	if (tach != ceil(cur_gear_rel * tachMax)) {
		tach = iround(ceil(cur_gear_rel * tachMax));
		bChangeTach = true;
		ship->update_panel = TRUE;
	}

	if (tach < 0)
		tach = 0;

	if (shift_frame > 0) {
		last_pitch = iround(2000+6500*fabs((frame_time/(shift_frame-shift_rate))-0.5));
		//		last_pitch=99999999;
	}
	else
		//		last_pitch = 1000+2000*(cur_gear_rel+0.01);
		last_pitch = iround(2000+6500*(cur_speed_rel+0.01));

	play_sound2(data->sampleExtra[0],255,last_pitch);

	if (!(thrust_backwards && thrust) && thrustMode == 1)
		vel = cur_speed_real * unit_vector(thrust_angle);

	//	if (thrustMode == 2)
	//	{
	//game->add(new Laser(this, swingVec.angle(), makecol(255,0,0), swingVec.magnitude()*80, 0, 1, this, Vector2(size.x/10,size.y/2)));

	//		vel = cur_speed_real * unit_vector((thrust_angle+swingVec.angle())/2);
	//		if (bAngle)
	//			vel += swingVec;
	//	}

}


void JadRacer::calculate_thrust()
{
	STACKTRACE;
	double new_speed = (getAccel(cur_gear_rel)*accel_rate*frame_time)/(1-fabs(drift_rate)*frame_time);

	//	double new_speed = getAccel(cur_gear_rel)*accel_rate*frame_time*exp(-drift_rate*frame_time);
	//	double new_speed = getAccel(cur_gear_rel)*accel_rate*frame_time;
	//	double new_speed = getAccel(cur_speed_rel)*accel_rate*frame_time;

	if (shift_frame <= 0) {
		if (thrustMode == 1) {
			game->add(new Laser(this, thrust_angle, makecol(255,0,0), new_speed*80, 0, 1, this, Vector2(size.x/10,size.y/2)));

			if (thrust && ((cur_speed_real + new_speed) <= gearMaxSpeed[curGear]))
				cur_speed_real += new_speed;

			if (thrust_backwards) {
				new_speed = getBrake(cur_gear_rel)*accel_rate*frame_time;
				if (cur_speed_real - new_speed >= gearMinSpeed[curGear])
					cur_speed_real -= new_speed;

				if (thrust)
					accelerate_gravwhip(this, angle+PI, new_speed, gearMinSpeed[0]);
			}
		}
		else if (thrustMode == 0) {
			game->add(new Laser(this, thrust_angle, makecol(255,0,0), new_speed*80, 0, 1, this, Vector2(size.x/10,size.y/2)));
			if (thrust)
				accelerate_gravwhip(this, angle, new_speed, gearMaxSpeed[curGear]);
			else if (thrust_backwards)
				accelerate_gravwhip(this, angle+PI, getBrake(cur_gear_rel)*accel_rate*frame_time, gearMinSpeed[0]);
		} else {
			accelerate_gravwhip(this, angle, vel.magnitude()+new_speed, gearMaxSpeed[curGear]);
			if (thrust) {
				if ((cur_speed_real + new_speed) <= gearMaxSpeed[curGear])
					cur_speed_real += new_speed;
			}
			if (thrust_backwards) {
				new_speed = getBrake(cur_gear_rel)*accel_rate*frame_time;
				if ((cur_speed_real - new_speed) >= gearMinSpeed[curGear])
					cur_speed_real -= new_speed;

				if (thrust)
					accelerate_gravwhip(this, angle+PI, new_speed, gearMinSpeed[0]);
			}

		}
	}
}


int JadRacer::activate_weapon()
{
	STACKTRACE;
	if (shift_frame <= 0) {
		if (curGear < GEARS - 1 && cur_gear_rel >= upshiftOK) {
			curGear++;
			message.print(1000,10, "Gear is now %i", curGear);
			shift_frame += shift_rate;
			return TRUE;
		}
	}
	return FALSE;
}


int JadRacer::activate_special()
{
	STACKTRACE;
	if (shift_frame <= 0) {
		if (curGear > 0 && cur_gear_rel <= downshiftOK) {
			curGear--;
			message.print(1000,10, "Gear is now %i", curGear);
			shift_frame += shift_rate;
			return TRUE;
		}
	}
	return FALSE;
}


double JadRacer::getAccel(double x)
{
	STACKTRACE;
	//	return pow((x*0.5),2)+constant;
	//	return (x+constant < 1)?x+constant:1;
	return constant;
	//	return cos(tan(exp(-x)));
	//	return pow((0-x),2);
}


double JadRacer::getBrake(double x)
{
	STACKTRACE;
	//	return pow(-x*0.5,5)-constant;
	return constant;
}


void JadRacer::calculate_hotspots()
{
	STACKTRACE;

	//		Ship::calculate_hotspots();

	if (extra_hotspot_frame > 0) {
		extra_hotspot_frame -= frame_time;
		return;
	}

	//	if (!thrust || shift_frame > 0)
	if (shift_frame > 0)
		return;

	while (extra_hotspot_frame <= 0)
								 //* fabs(0.99-cur_gear_rel);
		extra_hotspot_frame += extra_hotspot_rate;
	hotspot_index = !hotspot_index;

	int ff = int(ceil(1*cur_gear_rel*20));
	//	int ff = 19;
	if (ff == 0)
		return;
	else if (ff < 19)
		ff = 19;

	double rx, ry;
	double tx = cos(angle);
	double ty = sin(angle);

	int i;
	for (i=0; i<2; i++) {
		if (hotspot_index)
			rx = (i-0.5)*5;
		else
			rx = (i-0.5)*10;
		ry = -22;
		game->add(new Animation(this, Vector2(pos.x+ry*tx-rx*ty, pos.y+ry*ty+rx*tx),
			data->spriteExtraExplosion, 20-ff, ff, 25, LAYER_HOTSPOTS));
		//                e->vx = vx;
		//                e->vy = vy;
		//                e->accelerate(this, angle+PI, scale_velocity(30), GLOBAL_MAXSPEED);
	}

}


bool JadRacer::custom_panel_update(BITMAP* panel, int display_type)
{
	STACKTRACE;
	int i, bar_x, bar_y;
	int speed_x, speed_y;
	int tach_x, tach_y;
	int col;

	//if (bChangeSpeed && display_type == 1)
	{
		speed_x = 8;
		speed_y = 53;

		bar_x = 0;
		bar_y = 0;

		for(i = 0; i < speedMax; i++) {
			BITMAP *bmp = panel;

			if ((i - speed) < 0) {
				col = makecol(iround(ceil(128*(float(i)/speedMax))+127), iround(ceil(64*(float(i)/speedMax))), iround(ceil(64*(float(i)/speedMax))));
				putpixel(bmp, speed_x + bar_x, speed_y + bar_y, col);
				putpixel(bmp, speed_x + bar_x + 1, speed_y + bar_y, col);
			} else {
				putpixel(bmp, speed_x + bar_x, speed_y + bar_y, 0);
				putpixel(bmp, speed_x + bar_x + 1, speed_y + bar_y, 0);
			}
			if ((i % 2) == 0)
				bar_x = -3;
			else {
				bar_x = 0;
				bar_y -= 2;
			}
		}
		bChangeSpeed = false;

	}
	//if (bChangeTach && display_type != 1)
	{
		tach_x = 56;
		tach_y = 53;

		bar_x = 0;
		bar_y = 0;

		for(i = 0; i < tachMax; i++) {
			BITMAP *bmp = panel;

			if ((i - tach) < 0) {
				col = makecol(iround(ceil(255*(float(i)/tachMax))), iround(ceil(255*(1.0-(float(i)/tachMax)))), 0);
				putpixel(bmp, tach_x + bar_x, tach_y + bar_y, col);
				putpixel(bmp, tach_x + bar_x + 1, tach_y + bar_y, col);
			} else {
				putpixel(bmp, tach_x + bar_x, tach_y + bar_y, 0);
				putpixel(bmp, tach_x + bar_x + 1, tach_y + bar_y, 0);
			}
			if ((i % 2) == 0)
				bar_x = -3;
			else {
				bar_x = 0;
				bar_y -= 2;
			}
		}
		bChangeTach = false;
	}

	return true;
	//return false;
}


REGISTER_SHIP(JadRacer)
