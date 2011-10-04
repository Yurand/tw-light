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

#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "melee.h"

REGISTER_FILE

#include "twbutton.h"
#include "twwindow.h"

#include "twmenuexamples.h"

// make use of this textinfo box, in some "controlled" menu space:

PopupTextInfo::PopupTextInfo(EmptyButton *creator, char *ident, char *id2, int axshift, int ayshift,
FONT *afont, char *atext, int aNchar)
:
PopupT(creator, ident, axshift, ayshift)
{
	STACKTRACE;
	// add a text list box
								 //, &scroll);
	tia = new TextInfoArea(this, id2, afont, atext, aNchar);
	//tia->set_textinfo(atext, aNchar);

	char s[128];
	strcpy(s, id2);
	strcat(s, "close_");
	closebutton = new Button(this, s);

	exclusive = true;

}


PopupTextInfo::~PopupTextInfo()
{
	//if (tia)
	//	delete tia;
}


void PopupTextInfo::check_end()
{
	STACKTRACE;
	if (closebutton->flag.left_mouse_press)
		close(-1);
}


PopupTextInfo_toggle::PopupTextInfo_toggle(EmptyButton *creator, char *ident, char *id2, int axshift, int ayshift,
FONT *afont, char *atext, int aNchar)
:
PopupTextInfo(creator, ident, id2, axshift, ayshift, afont, atext, aNchar)
{
	STACKTRACE;
};

void PopupTextInfo_toggle::calculate()
{
	STACKTRACE;
	PopupTextInfo::calculate();
}


//void PopupTextInfo_toggle::check_end()
//{
//};

// This popup is invoked by a push-button somewhere. I'm not going to create some
// special push button type for that, I can just as well let this (new) class
// access the state of that general button type.

// (well, the following one isn't using a button, the one below is...)
PopupList::PopupList(BITMAP *outputscreen, const char *ident, char *id2, int axshift, int ayshift,
FONT *afont, char **aoptionslist)
:
PopupT(ident, axshift, ayshift, outputscreen)
{
	STACKTRACE;
	tbl = new TextList(this, id2, afont);
	tbl->set_optionlist(aoptionslist, makecol(0,0,0));

	exclusive = true;
}


PopupList::PopupList(EmptyButton *atrigger, const char *ident, char *id2, int axshift, int ayshift,
FONT *afont, char **aoptionslist)
:
PopupT(atrigger, ident, axshift, ayshift)
{
	STACKTRACE;
	//optionslist = aoptionslist;

	// add a text list box
	tbl = new TextList(this, id2, afont);
	tbl->set_optionlist(aoptionslist, makecol(0,0,0));

	exclusive = true;
}


PopupList::~PopupList()
{

	// delete the list
	// well, no, don't do this, cause it's a button and is deleted as part
	// of the button list in the twwindow.
	//if (tbl)
	//	delete tbl;
}


// this calls close with return value
void PopupList::check_end()
{
	STACKTRACE;
	PopupT::check_end();		 // this is also a way to end (without choosing anything)

	if (tbl->selected) {
		tbl->selected = false;
		close(tbl->getk());
	}
}


PopupFleetSelection::PopupFleetSelection(char *ident, int axshift, int ayshift,
char *datafilename, BITMAP *outputscreen,
BITMAP **alistIcon, double ascale, FONT *afont )
:
Popup(ident, axshift, ayshift, outputscreen)
{
	STACKTRACE;

	icons = new MatrixIcons(this, "icon_", KEY_ENTER);

	// this auto-configures the data as well ...
	icons->set_iconinfo(alistIcon, ascale);

	info = new TextButton(this, "info_", afont);
	oncerandom = new Button(this, "random_", KEY_R);
	alwaysrandom = new Button(this, "alwaysrandom_", KEY_A);

}


PopupFleetSelection::~PopupFleetSelection()
{
	// no ... this is deleted as part of the deletion of the button list in twwindow
	/*
	if (icons)
		delete icons;
	if (info)
		delete info;
	if (oncerandom)
		delete oncerandom;
	if (alwaysrandom)
		delete alwaysrandom;
		*/
}


// this calls close with return value
// (the general close is ok, it closes and hides the menu).
void PopupFleetSelection::check_end()
{
	STACKTRACE;
	if (icons->selected)
		close(icons->getk());
}


