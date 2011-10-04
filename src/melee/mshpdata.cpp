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

#include <stdio.h>
#include <string.h>
#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif

#include <map>
#include <string>

#include "melee.h"
REGISTER_FILE
#include "util/aastr.h"
#include "other/twconfig.h"

int auto_unload = false;

/*------------------------------*
 *		Ship Data Registration  *
 *------------------------------*/

int num_shipdatas = 0;

typedef std::map<std::string, ShipData*> ShipDataMap;
ShipDataMap shipdatas;

ShipData *shipdata ( const char *file )
{
	if (!file) return NULL;

	ShipDataMap::iterator shp = shipdatas.find(file);
	if (shp!=shipdatas.end())
		return (*shp).second;

	if (!exists(file)) return NULL;
	ShipData *data = new ShipData(file);
	shipdatas[file] = data;
	return data;
}


void save_spacesprite2(SpaceSprite *ss, const char *spritename, const char *destination, const char *extension)
{
	STACKTRACE;
	int i;
	char buf[512];

	if (ss->frames() != 64)
		tw_error("save_spacesprite2 - error");

	BITMAP *tmp = create_bitmap(int(ss->width() * 8), int(ss->height() * 8));
	for (i = 0; i < ss->frames(); i += 1) {
		blit(ss->get_bitmap(i), tmp, 0, 0, (i&7) * (int)ss->width(), int((i/8) * ss->height()), (int)ss->width(), (int)ss->height());
		sprintf(buf, "%s%i.%s", spritename, i, extension);
		save_bitmap(buf, tmp, NULL);
	}
	return;
}


void save_spacesprite(SpaceSprite *ss, const char *spritename, const char *destination, const char *extension)
{
	STACKTRACE;
	int i;
	char buf[512];

	if (!ss)
		return;

	if (ss->frames()) {
		for (i = 0; i < ss->frames(); i += 1) {
			if (strchr(extension, '.')) {
				sprintf(buf, "tmp/%s%03d%s", spritename, i, extension);
			} else {
				sprintf(buf, "tmp/%s%03d.bmp", spritename, i);
			}

			save_bitmap(buf, ss->get_bitmap(i), NULL);
		}

	}

	sprintf(buf, "tmp/%s.ini", spritename);
	tw_set_config_file(buf);
	set_config_string("Main", "Type", "SpaceSprite");
	set_config_int("SpaceSprite", "Number", (int)ss->frames());
	set_config_int("SpaceSprite", "Width", (int)ss->width());
	set_config_int("SpaceSprite", "Height", (int)ss->height());
	set_config_string("SpaceSprite", "SubType", "Normal");
	set_config_string("SpaceSprite", "Extension", extension);
	chdir("tmp");
	sprintf(buf, "dat ../ships/%s.dat -k -a *", destination);
	//	system(buf);
	sprintf(buf, "move * ..\\ships\\%s", destination);
	system(buf);
	chdir("..");
	return;
}


void save_samples(SpaceSprite *ss, const char *spritename, const char *destination, const char *extension)
{
	STACKTRACE;
}


void ShipData::lock()
{
	STACKTRACE;
	if (references == 0) {
		push_config_state();	 //can screw up badly if an error occurs while loading...
		load();
		pop_config_state();
	}
	references += 1;
}


void ShipData::unlock()
{
	STACKTRACE;
	references -= 1;
	if ((references == 0) && auto_unload) {
		unload();
	}
}


void unload_all_ship_data()
{
	STACKTRACE;
}


void unload_unused_ship_data()
{
	STACKTRACE;
}


void ShipData::unload()
{
	STACKTRACE;

	if (status != LOADED_FULL) return;

	if (spriteShip) {
		delete spriteShip;
		spriteShip = NULL;
	}
	if (spriteWeapon) {
		delete spriteWeapon;
		spriteWeapon = NULL;
	}
	if (spriteWeaponExplosion) {
		delete spriteWeaponExplosion;
		spriteWeaponExplosion = NULL;
	}
	if (spriteSpecial) {
		delete spriteSpecial;
		spriteSpecial = NULL;
	}
	if (spriteSpecialExplosion) {
		delete spriteSpecialExplosion;
		spriteSpecialExplosion = NULL;
	}
	if (spriteExtra) {
		delete spriteExtra;
		spriteExtra = NULL;
	}

	if (spritePanel) {
		delete spritePanel;
		spritePanel = NULL;
	}

	if (num_more_sprites) {
		int i;
		for (i = 0; i < num_more_sprites; i += 1) delete more_sprites[i];
		delete[] more_sprites;
		more_sprites = NULL;
		num_more_sprites = 0;
	}

	tw_unload_datafile(data);

	status = LOADED_NONE;
}


