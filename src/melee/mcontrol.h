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

#ifndef __MCONTROL_H__
#define __MCONTROL_H__

#include "melee.h"
#include "mframe.h"
#include "mship.h"

extern const char num_controls;
extern const char **control_name;
Control *getController(const char *type, const char *name, int channel);

class Control : public Presence
{
	public:

		/*! \brief  controls CANNOT arbitrarily be killed off, because the deal with networking directly */
		virtual bool die();

		/*! \brief true if this control will delete itself when it's ship dies */
		bool temporary;
		unsigned char target_sign_color;
		/*! \brief this pertains to network traffic - see comment above calculate() in mcontrol.cpp */
		int already;
		int channel;

		/*! \brief points at the current ship being controlled */
		Ship *ship;
		/*! \brief keys currently pressed */
		KeyCode keys;
		/*! \brief name of instance */
		char *iname;
		SpaceObject *target;
		int index;
		bool valid_target(SpaceObject *t);
		/*! \brief always-random selection, for the always-random button on the default ship selector */
		int always_random;
	public:
		/*! \brief handles camera focusing on a controls ship */
		virtual SpaceLocation *get_focus();
		int rand();
		/*! \brief loads configuration data from a ini file.
		  \example  player1.load("scp.ini", "Keyboard1"); */
		virtual void load(const char* inifile, const char* inisection);
		/*! \brief saves configuration data to a ini file. */
		virtual void save(const char* inifile, const char* inisection);
		/*! \brief presents the ship selection dialog */
		virtual int choose_ship(VideoWindow *window, char *prompt, class Fleet *fleet);
		/*! \brief called whenever the ship being controlled changes
		  WARNING: select_ship() is not thread-safe */
		virtual void select_ship(Ship* ship_pointer, const char* ship_name);
		/*! \brief called every frame of physics */
		virtual void calculate();
		/*! \brief called by calculate... this is where the important stuff goes */
		virtual int think();
		/*! \brief returns the name of the control type, like "Joystick" */
		virtual const char *getTypeName() = 0;
		/*! \brief returns the description of the control, like "NetworkGame(local)_Keyboard0" */
		virtual char *getDescription();
		/*! \brief the constructor initializes ship to NULL */
		Control (const char *name, int channel) ;
		/*! \brief the destructor, which is rarely used */
		virtual ~Control () ;
		/*! called by the GUI stuff to setup the controls (calibrate Joystick, set keys, etc.) */
		virtual void setup();
		virtual void set_target(int i);
		virtual void target_stuff();
		virtual void animate(Frame *space);

		enum {					 //must be a power of 2
			_prediction_keys_size = 128
		};
		//half-assed network bypass for prediction
		KeyCode *_prediction_keys;
		unsigned char _prediction_keys_index;

		/*! \brief for future mid-game lag changes */
		virtual void _event ( Event *e );
};

extern char selectTitleString[100];

int my_list_proc( int msg, DIALOG* d, int c );
int my_bitmap_proc( int msg, DIALOG* d, int c );
#endif							 // __MCONTROL_H__
