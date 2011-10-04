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
#include <ctype.h>
#include <stdlib.h>
#include <allegro.h>

#include "melee.h"
REGISTER_FILE
#include "mfleet.h"

#include "other/twconfig.h"

#include <algorithm>
#include <functional>
#include <string>

//
static const char *_fleetsort_ini_alphabetical_default = "\255\255\255";
static const int _fleetsort_ini_numerical_default = 0x0fffffff;

static char _fleetsort_ini_section[80] = "Info";
static char _fleetsort_ini_item[80] = "";

char * Fleet::sortingMethodName[] =
{
	"Name",
	"Cost",
	"Species",
	"Ship Name",
	"Coders",
	"Origin"
};

char * Fleet::fleetCostName[] =
{
	"Small",					 //FLEET_SIZE_SMALL = 100,
	"Medium",					 //FLEET_SIZE_MEDIUM = 250,
	"Large",					 //FLEET_SIZE_LARGE = 500,
	"Huge",						 //FLEET_SIZE_HUGE = 1000,
	"Massive"					 //FLEET_SIZE_MASSIVE = 10000
};

//global variable used in at least 42 places.
//TODO remove this global variable and use proper Object-oriented techinques.
Fleet* reference_fleet = NULL;

//global function to initialize reference_fleet
//TODO remove this global function and use proper Object-oriented techinques.

void init_fleet()
{
	STACKTRACE;
	if (reference_fleet)
		return;
	reference_fleet = new Fleet();
	int i;
	for(i = 0; i < num_shiptypes; i++)
		reference_fleet->addShipType( &shiptypes[i] );

	reference_fleet->Sort();
}


Fleet::Fleet()
{
	STACKTRACE;
	cost = 0;
	maxFleetCost = (FleetCost)FLEET_COST_DEFAULT;
	memset(title, '\0', MAX_TITLE_LENGTH);
};

void Fleet::reset()
{
	STACKTRACE;
	ships.clear();
	this->cost = 0;
	memset(title, '\0', MAX_TITLE_LENGTH);
}


void * Fleet::serialize (int *psize)
{
	unsigned char buffy[65536];
	int s = 0;
	int j;
	j = intel_ordering(getSize());
	memcpy(&buffy[s], &j, sizeof(j)); s += sizeof(j);

	MyFleetListType::iterator iter;

	for (iter = ships.begin(); iter != ships.end(); iter++) {
		char k = strlen((*iter)->id);
		if (k > 64)
			{tw_error("serialize_fleet - that's a hell of a long ship id");}
			memcpy(&buffy[s], &k, sizeof(k)); s += sizeof(k);
	}

	for (iter = ships.begin(); iter != ships.end(); iter++) {
		memcpy(&buffy[s], (*iter)->id, strlen((*iter)->id)); s += strlen((*iter)->id);
	}

	unsigned char *holder = (unsigned char *) malloc(s);
	memcpy(holder, buffy, s);
	*psize = s;
	return holder;
}


#define READ(a) if (int(s+sizeof(a))>psize) {delete k;tw_error ("deserialize_fleet - bad!");}memcpy(&a, &buffy[s], sizeof(a)); s += sizeof(a);
#define READ2(a,b) if (b+s>psize) {delete k;tw_error ("deserialize_fleet - bad!");}memcpy(&a, &buffy[s], b); s += b;

void Fleet::deserialize(void *data, int psize)
{
	STACKTRACE;
	unsigned char *buffy = (unsigned char *) data;
	int s = 0;
	int j;
	unsigned char *k = NULL;

	reset();
	READ(j);

	int _fleet_size = intel_ordering(j);
	if (_fleet_size > MAX_FLEET_SIZE)
		{tw_error("fleet too large! (%d ships)", _fleet_size);}
		k = new unsigned char[_fleet_size];
	for (j = 0; j < _fleet_size; j += 1) {
		READ(k[j]);
		if (k[j] > MAX_SHIP_ID) {
			tw_error ("deserialize_fleet - that's a long shipid! (%d)", k[j]);
		}
	}
	int s1 = s;

	char sname[64];
	for (j = 0; j < _fleet_size; j += 1) {
		READ2(sname, k[j]);
		sname[k[j]] = '\0';
		ShipType *t = shiptype(sname);
		if (!t) {
			// extra debug info :
			FILE *f;
			f = fopen("error2.log", "wt");
			fprintf(f, "j = %i  size = %i\n", j, _fleet_size);

			s = s1;
			int n;
			for (n = 0; n < _fleet_size; n += 1) {
				READ2(sname, k[n]);
				sname[k[n]] = '\0';
				fprintf(f, "%s\n", sname);
			}

			fclose(f);
			tw_error("deserialize fleet - bad shiptype (%s)", sname);
		}
		else addShipType(t);
	}
	if (s != psize)   {tw_error("deserialize_fleet - didn't use all the data...");}
	if (k)
		delete [] k;
	return;
}


