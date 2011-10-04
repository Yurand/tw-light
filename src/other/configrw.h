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

#ifndef __CONFIG_RW__
#define __CONFIG_RW__

/** \brief sets the direction for all subsequent calls to conf: read from the config file, or write to the config file

 */
extern bool config_read;

/** \brief the read-setting for the config_read variable
 */
extern const bool CONFIG_READ;

/** \brief the write-setting for the config_read variable
 */
extern const bool CONFIG_WRITE;

/** \brief sets the section string for all subsequent calls to conf.

 */
extern char *section;

/** \brief reads or writes an integer value to a allegro config file. Required: config_read
and section must be propertly defined. This works for values of int, double, and string. With
this, you can write a single "config" subroutine, which reads or writes all variables based
on the config_read setting.

*/

void conf(char *id, int &x, int def = 0);

void conf(char *id, double &x, double def = 0);

void conf(char *id, char *x, char *def = "none");

/** \brief front-end for conf for writing or reading an indexed value to a config file. Indexes
are appended to a general id, starting with "1". This works for values of int, double or string.

\param id0 is the general id for accessing value x
\param i is the index for accessing value x
*/

void confnum(char *id0, int i, int &x);

void confnum(char *id0, int i, double &x);

void confnum(char *id0, int i, char *x);

/** \brief the default directory, where all "clean" data are stored. Those data define
the start of a new game.
*/

extern char *init_dir;

/** \brief the directory used to get resources from. This can point to any valid data
directory: the init directory, or a save-game directory.
*/
extern char *source_dir;

/** \brief the directory where game-data are save to
 */
extern char *target_dir;

/** \brief this defines a tw_set_config_file based on the source or target-dir, and
append the f filename to that directory.
*/
void set_conf(char *f);
#endif
