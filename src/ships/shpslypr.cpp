/*
This file is part of "TW-Light"
					http://tw-light.appspot.com/
Copyright (C) 2001-2004  TimeWarp development team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "ship.h"

#include "frame.h"
REGISTER_FILE

class   SlylandroLaserNew;

class SlylandroProbe : public Ship
{
	public:
		int frame;
		int thrustActive;
		int thrustForward;
		int segments;
		int segment_length, segment_dispersion;
		int rnd_angle, aiming, dispersion;

		SlylandroLaserNew   *SlyLaser;

		double realturnstep;

		int sprite_index2;

		int last_turn_left, last_turn_right;

	public:
		SlylandroProbe(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		Color crewPanelColor(int k = 0);

		virtual void calculate_thrust();
		virtual void calculate_turn_left();
		virtual void calculate_turn_right();
		virtual void calculate_fire_special();
		virtual void calculate_hotspots();
		virtual int activate_weapon();
		virtual void calculate();
		virtual int accelerate_gravwhip(SpaceLocation *source, double angle, double velocity, double max_speed);

};

static const int Ndirections = 16;

int SlylandroProbe::accelerate_gravwhip(SpaceLocation *source, double angle, double velocity,
double max_speed)
{
	STACKTRACE;
	if (source == this)
		return Ship::accelerate(source, angle, velocity, max_speed);
	return false;
}


const int MAXsegs = 8;
const int MAXlines = 1;			 // yeah, there is only 1 lightning active at the time !!

struct lightsegstr
{
	//double	x1, y1, x2, y2;
	Vector2 pos1, pos2;
	int     color;
};

class SlylandroLaserNew : public Presence
{
	SpaceLocation   *mother, *target;
	double  max_length, lifetime, existtime, seglength, perturbamount;
	int     nseg[MAXlines], lastnseg[MAXlines];
	int     newlifeframe, oldlifeframe;
	lightsegstr     lightseg[MAXlines][MAXsegs];
	int     defaultcolor[MAXlines];
	double  damage_delay[MAXlines], damage_delay_total[MAXlines];

	public:
		SlylandroLaserNew(SpaceLocation *lroot, SpaceLocation *ltarget);

		virtual void calculate();
		virtual void animate(Frame *frame);
		//virtual void inflict_damage();
};

SlylandroProbe::SlylandroProbe(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code),
frame(0),
thrustActive(FALSE),
thrustForward(TRUE)
{
	STACKTRACE;
	sprite_index2 = random() & 63;
	segment_length = tw_get_config_int("Weapon", "SegmentLength",0);
	segment_dispersion = tw_get_config_int("Weapon", "SegmentLengthDispersion",0);
	segments = tw_get_config_int("Weapon", "Segments", 0);
	rnd_angle = tw_get_config_int("Weapon", "RandomAngle", 0);
	aiming = tw_get_config_int("Weapon", "Aiming", 0);
	dispersion = tw_get_config_int("Weapon", "Dispersion", 0);

	angle = (PI2/Ndirections) * int(0.5 + (Ndirections * angle) / PI2 );
	realturnstep = turn_step;

	SlyLaser = 0;

	last_turn_left = turn_left;
	last_turn_right = turn_right;
}


Color SlylandroProbe::crewPanelColor(int k)
{
	STACKTRACE;
	Color c = {64,64,64};
	return c;
}


void SlylandroProbe::calculate_turn_left()
{
	STACKTRACE;
	if ( turn_left ) {
		if (!last_turn_left)
								 //Ndirections;
			realturnstep = -PI2 / 16;
		else
			realturnstep -= turn_rate * frame_time;
	}

	/*
	if (!last_turn_left)
	{
		// turn at once ! This should make it look "jerky" since it responds
		// immediately to a keypress; no delays !

		realturnstep = -PI2 / Ndirections;
	} else

	// turnstep is the angle, really.
	if (turn_left)
		realturnstep -= turn_rate * frame_time;
		*/

	while (realturnstep < 0) {
		realturnstep += PI2 / 16;//Ndirections;
		turn_step -= PI2 / 16;	 //Ndirections;
	}

}


