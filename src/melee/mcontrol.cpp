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

#include <string.h>
#include <stdio.h>
#include <allegro.h>

#include "melee.h"
REGISTER_FILE
#include "ais.h"
#include "id.h"

#include "frame.h"
#include "gui.h"

#include "mnet1.h"
#include "mview.h"				 //remove this
#include "mfleet.h"
#include "scp.h"
#include "other/dialogs.h"

enum
{
	ai_index_none = 0,
	ai_index_human,
	ai_index_moron,
	ai_index_wussie,
	ai_index_vegetable,
	ai_index_end
};

const char num_controls = 4;
static const char *gcc_sucks_dick[] =
{
	"none",
	"Human",
	"MoronBot",
	"WussieBot",
	"VegetableBot",
	NULL
};
const char **control_name = gcc_sucks_dick;

void animate_target(Frame *frame, SpaceLocation *t, int dx, int dy, int r, int c)
{
	STACKTRACE;
	return;
}


int control2number(const char *name)
{
	STACKTRACE;
	if (!name)
		return 0;
	for (int i = 0; i < num_controls+1; i += 1) {
		if (!strcmp(name, control_name[i]))
			return i;
	}
	return 0;
}


Control *getController(const char *type, const char *name, int channel)
{
	if ((channel != -1) && (channel & Game::_channel_buffered)) {
		tw_error("getController - invalid channel # %d", channel);
	}
	switch (control2number(type)) {
		case  ai_index_human:     return new ControlHuman(name, channel);
		case  ai_index_moron:     return new ControlMoron(name, channel);
		case  ai_index_wussie:    return new ControlWussie(name, channel);
		case  ai_index_vegetable: return new ControlVegetable(name, channel);
	}
	return NULL;
}


int Control::rand()
{
	STACKTRACE;
	if (channel == Game::channel_none)
		return random();
	return (::rand() ^ ((::rand() << 12) + (::rand() <<24))) & 0x7fffffff;
}


void Control::setup()
{
	STACKTRACE;
}


void Control::select_ship(Ship* ship_pointer, const char* ship_name)
{
	STACKTRACE;
	ship = ship_pointer;
	if (ship) {
		ship->control = this;
		if (temporary && (channel != Game::channel_none) && (already != 0) && (already != game->lag_frames)) {
			tw_error ("Control::select_ship - bad operation (incompatible with networking)");
		}
	}
	target_stuff() ;
	return;
}


void Control::load(const char* inifile, const char* inisection)
{
	STACKTRACE;
	return;
}


void Control::save(const char* inifile, const char* inisection)
{
	STACKTRACE;
	return;
}


SpaceLocation *Control::get_focus()
{
	STACKTRACE;
	if (ship)
		return ship->get_focus();
	else return NULL;
}


char selectPlayer[18] = "";
char selectTitleString[100] = "";

char selectShipPrompt[100] = "";

int my_bitmap_proc( int msg, DIALOG* d, int c )
{
	STACKTRACE;
	if ( msg == MSG_END && d->dp ) {
		destroy_bitmap( (BITMAP*)d->dp );
		d->dp = NULL;
	}

	if ( msg != MSG_DRAW || d->dp ) return d_bitmap_proc( msg, d, c );
	return D_O_K;
}


