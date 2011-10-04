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

#include <assert.h>
#include "melee.h"
REGISTER_FILE
#include "id.h"

#include "melee/mframe.h"
#include "melee/mgame.h"
#include "melee/manim.h"
#include "melee/mview.h"
#include "melee/mship.h"
#include "melee/mcbodies.h"
#include "ship.h"
#include "frame.h"
#include "vtarget.h"

//this file should be placed in the /other directory
//MAX_X is the critical constant to remember when
//dealing with aliasing!

AimSys::AimSys(SpaceLocation* sourceP,
SpaceLocation* targetP,
double WeaponSpeed, double Relativity,
double WeaponOffsetX,
double WeaponOffsetY, double WeaponAngle,
double MaxRange,
double DegTolerance, int LagSetting)
{
	STACKTRACE;
	AimSys::SetupDefaults();
	source = sourceP; target = targetP;
	relativity = Relativity;
	nx1center = source->pos.x; ny1center = source->pos.y;
	nx2 = target->pos.x; ny2 = target->pos.y;
	vx1 = source->vel.x; vy1 = source->vel.y;
	vx2 = target->vel.x; vy2 = target->vel.y;
	weaponSpeed = WeaponSpeed;
	targetSpeed = sqrt(vx2 * vx2 + vy2 * vy2);
	weaponOffsetX = WeaponOffsetX;
	weaponOffsetY = WeaponOffsetY;
	weaponAngle = WeaponAngle;
	degTolerance =DegTolerance;
	lagSetting = LagSetting;
	AimSys::CalcWeaponOffset();

}


AimSys::AimSys(SpaceLocation* sourceP, SpaceLocation* targetP, double WeaponSpeed, double Relativity)
{
	STACKTRACE;
	//not anti-aliased here
	AimSys::SetupDefaults();
	source = sourceP; target = targetP;
	nx1center = source->pos.x; ny1center = source->pos.y;
	nx2 = target->pos.x; ny2 = target->pos.y;
	vx1 = source->vel.x; vy1 = source->vel.y;
	vx2 = target->vel.x; vy2 = target->vel.y;
	weaponSpeed = WeaponSpeed;
	targetSpeed = sqrt(vx2 * vx2 + vy2 * vy2);
	relativity=Relativity;
	AimSys::CalcWeaponOffset();

}


AimSys::AimSys(SpaceLocation* sourceP, SpaceLocation* targetP, double WeaponSpeed)
{
	STACKTRACE;
	//not anti-aliased here
	AimSys::SetupDefaults();
	source = sourceP; target = targetP;
	nx1center = source->pos.x; ny1center = source->pos.y;
	nx2 = target->pos.x; ny2 = target->pos.y;
	vx1 = source->vel.x; vy1 = source->vel.y;
	vx2 = target->vel.x; vy2 = target->vel.y;
	weaponSpeed = WeaponSpeed;
	targetSpeed = sqrt(vx2 * vx2 + vy2 * vy2);
	AimSys::CalcWeaponOffset();
}


AimSys::AimSys(SpaceLocation* sourceP, SpaceLocation* targetP)
{
	STACKTRACE;
	//not anti-aliased here
	//use with caution -- weapon speed is taken to be the
	//current ship speed.  Would work for the Tau Bomber.
	AimSys::SetupDefaults();
	source = sourceP; target = targetP;
	nx1center = source->pos.x; ny1center = source->pos.y;
	nx2 = target->pos.x; ny2 = target->pos.y;
	vx1 = source->vel.x; vy1 = source->vel.y;
	vx2 = target->vel.x; vy2 = target->vel.y;
								 // normally wrong!
	weaponSpeed = sqrt(vx1 * vx1 + vy1 * vy1);
	targetSpeed = sqrt(vx2 * vx2 + vy2 * vy2);
	AimSys::CalcWeaponOffset();
}


void AimSys::SetupDefaults(void) //sets up defaults
{
	relativity = 0.5;
	source = NULL; target = NULL;
	nx1center = 0; ny1center = 0;
	nx1 = 0; ny1 = 0; nx2 = 0; ny2 = 0;
	vx1 = 0; vx2 = 0; vy1 = 0; vy2 = 0;
	weaponSpeed = scale_velocity(100); targetSpeed = 0;
	weaponOffsetX = 0; weaponOffsetY = 0; weaponAngle = 0;
	lagSetting = FALSE;
	maxRange = scale_range(50);
	degTolerance = 0.1;
}


