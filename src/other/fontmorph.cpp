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

#include "fontmorph.h"
#include "util/errors.h"

// check plugins/datfont.c for FONT routines ...

/* upgrade_to_color, upgrade_to_color_data:
 *  Helper functions. Upgrades a monochrome font to a color font.
 */
FONT_COLOR_DATA* font_upgrade_to_color_data(FONT_MONO_DATA* mf)
{
	FONT_COLOR_DATA* cf = (FONT_COLOR_DATA*) _al_malloc(sizeof(FONT_COLOR_DATA));
	BITMAP** bits = (BITMAP**) _al_malloc(sizeof(BITMAP*) * (mf->end - mf->begin));
	int i;

	cf->begin = mf->begin;
	cf->end = mf->end;
	cf->bitmaps = bits;
	cf->next = 0;

	text_mode(-1);

	for(i = mf->begin; i < mf->end; i++) {
		FONT_GLYPH* g = mf->glyphs[i - mf->begin];
		BITMAP* b = create_bitmap_ex(8, g->w, g->h);
		clear_to_color(b, 0);
		#if (ALLEGRO_VERSION == 4 && ALLEGRO_SUB_VERSION == 0)
		b->vtable->draw_glyph(b, g, 0, 0, 1);
		#else
		b->vtable->draw_glyph(b, g, 0, 0, 1, 0);
		#endif

		bits[i - mf->begin] = b;
		free(g);
	}

	free(mf->glyphs);
	free(mf);

	return cf;
}


void font_upgrade_to_color(FONT* f)
{
	STACKTRACE;
	FONT_MONO_DATA* mf = (FONT_MONO_DATA*) f->data;
	FONT_COLOR_DATA *cf, *cf_write = 0;

	if (f->vtable == font_vtable_color) return;
	f->vtable = font_vtable_color;
	//f->vtable = font_vtable_mono;

	while(mf) {
		FONT_MONO_DATA* mf_next = mf->next;

		cf = font_upgrade_to_color_data(mf);
		if (!cf_write) f->data = cf;
		else cf_write->next = cf;

		cf_write = cf;
		mf = mf_next;
	}
}


// this converts the black font into a black font with white outline.
void morph_font(FONT *f)
{
	STACKTRACE;

	font_upgrade_to_color(f);

	int c;
	BITMAP* bmp;

	for ( c = 0; c < 128; ++c ) {
		bmp = _color_find_glyph(f, c);
		if (!bmp)
			continue;

		int ix, iy;

		for ( iy = 0; iy < bmp->h; ++iy) {
			for ( ix = 0; ix < bmp->w; ++ix) {

				int col;
				col = getpixel(bmp, ix, iy);
				if (col == 0)	 // transparent
					continue;

				int s, t, H, totcol;
				H = 1;
				totcol = 0;

				for (s = -H; s <= H; ++s) {
					for (t = -H; t <= H; ++t) {
						col = getpixel(bmp, ix + s, iy + t);
								 // -1 means outside area?
						if (col != 0 && col != -1)
							totcol += 1;
					}
				}

				int k;
				if (totcol > 0 && totcol < (2*H+1)*(2*H+1))
					k = 15;
				else
					k = 255;

				putpixel(bmp, ix, iy, k);
			}
		}

	}

}