int my_list_proc( int msg, DIALOG* d, int c )
{
	STACKTRACE;
	int old_d1 = d->d1;
	Fleet *fleet = (Fleet*)d->dp3;
	int ret = d_list_proc2( msg, d, c );
	if ( d->d1 != old_d1 || msg == MSG_START ) {
		ShipType* type = fleet->getShipType(d->d1);

		ASSERT(type != NULL);

		selectDialog[SELECT_DIALOG_TITLE].flags |= D_DIRTY;
		sprintf(selectTitleString, "%s\n%s\n%d of %d points",
			selectShipPrompt,
			(type != NULL) ? type->name : 0,
			(type != NULL) ? type->cost : 0,
			fleet->getCost());

		BITMAP* panel = NULL;
		TW_DATAFILE* data = tw_load_datafile_object( type->data->file, "SHIP_P00_PCX" );

		if ( data ) {
			BITMAP* bmp = (BITMAP*)data->dat;
			panel = create_bitmap_ex( bitmap_color_depth(screen), bmp->w, bmp->h );
			blit( bmp, panel, 0, 0, 0, 0, bmp->w, bmp->h );
			tw_unload_datafile_object( data );
			data = tw_load_datafile_object( type->data->file, "SHIP_P01_PCX" );
			bmp = (BITMAP*)data->dat;
			blit( bmp, panel, 0, 0, 4, 65, bmp->w, bmp->h );
			tw_unload_datafile_object( data );
			color_correct_bitmap( panel, 0 );
		}

		if ( selectDialog[SELECT_DIALOG_PIC].dp ) destroy_bitmap( (BITMAP*)selectDialog[SELECT_DIALOG_PIC].dp );
		selectDialog[SELECT_DIALOG_PIC].dp = panel;
		scare_mouse();
		SEND_MESSAGE( &selectDialog[SELECT_DIALOG_PIC], MSG_DRAW, 0 );
		unscare_mouse();
	}
	return ret;
}


int Control::choose_ship(VideoWindow *window, char * prompt, Fleet *fleet)
{
	STACKTRACE;
	int ret = -1, slot = 0;
	if (fleet->getSize() == 0) {tw_error ("Empty fleet! (prompt:%s)", prompt);}
	selectDialog[SELECT_DIALOG_LIST].dp3 = fleet;

	strcpy(selectShipPrompt,prompt);

	slot = -1;
	while (!always_random) {
		while (key[KEY_ENTER] || key[KEY_SPACE]) poll_keyboard();

		ret = tw_do_dialog(window, selectDialog, SELECT_DIALOG_LIST);
		if (ret == SELECT_DIALOG_INFO) {
			ship_view_dialog(
				selectDialog[SELECT_DIALOG_LIST].d1,
				fleet
				);
			continue;
		}
		break;
	}
	if ((ret == SELECT_DIALOG_SHIP) || (ret == SELECT_DIALOG_LIST))
		slot = selectDialog[SELECT_DIALOG_LIST].d1;
	if ((ret == SELECT_DIALOG_ARANDOM) || (ret == -1)) always_random = 1;
	return(slot);
}


void Control::set_target(int i)
{
	STACKTRACE;
	if (i >= targets->N) {tw_error("oscar hamburger!!!!!!!!!");}
	if (i == -1) {
		index = i;
		target = NULL;
		return;
	}
	if (!valid_target(targets->item[i])) {tw_error("oscer hambuger");}
	index = i;
	target = targets->item[index];
	return;
}


void Control::target_stuff()
{
	STACKTRACE;
	if (index == -1) {
		if (targets->N) {
			index = random() % targets->N;
			target = targets->item[index];
			goto validate;
		} else {
			goto done;
		}
	}
	blah:
	if (index >= targets->N) {
		if (targets->N) {
			index -= 1;
			if (index < 0) index = 0;
			goto blah;
		} else {
			index = -1;
			target = NULL;
			goto change;
		}
	}
	if (target == targets->item[index]) {
		goto done;
	}
	else goto search;
	search:
	int o;
	o = index;
	for (index = 0; index < targets->N; index += 1) {
		if (targets->item[index] == target) {
			goto done;
		}
	}
	index = o;
	validate:
	if (!ship) {
		goto done;
	}
	int start;
	start = index;
	while (!valid_target(targets->item[index])) {
		index = (index + 1) % targets->N;
		if (index == start) {
			index = -1;
			target = NULL;
			goto change;
		}
	}
	target = targets->item[index];
	change:
	done:
	return;
}