void AimSys::CalcWeaponOffset(void)
{
	STACKTRACE;

	double dx, dy;
	gamma = CalcVectorAngle(0,0,-weaponOffsetY, weaponOffsetX);
	dx = cos((gamma+source->angle) * PI / 180);
	dy = sin((gamma+source->angle) * PI / 180);
	nx1 = nx1center + weaponOffsetY * dx - weaponOffsetX * dy;
	ny1 = ny1center + weaponOffsetY * dy + weaponOffsetX * dx;
	//the above angle calculations are almost certainly mathematically
	//incorrect in x,y,sin,cosine assignments, but it works empirically
	//the errors must cancel out

}


void AimSys::CalcWeaponOffset(double xOff, double yOff)
{
	STACKTRACE;
	weaponOffsetX = xOff;
	weaponOffsetY = yOff;
	AimSys::CalcWeaponOffset();
}


void AimSys::SetSpaceLine(SpaceLine* SL)
{
	STACKTRACE;
	SL->pos.x = nx1;
	SL->pos.y = ny1;
	SL->angle = gamma+source->angle;
	//SL->length = scale_range(10);
	return;
}


double AimSys::RawDistance(double x1, double y1, double x2, double y2)
{
	STACKTRACE;
	return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2 - y1));
	//not anti-aliased here
}


double AimSys:: CalcVectorAngle(void)
{
	STACKTRACE;
	return AimSys::CalcVectorAngle(nx1, ny1, nx2, ny2);
}


double AimSys:: CalcVectorAngle(double x1, double y1, double x2, double y2)
{
	STACKTRACE;
	//angle of the line from source to target, in normal cartesian coordinates
	//zero degrees is going straight up.
	//the computer uses X+ across, Y+ down.
	//shouldn't really matter
	double dx, dy;
	dx = x2 - x1; dy = y2 - y1;
	if (dx==0 && dy==0) return(0.0);
	if (dx==0) {
		if (dy>0) return(0.0);
		else return (180.0);
	}
	if (dy==0) {
		if (dx>0) return (90.0);
		else return (270.0);
	}
	if ((dx>0)&&(dy>0))			 // cartesian quadrant 1
		vectorAngleRadians = atan(dy/dx);
	if ((dx<0)&&(dy>0))			 // cartesian quadrant 2
								 //atan(dy/dx);
		vectorAngleRadians = atan(dy/dx) + PI;
	if ((dx<0)&&(dy<0))			 // cartesian quadrant 3
		vectorAngleRadians = atan(dy/dx) + PI;
	if ((dx>0)&&(dy<0))			 // cartesian quadrant 4
		vectorAngleRadians = atan(dy/dx);
	vectorAngle = vectorAngleRadians / PI * 180;
	return(vectorAngle);
}


int AimSys::CalcTrialValues(void)
{
	STACKTRACE;
	//HERE is where the anti-ailasing happens!
	//all 8 wrap-around positions are tested, along with the non
	//wrapped one.
	//This can potentially make the AimSys aware of long 'wrap around' shots.
	int i, j, k, best = -1;
	double x;
	x = 9E30;
	for(i=-1;i<=1;i++) {
		for(j=-1;j<=1; j++) {
			k = 4 + (3 * i) + j; //subscripts
			X2[k]=nx2 + i * map_size.x;
			Y2[k]=ny2 + j * map_size.y;
			trialEpsilon[k] = CalcVectorAngle(nx1, ny1, X2[k], Y2[k]);
			trialThetaPrime[k] = CalcVectorAngle(0,0, vx2, vy2);
			trialTheta[k] = trialThetaPrime[k]-trialEpsilon[k];
			trialDistance[k] = RawDistance(nx1, ny1, X2[k], Y2[k]);
			trialBetaPrime[k] = CalcVectorAngle(0,0,vx1, vy1);
			trialBeta[k] = trialBetaPrime[k] - trialEpsilon[k];
			//questionable line below:
			trialShotDistance[k]=trialDistance[k] * (1+targetSpeed/weaponSpeed);
			trialSinAlpha[k] = (targetSpeed * sin(trialTheta[k] * PI / 180) / weaponSpeed)
				- (relSpeed * sin(trialBeta[k] * PI / 180) / weaponSpeed);
			if (trialSinAlpha[k]>-1&&trialSinAlpha[k]<1)
				trialAlphaRad[k] = asin(trialSinAlpha[k]);
			else
								 //outside the normal range of the arcsin function
				trialAlphaRad[k] = PI;
			trialAlpha[k] = trialAlphaRad[k] * 180 / PI;
			if (trialDistance[k]<x) {
				x = trialDistance[k];
				best = k;
			}
		}
	}
	assert(best!=-1&&"Error variable best supposed to be defined here");
	bestTrialEpsilon = trialEpsilon[best];
	pursuitAngle = trialAlpha[best] + trialEpsilon[best];
	bestTrialAlpha = trialAlpha[best];
	bestTrialTheta = trialTheta[best];
	bestTrialBeta = trialBeta[best];
	bestTrialBetaPrime = trialBetaPrime[best];

	//while(pursuitAngle>360) pursuitAngle -= 360;
	//while(pursuitAngle<0) pursuitAngle += 360;
	return(TRUE);
}


