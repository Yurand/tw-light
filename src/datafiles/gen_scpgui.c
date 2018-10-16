 /* This is generated file, do not edit, you changes will be lost 

Run: gen_datafiles_interface.py script to update it
*/

#include <stdio.h>

int datafile_scpgui_index(const char* str) {
	if (strcmp(str, "LOGO_BMP") == 0)
		return 0;
	if (strcmp(str, "TITLE_BMP") == 0)
		return 1;
	if (strcmp(str, "MENUACCEPT_WAV") == 0)
		return 2;
	if (strcmp(str, "MENUDISABLED_WAV") == 0)
		return 3;
	if (strcmp(str, "MENUFOCUS_WAV") == 0)
		return 4;
	if (strcmp(str, "MENUSPECIAL_WAV") == 0)
		return 5;
	if (strcmp(str, "TITLEMUSIC_WAV") == 0)
		return 6;
	return -1;
}