#undef READ
#undef READ2

int Fleet::addShipType(ShipType * type)
{
	STACKTRACE;
	if ( (getSize() >= MAX_FLEET_SIZE) || (type == NULL))
		return -1;

	cost += type->cost;
	ships.push_back(type);

	return ships.size()-1;
}


void Fleet::addFleet(Fleet * fleetToAdd)
{
	STACKTRACE;
	for (int i=0; i<fleetToAdd->getSize(); i++)
		addShipType(fleetToAdd->getShipType(i));
}


void Fleet::clear_slot (int slot)
{
	if ( (slot >= getSize()) || (slot<0) )
		return;
	cost -= ships[slot]->cost;
	ships.erase( ships.begin() + slot );

}


ShipType * Fleet::getShipType(int slot)
{
	STACKTRACE;
	if ( (slot<0) || (slot>=(int)ships.size()))
		return NULL;

	return ships[slot];
}


void Fleet::save(const char *filename, const char *section)
{
	STACKTRACE;
	int count = 0;
	char slot_str[8];

	std::sort(ships.begin(), ships.end());

	if (filename)
		tw_set_config_file(filename);

	for (MyFleetListType::iterator iter = ships.begin(); iter != ships.end(); iter++) {
		if (!(*iter)) {tw_error("trying to save invalid ship type in fleet");}
		sprintf(slot_str, "Slot%d", count);
		set_config_string(section, slot_str, (*iter)->id);
		count ++;
	}

	set_config_int(section, "Size", count);
	set_config_string(section, "Title", title);
	set_config_int(section, "MaxFleetCost", getMaxCost());
}


void Fleet::load(const char *filename, const char *section)
{
	STACKTRACE;
	int i, count;
	ShipType *type;
	char slot_str[8];
	const char *slot_id, *_fleet_title;

	reset();

	if (filename) tw_set_config_file(filename);
	int _fleet_size = get_config_int(section, "Size", 0);
	_fleet_title = get_config_string(section, "Title", "");
	//sprintf(title, _fleet_title);	this is a bit dangerous
	strcpy(title, _fleet_title);
	maxFleetCost = (FleetCost) get_config_int(section, "MaxFleetCost", FLEET_COST_DEFAULT);

	count = 0;
	for(i = 0; i < _fleet_size; i++) {
		sprintf(slot_str, "Slot%d", i);
		slot_id = get_config_string(section, slot_str, "?????");
		type = shiptype(slot_id);
		if (type) {
			addShipType(type);
			count++;
		}
	}
	return;
}


/// \brief Config Sort function
static bool _CharConfigAscendingP(ShipType * x, ShipType * y)
{
	STACKTRACE;
	if (!x || !y)
		return false;

	tw_set_config_file(x->file);
	char * tmp1 = strdup(get_config_string(_fleetsort_ini_section,
		_fleetsort_ini_item,
		_fleetsort_ini_alphabetical_default));
	tw_set_config_file(y->file);
	char * tmp2 = strdup(get_config_string(_fleetsort_ini_section,
		_fleetsort_ini_item,
		_fleetsort_ini_alphabetical_default));
	if (!tmp1 || !tmp2)
		return false;

	bool result = (strncmp(tmp1,tmp2,80) > 0);
	free(tmp1);
	free(tmp2);

	return result;
}


static bool _IntConfigAscendingP(ShipType * x, ShipType * y)
{
	STACKTRACE;
	if (!x || !y)
		return false;

	tw_set_config_file(x->file);
	int tmp1 = get_config_int(_fleetsort_ini_section,
		_fleetsort_ini_item,
		_fleetsort_ini_numerical_default);
	tw_set_config_file(y->file);
	int tmp2 = get_config_int(_fleetsort_ini_section,
		_fleetsort_ini_item,
		_fleetsort_ini_numerical_default);
	if (!tmp1 || !tmp2)
		return false;

	return (tmp1 > tmp2);
}


/// \brief Sort object
struct _NumericConfigAscending : public std::binary_function<ShipType *, ShipType *, bool>
{
	bool operator()(ShipType * x, ShipType * y) {
		return _IntConfigAscendingP(x, y);
	}
};

/// \brief Sort Object
struct _NumericConfigDescending : public std::binary_function<ShipType *, ShipType *, bool>
{
	bool operator()(ShipType * x, ShipType * y) {
		return !_IntConfigAscendingP(x, y);
	}
};

/// \brief Sort object
struct _AlphabeticConfigAscending : public std::binary_function<ShipType *, ShipType *, bool>
{
	bool operator()(ShipType * x, ShipType * y) {
		return _CharConfigAscendingP(x, y);
	}
};

/// \brief Sort object
struct _AlphabeticConfigDescending : public std::binary_function<ShipType *, ShipType *, bool>
{
	bool operator()(ShipType * x, ShipType * y) {
		return !_CharConfigAscendingP(x, y);
	}
};

