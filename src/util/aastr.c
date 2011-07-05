/* modified by orz for Star Control: TimeWarp
AA stretching engine was re-written to support new features
and to be understandable (for me, anyway).
*/

#define AASTR2_HW_SUPPORT_LEVEL 2

/*
 * aastr.c --- anti-aliased stretching for Allegro
 *
 * This file is gift-ware.  This file is given to you freely
 * as a gift.  You may use, modify, redistribute, and generally hack
 * it about in any way you like, and you do not have to give anyone
 * anything in return.
 *
 * I do not accept any responsibility for any effects, adverse or
 * otherwise, that this code may have on just about anything that
 * you can think of.  Use it at your own risk.
 *
 * Copyright (C) 1998, 1999  Michael Bukin
 */

#include <allegro.h>

#include <math.h>

#include "types.h"
#include "round.h"
#include "aastr.h"
#include "aautil.h"

#define bit(a) (1<<(a))
#define bitn(a) (bit(a)-1)

int _aa_mode = 0;
void aa_set_mode ( int a ) { _aa_mode = a; }
int aa_get_mode ( ) { return _aa_mode; }

int _aa_trans = 0;
int _aa_trans2 = 0;
void aa_set_trans ( int a )
{
	if (a < 0) a = 0;
	if (a > 256) a = 255;
	_aa_trans2 = a;
	if (a < 256) _aa_trans = (a<<8)/(256-a);
}


int aa_get_trans ( )
{
	return _aa_trans2;
}


void aa_set_background ( RGB rgb )
{
	_aa.background.rgb = rgb;
	return;
}


RGB aa_get_background ( )
{
	return _aa.background.rgb;
}


/*
modified Engine of anti-aliased stretching
Takes non-integer coordinates for x,y,w, & h.
Easier to read (doesn't use those evil macros).

Flaws: distorts large scaling factors
*/

void _aa_stretch_blit (BITMAP *src, BITMAP *dest,
int sx, int sy,
int sw, int sh,
int dx, int dy,
int dw, int dh,
int mode )
{
	unsigned long addr;
	int xscale, yscale, xbase, ybase;
	int y1, y2, x1, x2;
	int cx, cy, cw, ch;
	int iy, ix;
	int idx, idy;
	int mw, mh;
	ADD_TYPE *add;
	PUT_TYPE *put;

	if (((mode&~(AA_MASKED)) == AA_NO_AA) &&
	(bitmap_color_depth(src) == bitmap_color_depth(dest))) {
		//	if (0)
		if (mode & AA_MASKED)
			masked_stretch_blit(src, dest,
				sx>>aa_BITS, sy>>aa_BITS, sw>>aa_BITS,
				sh>>aa_BITS,
				dx>>aa_BITS, dy>>aa_BITS, dw>>aa_BITS,
				dh>>aa_BITS);
		else
			stretch_blit(src, dest,
				sx>>aa_BITS, sy>>aa_BITS, sw>>aa_BITS, sh>>aa_BITS,
				dx>>aa_BITS, dy>>aa_BITS, dw>>aa_BITS, dh>>aa_BITS);
		return;
	}

	if ((dx + dw < 0) || (dy + dh < 0) ||
		(dx >= dest->w << aa_BITS) || (dy >= dest->h << aa_BITS))
		return;

	if (!(mode & AA_NO_ALIGN)) {
		sx &= (sx + bit(aa_BITS-1)) &~bitn(aa_BITS);
		sy &= (sy + bit(aa_BITS-1)) &~bitn(aa_BITS);
		sw &= (sw + bit(aa_BITS-1)) &~bitn(aa_BITS);
		sh &= (sh + bit(aa_BITS-1)) &~bitn(aa_BITS);
		dx &= (dx + bit(aa_BITS-1)) &~bitn(aa_BITS);
		dy &= (dy + bit(aa_BITS-1)) &~bitn(aa_BITS);
		dw &= (dw + bit(aa_BITS-1)) &~bitn(aa_BITS);
		dh &= (dh + bit(aa_BITS-1)) &~bitn(aa_BITS);
	}

	if ((sw < (1 << (aa_BITS/2))) || (sh < (1 << (aa_BITS/2))))
		return;
	if ((dw < (1 << (aa_BITS/2))) || (dh < (1 << (aa_BITS/2))))
		return;

	yscale = ((sh) << aa_BITS) / dh;
	ybase = (((dy&bitn(aa_BITS)) * yscale) >> aa_BITS) - sy;
	xscale = ((sw) << aa_BITS) / dw;
	xbase = (((dx&bitn(aa_BITS)) * xscale) >> aa_BITS) - sx;

	add = get_aa_add_function(src, mode);
	put = get_aa_put_function(dest, mode);
	if (!add || !put) return;

	#if AASTR2_HW_SUPPORT_LEVEL >= 1
	bmp_select(dest);
	#endif

	y1 = (dy        >> aa_BITS);
	y2 = ((dy+dh-1) >> aa_BITS);
	x1 = (dx        >> aa_BITS);
	x2 = ((dx+dw-1) >> aa_BITS);
	cw = xscale;
	ch = yscale;
	idy = dy >> aa_BITS;
	idx = dx >> aa_BITS;
	mw = sx+sw;
	mh = sy+sh;
	if ((unsigned int)cw < aa_SIZE) cw = aa_SIZE;
	if ((unsigned int)ch < aa_SIZE) ch = aa_SIZE;
	_aa.total = cw * ch;
	if (!(mode & (AA_NO_ALIGN | AA_NO_FILTER))) {
		if (_aa.total > aa_MAX_NUM) {
			if ((unsigned int)cw > aa_MAX_SIZE) {
				//				xbase -= (cw - aa_MAX_SIZE) >> 1;
				//				xbase = 0;
				cw = aa_MAX_SIZE;
			}
			if ((unsigned int)ch > aa_MAX_SIZE) {
				//				ybase -= (ch - aa_MAX_SIZE) >> 1;
				//				ybase = 0;
				ch = aa_MAX_SIZE;
			}
			_aa.total = cw * ch;
		}
	}
	if (_aa_mode & AA_VFLIP) {
		ybase -= yscale * (y2-idy);
		yscale *= -1;
	}
	if (_aa_mode & AA_HFLIP) {
		xbase -= xscale * (x2-idx);
		xscale *= -1;
	}
	if (dest->clip) {
		if (y1 < dest->ct) y1 = dest->ct;
		if (y2 >= dest->cb) y2 = dest->cb - 1;
		if (x1 < dest->cl) x1 = dest->cl;
		if (x2 >= dest->cr) x2 = dest->cr - 1;
	}
	if (_aa_trans) {
		if (_aa_trans2 >= 256) return;
		_aa.total += (_aa.total * (Uint64)_aa_trans) >> 8;
	}

	if (_aa.total)
		_aa.inverse = 1 + (0xffffffffUL /  _aa.total );
	else
		_aa.inverse = 1;

	for (iy = y1; iy <= y2; iy += 1) {
		int th;
		cy = (iy-idy) * yscale - ybase;
		#if AASTR2_HW_SUPPORT_LEVEL >= 2
								 //this helps if dest is a video bitmap
		addr = bmp_write_line(dest, iy);
		#else
		addr = (int)dest->line[iy];
		#endif
		_aa.y = iy;
		th = ch;
		if (cy < 0) {			 //top edge of image
			th += cy;
			cy = 0;
		}
		if (cy + th > mh) {		 //bottom edge of image
			th = mh - cy;
		}
								 //either edge
		if ((unsigned int)th < aa_SIZE) {
			cy -= (aa_SIZE - th) / 2;
			th = aa_SIZE;
			if (cy < 0) cy = 0;
			if (cy > (int)(mh - aa_SIZE)) cy = mh - aa_SIZE;
		}
		for (ix = x1; ix <= x2; ix += 1) {
			int tw;
			cx = (ix-idx) * xscale - xbase;
			tw = cw;
			if (cx < 0) {		 //left edge of image
				tw += cx;
				cx = 0;
			}
			if (cx + tw > mw) {	 //right edge of image
				tw = mw - cx;
			}
								 //either edge of image
			if ((unsigned int)tw < aa_SIZE) {
				cx -= (aa_SIZE - tw) / 2;
				tw = aa_SIZE;
				if (cx < 0) cx = 0;
				if (cx > (int)(mw - aa_SIZE)) cx = mw - aa_SIZE;
			}
			add ((BITMAP *)src, cx, cx + tw, cy, cy + th);
			AA_PUT_PIXEL (put, addr, ix);
		}
	}
	#if AASTR2_HW_SUPPORT_LEVEL >= 2
	bmp_unwrite_line(dest);		 //this helps if dest is a video bitmap
	#endif
	return;
}