double AimSys::getPursuitAngle()
{
	STACKTRACE;
	return pursuitAngle;
}


double AimSys::getBestTrialAlpha()
{
	STACKTRACE;
	return bestTrialAlpha;
}


double AimSys::getBestTrialTheta()
{
	STACKTRACE;
	return bestTrialTheta;
}


double AimSys::getAngleOfShortestDistance()
{
	STACKTRACE;
	return bestTrialEpsilon;
}


void AimSys::setNewTarget(SpaceObject* newTarget)
{
	STACKTRACE;
	target = newTarget;
}


void AimSys::Update(void)
{
	STACKTRACE;
	if (source==NULL||target==NULL) return;
	nx1center = source->pos.x; ny1center = source->pos.y;
	nx2 = target->pos.x; ny2 = target->pos.y;
	vx1 = source->vel.x; vy1 = source->vel.y;
	vx2 = target->vel.x; vy2 = target->vel.y;
	targetSpeed = sqrt(vx2 * vx2 + vy2 * vy2);
	vxRel = vx1 * relativity;
	vyRel = vy1 * relativity;
	AimSys::CalcWeaponOffset();

	vRel = sqrt(vxRel * vxRel + vyRel * vyRel);
	relSpeed = sqrt(vxRel * vxRel + vyRel * vyRel);
	AimSys::CalcTrialValues();
}


double AimSys::absAngleDifference(double A1, double A2)
{
	STACKTRACE;
	double x;
	x = fabs(A2 - A1);
	if (x>180)
		return(360 - x);
	else
		return x;
}


int AimSys::shouldFireNow(void)
{
	STACKTRACE;
	// at the moment, just checks for current angle
	//within tolerance
	//the quick and dirty answer
	int i;
	int fire;
	fire = FALSE;
	for(i=0;i<9;i++) {			 //should be trialShotDistance below
		if ((trialShotDistance[i]<maxRange) &&
			((absAngleDifference(source->angle-weaponAngle,trialAlpha[i] + trialEpsilon[i])<degTolerance) ||
			(absAngleDifference(source->angle-weaponAngle,trialAlpha[i] + trialEpsilon[i])>360-degTolerance)))
			fire=TRUE;
	}							 //angle crossing logic would happen now.

	return(fire);
}


int AimSys::shouldTurnLeft(void)
{
	STACKTRACE;
	int x1, x2;
	x1 = iround(source->angle-weaponAngle);
	x2 = iround(pursuitAngle);
	while(x2<x1) x2 +=360;
								 //it's really close now.
	if (x2-x1<1||x2-x1>359) return (FALSE);
	if ((x2-x1)<180)
		return(FALSE);
	else
		return(TRUE);
}


int AimSys::shouldTurnRight(void)
{
	STACKTRACE;
	int x1, x2;
	x1 = iround(source->angle-weaponAngle);
	x2 = iround(pursuitAngle);
	while(x2<x1) x2 +=360;
								 //it's really close now.
	if (x2-x1<1||x2-x1>359) return (FALSE);
	if ((x2-x1)>180)
		return(FALSE);
	else
		return(TRUE);

}