struct _nameAscending : public std::binary_function<ShipType *, ShipType *, bool>
{
	bool operator()(ShipType * x, ShipType * y) {
		if (x && y)
			return strcmp(x->name, y->name) > 0;
		else
			return false;
	}
};

/// \brief Sort object
struct _nameDecending : public std::binary_function<ShipType *, ShipType *, bool>
{
	bool operator()(ShipType * x, ShipType * y) {
		if ((x) && (y))
			return strcmp(x->name, y->name) < 0;
		else
			return false;
	}
};

/// \brief Sort object
struct _costAscending : public std::binary_function<ShipType *, ShipType *, bool>
{
	bool operator()(ShipType * x, ShipType * y) {
		if (x && y)
			return (x->cost < y->cost);
		else
			return false;
	}
};

/// \brief Sort object
struct _costDecending : public std::binary_function<ShipType *, ShipType *, bool>
{
	bool operator()(ShipType * x, ShipType * y) {
		if (x && y)
			return (x->cost > y->cost);
		else
			return false;
	}
};

/// \brief Sort object
struct _originAscending : public std::binary_function<ShipType *, ShipType *, bool>
{
	bool operator()(ShipType * x, ShipType * y) {
		if (x && y)
			return (x->origin < y->origin);
		else
			return false;
	}
};

/// \brief Sort object
struct _originDecending : public std::binary_function<ShipType *, ShipType *, bool>
{
	bool operator()(ShipType * x, ShipType * y) {
		if (x && y)
			return (x->origin > y->origin);
		else
			return false;
	}
};

void Fleet::Sort(SortingMethod sortMethod, bool ascending, int startIndex, int endIndex)
{
	STACKTRACE;
	MyFleetListType::iterator _begin, _end;
	int _size = ships.size();

	if (startIndex < 0) startIndex = 0;
	if (startIndex >= _size) startIndex = _size;
	if ( (endIndex < 0) && (endIndex != -1) ) endIndex = 0;
	if (endIndex > _size) endIndex = _size;

	if  ( (startIndex > endIndex) && (endIndex != -1) ) {
		int temp = endIndex;
		endIndex = startIndex;
		startIndex = temp;
	}

	if (startIndex == endIndex)
		return;

	if (startIndex == 0)
		_begin = ships.begin();
	else
		_begin = ships.begin() + startIndex;

	if (endIndex == -1)
		_end = ships.end();
	else
		_end = ships.begin() + endIndex;

	switch (sortMethod) {
		case SORTING_METHOD_COST:
			if (ascending)
				std::sort(_begin, _end, _costAscending());
			else
				std::sort(_begin, _end, _costDecending());
			break;

		case SORTING_METHOD_NAME1:
			strcpy(_fleetsort_ini_item, "Name1");
			if (ascending)
				std::sort(_begin, _end, _AlphabeticConfigAscending());
			else
				std::sort(_begin, _end, _AlphabeticConfigDescending());
			break;

		case SORTING_METHOD_NAME2:
			strcpy(_fleetsort_ini_item, "Name1");
			if (ascending)
				std::sort(_begin, _end, _AlphabeticConfigAscending());
			else
				std::sort(_begin, _end, _AlphabeticConfigDescending());
			break;

		case SORTING_METHOD_CODERS:
			strcpy(_fleetsort_ini_item, "Coders");
			if (ascending)
				std::sort(_begin, _end, _AlphabeticConfigAscending());
			else
				std::sort(_begin, _end, _AlphabeticConfigDescending());
			break;

		case SORTING_METHOD_ORIGIN:
			if (ascending)
				std::sort(_begin, _end, _originAscending());
			else
				std::sort(_begin, _end, _originDecending());
			break;

		case SORTING_METHOD_NAME:
		default:
			if (ascending)
				std::sort(_begin, _end, _nameAscending());
			else
				std::sort(_begin, _end, _nameDecending());
			break;
	}
}


int Fleet::getNextFleetEntryByCharacter(unsigned int currentShip, char c)
{
	STACKTRACE;

	ASSERT(ships.at(currentShip) != NULL);
	ASSERT(currentShip < ships.size());
	ASSERT(currentShip >=0);
	c = toupper(c);

	unsigned int i;
	for (i=currentShip+1; i<ships.size(); i++) {
		MyFleetShipType temp = ships.at(i);
		ASSERT(temp!=NULL);
		ASSERT(temp->name != NULL);

		if (toupper(temp->name[0]) == c) {
			return i;
		}
	}

	for (i=0; i<currentShip; i++) {
		MyFleetShipType temp = ships.at(i);
		ASSERT(temp!=NULL);
		ASSERT(temp->name != NULL);

		if (temp->name[0] == c) {
			return i;
		}
	}
	return currentShip;
}