void Control::calculate()
{
	STACKTRACE;

	if (!exists())
		return;

	target_stuff();

	if (ship) {
		if (!ship->exists() || (ship->death_counter != -1)) {
			//message.print(5000, 12, "Ship died in frame %d", game->frame_number);
			select_ship( NULL, NULL);
		}
		else keys = think();
	}

	if (!ship) {
		keys = 0;
		if (temporary)
			state = 0;
	}

	if (channel != Game::channel_none) {
		//prediction stuff
		_prediction_keys[_prediction_keys_index] = keys;
		_prediction_keys_index = (_prediction_keys_index + 1) & (_prediction_keys_size-1);

		//network prep for dieing (set state to unbuffering)
		if (!ship && temporary && (already > 0)) already = -already;

		//network traffic
		int lf = game->lag_frames;
		if (0) ;
		else if (already < 0) {
								 //unbuffering
			game->log->unbuffer(channel + Game::_channel_buffered, &keys, sizeof(KeyCode));
			keys = intel_ordering_short(keys);
			already += 1;
		}
		else if (already < lf) {
								 //buffering
			keys = intel_ordering_short(keys);
			game->log->buffer(channel + Game::_channel_buffered, &keys, sizeof(KeyCode));
			keys = intel_ordering_short(keys);
			already += 1;
		}
		else if (already > lf) {
								 //stupid error check
			tw_error("Control::calculate() - inconcievable!");
		}						 //stable, perform no action
		else {
			game->log_short(channel + Game::_channel_buffered, keys);
		}
	}

	return;
}


int Control::think()
{
	STACKTRACE;
	return 0;
}


char *Control::getDescription()
{
	STACKTRACE;
	return iname;
}


void Control::_event(Event *e)
{
	STACKTRACE;
	//add code for lag increase / decrease here
	return;
}


Control::Control(const char *name, int _channel) : temporary(false), target_sign_color(255),
already(0), channel(_channel), ship(NULL),
target(NULL), index(-1), always_random(0), _prediction_keys(NULL)
{
	STACKTRACE;
	id |= ID_CONTROL;
	attributes |= ATTRIB_SYNCHED;
	if (channel != Game::channel_none) {
		attributes |= ATTRIB_LOGGED;
		_prediction_keys = new KeyCode[_prediction_keys_size];
		_prediction_keys_index = 0;
		if (channel & Game::_channel_buffered) {
			tw_error("Control::Control - invalid channel!");
		}
	}
	iname = strdup(name);
}


Control::~Control()
{
	if (_prediction_keys) delete[] _prediction_keys;
}


bool Control::die()
{
	STACKTRACE;
	if (channel == Game::channel_none)
		return Presence::die();
	// controls CANNOT arbitrarily be killed off, because the deal with networking directly
	tw_error("controls cannot be killed");
	//the error can be removed eventually...
	//I just want to find out if anything is actually calling this
	//because before this function was added, that would have resulted in a desynch
	return false;
}


bool Control::valid_target(SpaceObject *t)
{
	STACKTRACE;
	// GEO: this error sometimes occur, unknown why.
	// speculation: it happened with a wasx clone; perhaps its mother died before
	// and since it shared control, and didn't check for a dead mother before the
	// ship::calculate function, it may've crashed.
	// This kinda thing may occur more often in case control is shared among objects
	if (!ship) {
		tw_error("Control::valid_target - !ship");
	}
	if (t->sameTeam(ship))
		return false;
	if (!t->exists())
		return false;
	return true;
}


void Control::animate(Frame *space)
{
	STACKTRACE;
	if (!ship)
		return;
	if (!target || target->isInvisible())
		return;
	if (!(attributes & ATTRIB_ACTIVE_FOCUS))
		return;
	if (targets->N < 3)
		return;
	if (target_sign_color == 255)
		return;

	int i = target_sign_color;
	int col = target_sign_color;
	animate_target(space, target, (i%3)*2-2, ((i/3)%3)*2-2, 140 + i, pallete_color[col]);
}