ShipData::ShipData(const char *filename) :
data(NULL),
spriteShip(NULL),
spriteWeapon(NULL),
spriteWeaponExplosion(NULL),
spriteSpecial(NULL),
spriteSpecialExplosion(NULL),
spriteExtra(NULL),
spriteExtraExplosion(NULL),
num_weapon_samples(0),
sampleWeapon(NULL),
num_special_samples(0),
sampleSpecial(NULL),
num_extra_samples(0),
sampleExtra(NULL),
moduleVictory(NULL)
{
	STACKTRACE;
	file = strdup(filename);
	references = 0;
	status = LOADED_NONE;
}


SpaceSprite *load_sprite(const char *string, TW_DATAFILE *data, int *index)
{
	char buffy[512]; buffy[0] = 0;
	char *cp = buffy;
	char *tp;
	int argc, i;
	int rotations = 1;
	char **argv = get_config_argv("Objects", string, &argc);
	int count = 0;
	if (!argc) return NULL;
	count = atoi(argv[0]);
	if (!count) return NULL;
	tp = strchr(argv[0], 'r');
	for (i = 1; i < argc; i += 1) {
		if ((argv[i][0] == '-') || (argv[i][0] == '+')) {
			cp += sprintf(cp, "%s ", argv[i]);
		}
		else if (argv[i][0] == 'r') tp = argv[i];
		else {tw_error("load_sprite - unrecognized modifiers '%s'", argv[i]);}
	}
	if (tp) {
		rotations = atoi(tp+1);
		if (rotations == 0) rotations = 64;
	}
	SpaceSprite *sprite = NULL;
	int attrib = string_to_sprite_attributes(buffy);
	sprite = new SpaceSprite(&data[*index], count, attrib, rotations);
	for (i = 0; i < count; i += 1) {
		destroy_rle_sprite((RLE_SPRITE*)data[(*index)+i].dat);
		data[(*index)+i].dat = NULL;
		// brutal hack to free up the memory
	}
	*index += count;
	return sprite;
}


void ShipData::load()
{
	STACKTRACE;
	int i, index = 0, count;

	if (status != LOADED_NONE) return;

	data = tw_load_datafile(file);

	if (!data)
		tw_error("Error loading '%s'", file);

	set_config_data((char *)(data[index].dat), data[index].size);

	int num_panel_bitmaps = get_config_int("Objects", "PanelBitmaps", 0);

	index++;

	// load ship panel
	if (num_panel_bitmaps < 2)
		tw_error("Too few ship panel bitmaps");
	spritePanel = new SpaceSprite(&data[index], num_panel_bitmaps, SpaceSprite::IRREGULAR);
	index += num_panel_bitmaps;

	// load ship sprites
	spriteShip = load_sprite("ShipSprites", data, &index);

	// load weapon sprites
	spriteWeapon = load_sprite("WeaponSprites", data, &index);

	// load weapon explosion sprites
	spriteWeaponExplosion = load_sprite("WeaponExplosion", data, &index);

	// load special ability sprites
	spriteSpecial = load_sprite("SpecialSprites", data, &index);

	// load special ability explosion sprites
	spriteSpecialExplosion = load_sprite("SpecialExplosion", data, &index);

	// load extra sprites
	spriteExtra = load_sprite("ExtraSprites", data, &index);

	// load extra explosion sprites
	spriteExtraExplosion = load_sprite("ExtraExplosion", data, &index);

	//load optional super-extra sprites
	i = 0;
	more_sprites = NULL;
	while (true) {
		char buffy[512];
		sprintf(buffy, "ExtraExtraSprites%d", i);
		if (get_config_int("Objects", buffy, -1) == -1) break;
		more_sprites = (SpaceSprite**) realloc(more_sprites, (i+1) * sizeof(SpaceSprite*));
		more_sprites[i] = load_sprite(buffy, data, &index);
		i += 1;
	}
	num_more_sprites = i;

	// initialize ship victory ditty
	moduleVictory = (Music *)(data[index].dat);
	index++;

	// load weapon samples
	count = get_config_int("Objects", "WeaponSamples", 0);
	num_weapon_samples = count;
	if (count > 0) {
		sampleWeapon = new SAMPLE*[count];
		for(i = 0; i < count; i++) {
			sampleWeapon[i] = (SAMPLE *)(data[index].dat);
			index++;
		}
	}
	else sampleWeapon = NULL;

	// load special ability samples
	count = get_config_int("Objects", "SpecialSamples", 0);
	num_special_samples = count;
	if (count > 0) {
		sampleSpecial = new SAMPLE*[count];
		for(i = 0; i < count; i++) {
			sampleSpecial[i] = (SAMPLE *)(data[index].dat);
			index++;
		}
	}
	else sampleSpecial = NULL;

	// load extra samples
	count = get_config_int("Objects", "ExtraSamples", 0);
	num_extra_samples = count;
	if (count > 0) {
		sampleExtra = new SAMPLE*[count];
		for(i = 0; i < count; i++) {
			sampleExtra[i] = (SAMPLE *)(data[index].dat);
			index++;
		}
	}
	else sampleExtra = NULL;

	status = LOADED_FULL;

	return;

}


ShipData::~ShipData()
{
	unload();
	free(file);
}