void SlylandroProbe::calculate_turn_right()
{
	STACKTRACE;
	if ( turn_right ) {
		if (!last_turn_right)
								 //Ndirections;
			realturnstep = PI2 / 16;
		else
			realturnstep += turn_rate * frame_time;
	}

	/*
	if (!last_turn_right)
	{
		// turn at once ! This should make it look "jerky" since it responds
		// immediately to a keypress; no delays !

		realturnstep = PI2 / Ndirections;
	} else

	if (turn_right)
		realturnstep += turn_rate * frame_time;
		*/

	while (realturnstep > 0) {
		realturnstep -= PI2 / 16;// Ndirections;
		turn_step += PI2 / 16;	 // Ndirections;
	}

}


void SlylandroProbe::calculate()
{
	STACKTRACE;
	if (!(SlyLaser && SlyLaser->exists()) )

		SlyLaser = 0;

	last_turn_left = turn_left;
	last_turn_right = turn_right;

	Ship::calculate();

	/*
	// cause jerky movement ... 16 directions available, like in star control 2.
	angle = (PI2/Ndirections) * int(0.5 + (Ndirections * realangle) / PI2 );
	double v;
	//	v = sqrt(vx*vx + vy*vy);
	v = magnitude(vel);
	//	vx = v * cos(angle*PI/180);
	//	vy = v * sin(angle*PI/180);

	vel = v * unit_vector(angle);
	// now you've jerky movement , 12 directions only.
	*/

	if (fire_special) {
		Query q;
		for (q.begin(this, bit(LAYER_CBODIES), 100);q.current;q.next()) {
			if (q.current->isAsteroid() && q.current->canCollide(this)) {
				if (damage(q.current, 1)) {
					batt = batt_max;
					play_sound2(data->sampleSpecial[0]);
				}
			}
		}
		q.end();
	}

	frame+= frame_time;
	if (frame >= 50) {
		frame -= 50;
		if (thrustForward) {
			sprite_index2++;
			if (sprite_index2 == 64)
				sprite_index2 = 0;
		} else {
			sprite_index2--;
			if (sprite_index2 == -1)
				sprite_index2 = 63;
		}
	}
	sprite_index = sprite_index2;

}


void SlylandroProbe::calculate_hotspots()
{
	STACKTRACE;
}


void SlylandroProbe::calculate_fire_special()
{
	STACKTRACE;
	return;
}


void SlylandroProbe::calculate_thrust()
{
	STACKTRACE;
	if (thrust && !thrustActive) {
		angle = angle + PI;
		if (angle > PI2) angle -= PI2;
	}
	thrustActive = thrust;
	thrust = true;
	Ship::calculate_thrust();
	thrust = thrustActive;
	return;
}


int SlylandroProbe::activate_weapon()
{
	STACKTRACE;

	SpaceLocation *t = NULL;
	double r = 99999;
	int i;
	for (i = 0; i < targets->N; i += 1) {
		SpaceObject *s = targets->item[i];
		if (s && s->exists() && control->valid_target(s) && (distance(s) < r)) {
			t = s;
			r = distance(t);
		}
	}
	//add(new SlylandroLaser( this, this, t, segments, segment_length,
	//                                     segment_dispersion, rnd_angle, aiming, dispersion));

	if (!(SlyLaser && SlyLaser->exists()) ) {
		SlyLaser = new SlylandroLaserNew( this, t );
		game->add( SlyLaser );
		return TRUE;
	} else
	return FALSE;

}


// ALTERNATIVE FOR THE SLYLANDRO LASER