void PopupFleetSelection::newscan(BITMAP **alistIcon, double ascale, char *txt)
{
	STACKTRACE;
	Popup::newscan();
	icons->set_iconinfo(alistIcon, ascale);
	info->set_text(txt, makecol(255,255,0));
}


/*
// let the "main" routine determine whether to stop or not ;)
void PopupFleetSelection::check_end()
{
	STACKTRACE;
  STACKTRACE;
	if (icons->selected)
		close(icons->getk());
}
*/

// here, buttons are initialized
PopupYN::PopupYN(char *ident, int xcenter, int ycenter,
BITMAP *outputscreen,
bool inherited)
:
Popup(ident,
xcenter, ycenter,
outputscreen)
{
	STACKTRACE;
	yes = new Button(this, "yes_",KEY_Y);

	no = new Button(this, "no_", KEY_N);

	exclusive = true;
}


PopupYN::~PopupYN()
{
}


void PopupYN::check_end()
{
	STACKTRACE;
	if (yes->flag.left_mouse_press)
		close(1);
	if (no->flag.left_mouse_press)
		close(0);
}


// here, buttons are initialized
PopupOk::PopupOk(char *ident, int xcenter, int ycenter,
BITMAP *outputscreen,
bool inherited)
:
Popup(ident,
xcenter, ycenter,
outputscreen)
{
	STACKTRACE;
	ok = new Button(this, "ok_", KEY_ENTER);

	exclusive = true;
}


PopupOk::~PopupOk()
{
}


void PopupOk::check_end()
{
	STACKTRACE;
	if (ok->flag.left_mouse_press)
		close(1);
}


FileBrowser::FileBrowser(EmptyButton *creator, const char *ident, int axshift, int ayshift, FONT *afont)
:
PopupList(creator, ident, "list_", axshift, ayshift, afont, 0)
{
	STACKTRACE;
	strcpy(dir, ".");			 // you can't get below this (which is the game directory)
	strcpy(fname, "none");

	downdir = new Button(this, "downdir_");

	selection = false;

	reset_dirlist();

	required_ext[0] = 0;
}


FileBrowser::~FileBrowser()
{
	//delete downdir;
}


void FileBrowser::set_dir(char *newdir)
{
	STACKTRACE;
	strcpy(dir, newdir);
	reset_dirlist();
}


void FileBrowser::set_ext(char *ext)
{
	STACKTRACE;
	strcpy(required_ext, ".");
	strcat(required_ext, ext);
}


void FileBrowser::reset_dirlist()
{
	STACKTRACE;
	int err;
	al_ffblk info;

	tbl->clear_optionlist();

	char scanname[512];
	strcpy(scanname, dir);
	strcat(scanname, "/*");

	// first check the directories, after that the normal files.
	int i = 0;

	err = al_findfirst(scanname, &info, FA_DIREC | FA_ARCH);

	while (!err) {
		if (strcmp(info.name, ".") && strcmp(info.name, "..") ) {
			char tmp[512];
			strcpy(tmp, info.name);

			if ((info.attrib & FA_DIREC) != 0)
				strcat(tmp, "/");

			else if (required_ext[0] != 0) {
				// check if this file has a valid extension (if needed)
				char *ext2 = &tmp[strlen(tmp)-strlen(required_ext)];
				if (strcmp(required_ext, ext2))
					tmp[0] = 0;
			}

			if (tmp[0] != 0) {
				tbl->add_optionlist(tmp);

				if (i < 2048)
					fattr[i] = info.attrib;

				++i;
			}
		}

		err = al_findnext(&info);
	}

	al_findclose(&info);
}


void FileBrowser::calculate()
{
	STACKTRACE;
	PopupList::calculate();

	if (disabled)
		return;

	// go down one directory.
	if (downdir->flag.left_mouse_press) {
		char *tmp;				 // right-most char
		tmp = strrchr(dir, '/');
		if (tmp) {
			tmp[0] = 0;			 // remove that sub-dir from the string
			reset_dirlist();
		}
	}

	if (tbl->flag.left_mouse_press) {
		// change directory, OR, select a file ...

		int k;
		k = tbl->getk();

								 // if the dir is not empty...
		if (k >= 0 && tbl->optionlist) {
			if ((fattr[k] & FA_DIREC) != 0) {
				// change directory... and refresh the filelist
				strcat(dir, "/");
				strcat(dir, tbl->optionlist[k]);
								 // remove this / again, cause it's only for display
				if (dir[strlen(dir)-1] == '/')
					dir[strlen(dir)-1] = 0;
				reset_dirlist();
			} else
			// do NOT check for fa_arch here, cause a file is often not an archive
			{
				// select the file ...
				selection = true;
			}

		}
	}
}


