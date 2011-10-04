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

#ifndef __VTARGET__
#define __VTARGET__

//this file should be placed in the /other directory

/*! \brief Aim system for weapons

this object assumes that all incoming values are scaled
(velocity, range, acceleration, etc.).  so, if using a raw
number, remember to call scale_velocity, scale_range, etc
on the values going into this object.

turrets can be done by making sourceP the turret, not the
firing ship.

Angles should always be passed back and forth as degrees.
sometimes radian angles are held, but they are for internal
routine use only, not for passing back and forth.

I'm trying to make sure that any variable which is an
angle holds a degree reading unless it ends with the word
Radian in the variable name.
*/

class AimSys
{
	public:
		double pursuitAngle;
		SpaceLocation* source;
		SpaceLocation* target;
		/// \brief weaponOffsetX is for the X coord offset,
		/// for guns that are not on the center.  Use negative values for left,
		/// positive values for right.  Not yet implemented. (it's messy)
		double weaponOffsetX;
		/// \brief weaponOffsetY is to adjust this if the gun is mounted forward
		/// or backward of the center.  Most guns are mounted forward
		/// of the center.  Use positive values for forward, negative
		/// values for backward.
		double weaponOffsetY;
		/// \brief weaponAngle is for weapons that do not fire straight forward
		/// along the facing of the ship.
		double weaponAngle;
		double relativity;
		/// \brief degTolerance is the size of the window that will allow firing.
		/// degTolerance of zero means a very narrow window is used.
		double degTolerance;
		/// \brief lagSetting is T/F.  if false, it is set to err on the side of
		/// anticipation.  If true, it is set to err on the side of lagging
		/// behind.
		double lagSetting;
		double maxRange;
								 //nx1 ny1 for firing ship.
		double nx1, ny1, nx2, ny2;
		double nx1center, ny1center;
		double alphaDegree, thetaDegree, alphaRadian, thetaRadian;
		double alphaDegOld;
								 //X,Y velocities vx/y1 for firing ship, vx/y2 for target.
		double vx1, vy1, vx2, vy2;
								 // the relativistic vector
		double vxRel, vyRel, vRel;
		double weaponSpeed, targetSpeed, relSpeed, distance;
		double RawDistance(double x1, double y1, double x2, double y2);
		double vectorAngle, vectorAngleRadians, beta;
		double relVectorAngle, relVectorAngleRadians;
		double bestTrialAlpha, bestTrialTheta, bestTrialEpsilon;
		double bestTrialBeta, bestTrialBetaPrime;
		double trialDistance[9];
		double trialShotDistance[9];
		double trialAlpha[9];
		double trialAlphaRad[9];
		double trialBeta[9];	 // used for rel adjust
		double trialTheta[9];
		double trialEpsilon[9];
		double trialAlphaPrime[9];
		double trialBetaPrime[9];// used for rel adjust
		double trialThetaPrime[9];
		double trialSinAlpha[9];
		double X2[9]; double Y2[9];
		double gamma;

		void SetupDefaults(void);
		AimSys(SpaceLocation* sourceP,
			SpaceLocation* targetP,
			double weaponSpeed, double Relativity,
			double WeaponOffsetX,
			double WeaponOffsetY, double WeaponAngle,
			double MaxRange,
			double degTolerance, int lagSetting);
		AimSys(SpaceLocation* source, SpaceLocation* target, double weaponSpeed, double Relativity);
		AimSys(SpaceLocation* sourceP, SpaceLocation* targetP, double weaponSpeed);
		AimSys(SpaceLocation* sourceP, SpaceLocation* targetP);
		double CalcVectorAngle(double x1, double y1, double x2, double y2);
		double CalcVectorAngle(void);
		int CalcTrialValues(void);
		void Update(void);
		void setNewTarget(SpaceObject* newTarget);
		double getPursuitAngle(void);
		double getAngleOfShortestDistance(void);
		double getBestTrialAlpha(void);
		double getBestTrialTheta(void);
		int shouldFireNow(void);
		int shouldTurnLeft(void);
		int shouldTurnRight(void);
		double absAngleDifference(double A1, double A2);
		void CalcWeaponOffset(void);
		void CalcWeaponOffset(double xOff, double yOff);
		void SetSpaceLine(SpaceLine* SL);

};
#endif							 // __VTARGET__