/*
 * Anti-aliased bitmap stretching with blit.
 */
void
aa_stretch_blit (BITMAP *src, BITMAP *dst,
double sx, double sy, double sw, double sh,
double dx, double dy, double dw, double dh)
{
	if (!sw || !sh) return;
	if (sx < 0) {
		double a = dw * sx / sw;
		sw += sx;
		sx = 0;
		dx = dx - a;
		dw = dw + a;
		if (sw <= 0) return;
	}
	if (sx + sw > src->w) {
		double a = (sx+sw-src->w);
		dw -= a * dw / sw;
		sw -= a;
		if (sw <= 0) return;
	}
	if (sy < 0) {
		double a = dh * sy / sh;
		sh += sy;
		sy = 0;
		dy = dy - a;
		dh = dh + a * dh;
		if (sh <= 0) return;
	}
	if (sy + sh > src->h) {
		double a = (sy+sh-src->h);
		dh -= a * dh / sh;
		sh -= a;
		if (sh <= 0) return;
	}

	if (is_video_bitmap(src))
		acquire_bitmap(src);
	if (is_video_bitmap(dst))
		acquire_bitmap(dst);
	_aa_stretch_blit (src, dst,
		iround(ldexp(sx,aa_BITS)), iround(ldexp(sy,aa_BITS)),
		iround(ldexp(sw,aa_BITS)), iround(ldexp(sh,aa_BITS)),
		iround(ldexp(dx,aa_BITS)), iround(ldexp(dy,aa_BITS)),
		iround(ldexp(dw,aa_BITS)), iround(ldexp(dh,aa_BITS)),
		_aa_mode);
	if (is_video_bitmap(src))
		release_bitmap(src);
	if (is_video_bitmap(dst))
		release_bitmap(dst);
	return;
}


/*
 * Anti-aliased bitmap stretching with blit (masked).
 */
void
aa_stretch_sprite (BITMAP *dst, BITMAP *src, double dx, double dy, double dw, double dh)
{
	if (_aa_mode & AA_NO_AA) {
		stretch_sprite(dst,src,
			iround(dx),iround(dy),iround(dw),iround(dh)
			);
		return;
	}
	_aa_stretch_blit (
		src, dst,
		0, 0, src->w << aa_BITS, src->h << aa_BITS,
		iround(ldexp(dx, aa_BITS)), iround(ldexp(dy, aa_BITS)),
		iround(ldexp(dw, aa_BITS)), iround(ldexp(dh, aa_BITS)),
		AA_MASKED | _aa_mode
		);
	return;
}


/*
 * aastr.c ends here
 */