// this calls close with return value
void FileBrowser::check_end()
{
	STACKTRACE;
	if (selection) {
		selection = false;
		int k = tbl->getk();

		// to save work elsewhere, construct the filename
		strcpy(fname, dir);
		strcat(fname, "/");
		strcat(fname, tbl->optionlist[k]);

		// also supply the filename-index separately, could be useful sometimes?
		close(k);
	}
}


void ValueStr::set(valuetypes atype, char *adescr, double amin, double amax)
{
	STACKTRACE;
	type = atype;
	strncpy(descr, adescr, sizeof(descr)-1);
	min = amin;
	max = amax;

	value = 0.0;

	if (atype == vtype_float)
		strcpy(format, "%10.2f");
	else
		strcpy(format, "%10.0f");
}


double ValueStr::getval()
{
	STACKTRACE;
	if (type == vtype_float)
		return value;
	else
		return (int)value;
}


ValueEdit::ValueEdit(TWindow *menu, char *identbranch, FONT *afont, int aNmax)
:
EmptyButton(menu, identbranch)
{
	STACKTRACE;
	Nmax = aNmax;
	values = new ValueStr* [Nmax];

	int i;
	for ( i = 0; i < Nmax; ++i ) {
		values[i] = new ValueStr();
		values[i]->descr[0] = 0; // an empty description string
								 // indicates the end of the list.
	}

	// load a text-field, edit-field, and a scrollbar

	char tmp[512];

	strcpy(tmp, identbranch);
	strcat(tmp, "info_");
	info = new TextButton(menu, tmp, afont);
	info->passive = false;

	strcpy(tmp, identbranch);
	strcat(tmp, "edit_");
	edit = new TextEditBox(menu, tmp, afont, edit_text, sizeof(edit_text));

	scroll.setup(menu, identbranch);

	isel = 0;

	barpos = -1;
	do_init = true;

}


ValueEdit::~ValueEdit()
{
	int i;
	for ( i = 0; i < Nmax; ++i )
		delete values[i];

	delete values;
}


void ValueEdit::edit_update()
{
	STACKTRACE;
	sprintf(edit->text, values[isel]->format, values[isel]->value);
}


void ValueEdit::calculate()
{
	STACKTRACE;
	EmptyButton::calculate();

	// the scrollbar:
	double oldpos = barpos;
	barpos = scroll.get_relpos();
	if (barpos > 0.999999)
		barpos = 0.999999;

	int k = 4;					 // power of increase

	if (barpos != oldpos) {
		double min = values[isel]->min;
		double max = values[isel]->max;

		double value;

		if (barpos < 1E-5)
			value = min;
		else
			value = min + (max - min) * exp(k * log(barpos));

		values[isel]->value = value;
		edit_update();
	}

	// clicking on the info button, changes the selected value that you want
	// to edit.

	int lastisel = isel;

	if (info->flag.left_mouse_press) {
		++isel;
								 // end of the list.
		if (values[isel]->descr[0] == 0 || isel >= Nmax)
			isel = 0;
	}

	if (info->flag.right_mouse_press) {
		--isel;
		if (isel < 0)
			isel = Nmax-1;

		while (values[isel]->descr[0] == 0 && isel > 0)
			--isel;
	}

	if (isel != lastisel || do_init) {
		do_init = false;		 // needed because of external manipulation of the lists
		info->set_text(values[isel]->descr, makecol(250,250,250));
		edit_update();
		//sprintf(edit->text, "Hello there !!");
		edit->text_reset();

		double min = values[isel]->min;
		double max = values[isel]->max;
		double value = values[isel]->value;
		scroll.set_percent_pos_x(exp( log( (value - min)/(max - min) ) / k));
	}
}