SlylandroLaserNew::SlylandroLaserNew(SpaceLocation *lroot, SpaceLocation *ltarget)
{
	STACKTRACE;

	max_length = 400.0;
	existtime = 1200.0;			 // in milliseconds ?
	seglength = 40.0;
	perturbamount = 15.0;

	mother = lroot;
	target = ltarget;

	lifetime = 0;
	for ( int iline = 0; iline < MAXlines; ++iline ) {
		nseg[iline] = 0;
		damage_delay_total[iline] = 500.0 + tw_random(500);
		damage_delay[iline] = 0.0;
	}

								 // white
	defaultcolor[0] = tw_makecol(200,200,200);
								 // blue
	defaultcolor[1] = tw_makecol(  0,  0,255);

	newlifeframe = -1;			 // nothing yet: the first beam will be calculated.
}


void SlylandroLaserNew::calculate()
{
	STACKTRACE;
	lifetime += frame_time;

	if ( lifetime > existtime || !(mother && mother->exists()) ) {
		state = 0;
		mother = 0;				 // needed, next iteration mother may be really removed.
		return;
	}

	Presence::calculate();

	double
								 // a value increasing from 0 to 1
		relativelifetime = lifetime / existtime;

	Vector2 S, D;

	S = mother->pos;

	int directedbeam;
	if (!(target && target->exists()))
		directedbeam = 0;
	else
		directedbeam = 1;

	if ( directedbeam )
		D = min_delta(target->normal_pos(), mother->normal_pos(), map_size);
	else
		D = Vector2(1,1);		 // just some arbitrary value, so that the math works at least

	double R;
	R = magnitude(D);

	double angle_toenemy;
	angle_toenemy = atan(D);

	int iline;

	SpaceSprite *tsprite;
	if ( directedbeam )
		tsprite = target->ship->get_sprite();
	else
		tsprite = 0;

	int totdamage;

	double abeams;
	abeams = 0;

	int recalclights;
	oldlifeframe = newlifeframe;
								 // about 8 frame or something
	newlifeframe = int(0.5 + relativelifetime * 16);

	if ( newlifeframe != oldlifeframe )
		recalclights = 1;
	else
		recalclights = 0;

	for ( iline = 0; iline < MAXlines; ++iline ) {
		totdamage = 0;

		lightsegstr *lights;
		lights = lightseg[iline];

		// calculate the length (= number of segments):

		if ( recalclights ) {
			lastnseg[iline] = nseg[iline];
			nseg[iline] = iround(1 + MAXsegs * (-0.00001 + sin(relativelifetime * PI)));

			if ( nseg[iline] > MAXsegs )
				nseg[iline] = MAXsegs;

			// change the color from blue to white, and back
			// 4 times change color, or so ?

			int col = int( 255 * (0.8+0.2*sin(24*relativelifetime * PI)) );
			if ( col < 250 )
				defaultcolor[iline] = tw_makecol(0, 0, col);
			else
				defaultcolor[iline] = tw_makecol(col, col, col);
		}

		// NOTE: the positions are relative (to above = standard!) to
		// the mother position (i.e, starting at 0 )

		// re-calculate all the node positions of the lightning:

		int i;

		for ( i = 0; i < nseg[iline]; ++i ) {
								 // the white beam
			lights[i].color = defaultcolor[iline];

			if ( recalclights ) {
				if ( i == 0 ) {
					lights[i].pos1 = Vector2(0.0, 0.0);
				} else {
					lights[i].pos1 = lights[i-1].pos2;
				}

				// move (a little) towards the enemy:
				double adirect, danoise, da;
				if ( directedbeam )
								 // perfect direction
					adirect = atan(min_delta(target->normal_pos(), mother->normal_pos() + lights[i].pos1, map_size));
				else
					adirect = 0.0;

				if ( i == 0 )
					abeams = adirect;

								 // the ideal directional change ?!
				da = adirect - abeams;
				if ( da >  PI ) da -= PI2;
				if ( da < -PI ) da += PI2;

				danoise = 0.25*PI + i*0.05*PI;
				if ( da >  danoise )    da =  danoise;
				if ( da < -danoise )    da = -danoise;

				// also a 25 % chance of going the wrong direction ?!
				double damin, damax, damid, darange;
				damin = -0.5 * da;
				damax = da;
				damid = 0.5 * (damin + damax);
				darange = fabs(damax - damin);
				if (darange < danoise)
					darange = danoise;

				if ( !directedbeam )
								 // can be anything !! totally random
					darange = PI2;

								 // add noise
				abeams += damid + (tw_random(darange)-0.5*darange);

				lights[i].pos2 = lights[i].pos1 +
					(seglength + tw_random(perturbamount)) * unit_vector(abeams);

				// check for collision ... if there's a collision of a line with the target, then ...
				// well,, then apply damage !!
				// only check, when the rays are recalculated (to simulate starcon2 rates when applying damage)
				double range = magnitude(lights[i].pos2) * 1.1;
				Query q;
				for (q.begin(mother, bit(LAYER_CBODIES)+bit(LAYER_SHOTS)+bit(LAYER_SHIPS), range); q.currento; q.next()) {
					tsprite = q.currento->get_sprite();
					int indexnum = q.currento->get_sprite_index();

					// normal_pos return normalize(pos,mapsize)

					int     x1, y1, x2, y2, dx, dy, sx, sy;
					x1 = iround(mother->pos.x + lights[i].pos1.x);
					y1 = iround(mother->pos.y + lights[i].pos1.y);
					x2 = iround(mother->pos.x + lights[i].pos2.x);
					y2 = iround(mother->pos.y + lights[i].pos2.y);

					// position of the target sprite:
					sx = iround(q.currento->pos.x);
					sy = iround(q.currento->pos.y);

					// you may have to re-locate this position, to correct for normalization ?!
					dx = iround(min_delta(sx, x1, map_size.x));
					dy = iround(min_delta(sy, y1, map_size.y));

					sx = x1 + dx;
					sy = y1 + dy;

					if ( tsprite->collide_ray(x1, y1, &x2, &y2, sx, sy, indexnum )) {
						// something here ?!
						dx = x2 - x1;
						dy = y2 - y1;
						lights[i].pos2.x = lights[i].pos1.x + dx;
						lights[i].pos2.y = lights[i].pos1.y + dy;
						nseg[iline] = i+1;

								 // need to supply mother as argument ... has the victory ditty ??
						q.currento->handle_damage( mother, 1, 0);
								 // it disappears more quickly striking ?
						lifetime += 0.4 * existtime;

						// add a sprite there
						// sparks
						// how ?! I've to make them first :(
						game->add(new Animation( mother, mother->normal_pos() + lights[i].pos2, this->mother->data->spriteWeaponExplosion, 0,
							1, 30, DEPTH_EXPLOSIONS));

						continue;
					}

				}
				q.end();

			}
		}

	}

}


// all-righty then ! now, draw the thing !

void SlylandroLaserNew::animate(Frame *frame)
{
	STACKTRACE;
	int i, iline;

	if ( !(mother && mother->exists()) )
		return;

	for ( iline = 0; iline < MAXlines; ++iline ) {
		for ( i = 0; i < nseg[iline]; ++i ) {
			lightsegstr *lights;
			lights = lightseg[iline];

			Vector2 V1, V2;
			int     color;

			// the "real" coordinates this time:
			V1 = mother->pos + lights[i].pos1;
			V2 = mother->pos + lights[i].pos2;

			color = lights[i].color;

			V1 = corner( V1, 0 );
			V2 = corner( V2, 0 );

			tw_line(frame->surface, (int)(V1.x), (int)(V1.y), (int)(V2.x), (int)(V2.y), color);
			frame->add_line((int)(V1.x), (int)(V1.y), (int)(V2.x), (int)(V2.y));
		}
	}
}


REGISTER_SHIP(SlylandroProbe)
