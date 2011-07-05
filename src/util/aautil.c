/* modified by orz for Star Control: TimeWarp
Lots of features added, large code re-organization, etc.
*/

/*
 * aautil.c --- helpers for anti-aliasing routines for Allegro
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

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "aautil.h"
#include "aastr.h"

/* Multiply b by a (0 <= a <= aa_SIZE). */
#define MUL(a, b) ((b) * (a))

struct _aa_type _aa;

#define aa_PIXEL (aa_SIZE * aa_SIZE)

/*
 * Prepare offsets for direct access to 24bpp bitmap.
 */
static void _aa_prepare_for_8bpp (void)
{
	PALETTE p;
	if (!rgb_map) {
		rgb_map = (RGB_MAP*)malloc(32768);
		get_palette(p);
		create_rgb_table ( rgb_map, p, NULL);
	}
}


static void _aa_prepare_for_24bpp (void)
{
	_aa.roffset24 = _rgb_r_shift_24 / 8;
	_aa.goffset24 = _rgb_g_shift_24 / 8;
	_aa.boffset24 = _rgb_b_shift_24 / 8;
}


/*
 * Some macros for making our put-pixel functions
 */

#define bit(a)   (1 << (a))
#define bitn(a)  (bit(a)-1)

//bpp is output size
//bpp2 is output format

//declare some variables, set up a few
#define BEGIN(bpp,bpp2)             int c, r=_aa.r, g=_aa.g, b = _aa.b;
//this macro calculates the color without dither
#define NO_DITHER(bpp,bpp2)         c = makecol##bpp2(r,g,b);

#define RAW(bpp,bpp2)               c = makeacol##bpp2(r,g,b,_aa.trans);

//these macros calculate the color with dither
#define _DITHER1()                  r += _aa.r_left; g += _aa.g_left; b += _aa.b_left;
#define _DITHER1_RAND(br,bg,bb)     r += (rand() & bitn(br))-bit(br-1);g += (rand() & bitn(bg))-bit(bg-1);b += (rand() & bitn(bb))-bit(bb-1);
#define _DITHER2()                  if (r<0) r=0; else if (r>255) r=255; if (g<0) g=0; else if (g>255) g=255; if (b<0) b=0; else if (b>255) b=255;
#define _DITHER3(bpp2)              _aa.r_left += _aa.r - getr##bpp2(c);_aa.g_left += _aa.g - getg##bpp2(c);_aa.b_left += _aa.b - getb##bpp2(c);
#define DITHER(bpp,bpp2)            {_DITHER1() _DITHER2() NO_DITHER(bpp,bpp2) _DITHER3(bpp2)}
#define OPTIONAL_DITHER(bpp,bpp2)   if (_aa.mode&AA_DITHER) DITHER(bpp,bpp2) else NO_DITHER(bpp,bpp2)

//this macro adjusts the color for alpha
#define _ALPHA(bpp2,channel)        channel += ((get##channel##bpp2(o)*_aa.trans)>>8);
#define _ALPHA_N(bpp2,channel)      channel += ((get##channel##_depth(bpp2,o)*_aa.trans)>>(aa_BITS*2));
#define ALPHA(bpp,bpp2)             if (_aa.trans) {int o = bmp_read##bpp(_addr+_x*((bpp+7)/8));_ALPHA(bpp,r) _ALPHA(bpp,g) _ALPHA(bpp,b)}
#define ALPHA_N(bpp,bpp2)           if (_aa.trans) {int o = getpixel(_aa.destination, _x, _aa.y);_ALPHA_N(bpp2,r) _ALPHA_N(bpp2,g) _ALPHA_N(bpp2,b)}
#define OPTIONAL_ALPHA(bpp,bpp2)    if (_aa.mode&AA_ALPHA) ALPHA(bpp,bpp2)

//these macros adjust the color to remove the mask color from the output
#define NO_TRANS(bpp,bpp2)   if (c == MASK_COLOR_ ## bpp2) c = NON_MASK_COLOR_ ## bpp2;
#define OPTIONAL_NO_TRANS(bpp,bpp2) if (_aa.mode&AA_NOTRANS) NO_TRANS(bpp,bpp2)

//these macros apply the blender
#define BLENDER_WITH_N(bpp,bpp2)      {c=_blender_func_##bpp2(bmp_read##bpp(_addr+_x*((bpp+7)/8)), c, (_aa.trans * _aa.blender_n) >> 8);}
#define BLENDER_WITH_A(bpp,bpp2)      {c=_blender_func_##bpp2(bmp_read##bpp(_addr+_x*((bpp+7)/8)), c, (_aa.trans * _aa.trans) >> 8);}

#define BLENDER32_WITH_N(bpp,bpp2)    {int c2;c=makecol24(r,g,b);c2=bmp_read##bpp(_addr+_x*((bpp+7)/8));c=_blender_func_24(makecol24(getr##bpp2(c2),getg##bpp2(c2),getb##bpp2(c2)), c, (_aa.trans * _aa.blender_n) >> 8);r=getr24(c);g=getg24(c);b=getb32(c);
#define BLENDER32_WITH_A(bpp,bpp2)    {int c2;c=makecol24(r,g,b);c2=bmp_read##bpp(_addr+_x*((bpp+7)/8));c=_blender_func_24(makecol24(getr##bpp2(c2),getg##bpp2(c2),getb##bpp2(c2)), c, (_aa.trans * _aa.trans) >> 8);r=getr24(c);g=getg24(c);b=getb32(c);

//these macros output the color to the screen
#define PUT(bpp,bpp2)             bmp_write##bpp(_addr+_x*((bpp+7)/8), c);
#define PUT_8X(bpp,bpp2)          outportw(0x3C4, (0x100<<(_x&3))|2); bmp_write8(_addr + (_x>>2), c);
#define PUT_N(bpp,bpp2)           putpixel(_aa.destination, _x, _aa.y, c);
//parameters ignored for 8X & N

//dummy macro
#define NONE(bpp,bpp2)

//so... should look like this:
//BEGIN
//[ALPHA | ALPHA_N | NONE | BLENDER32]
//NO_DITHER | DITHER
//BLENDER | NONE
//NO_TRANS_OUT | NONE
//PUT | PUT_8X | PUT_N

#define MAKE_PUTFUNC(name,bpp,bpp2,alpha,dither,no_trans,put)  void name (unsigned long _addr, int _x) {BEGIN(bpp,bpp2);alpha(bpp,bpp2);dither(bpp,bpp2);no_trans(bpp,bpp2);put(bpp,bpp2);}

/*
#define MAKE_PUTFUNCS_ALPHA_NOTRANS(name1,name2,bpp,dither,put)  \ 
MAKE_PUTFUNCS(name1##name2,bpp,bpp,NONE,dither,NONE,put) \ 
MAKE_PUTFUNCS(name1##_alpha##name2,bpp,bpp2,ALPHA,dither,NONE,put) \ 
MAKE_PUTFUNCS(name1##_notrans##name2,bpp,bpp2,NONE,dither,NO_TRANS,put) \ 
MAKE_PUTFUNCS(name1##_alpha_notrans##name2,bpp,bpp2,ALPHA,dither,NO_TRANS,put)
*/

/*

//name order:  alpha dither notrans modex

N rgb		ALPHA		DITHER		NOTRANS			Any-Surface
8 rgb		ALPHA		DITHER				MODE-X
15 rgb		ALPHA		DITHER		NOTRANS
16 rgb		ALPHA		DITHER		NOTRANS
24 rgb		ALPHA					NOTRANS
32 rgb		ALPHA					NOTRANS
32 rgba		*
16 rgba		*			DITHER

additional things:
alpha w/ saturation

blender with destination
blender with color
blender-32 with destination
blender-32 with color

fblend with surface
*/

#define num_bpps                      5
static char BPP_ARRAY[num_bpps] =
{
	8,
	#ifdef ALLEGRO_COLOR16
	15, 16,
	#else
	0, 0,
	#endif
	#ifdef ALLEGRO_COLOR24
	24,
	#else
	0,
	#endif
	#ifdef ALLEGRO_COLOR32
	32
	#else
	0
	#endif
};

//void (unsigned long, int)         {alpha(bpp,bpp2);dither(bpp,bpp2);no_trans(bpp,bpp2);put(bpp,bpp2);}
//macro       name                    bpp/2 ALPHA  DITHER     NOTRANS PUT

//normal modes: (all color depths)
MAKE_PUTFUNC( _aa_put_8                ,8,8,   NONE   ,NO_DITHER ,NONE   ,PUT)
MAKE_PUTFUNC( _aa_put_alpha_8          ,8,8,   ALPHA  ,NO_DITHER ,NONE   ,PUT)
MAKE_PUTFUNC( _aa_put_dither_8         ,8,8,   NONE   ,DITHER    ,NONE   ,PUT)
MAKE_PUTFUNC( _aa_put_alpha_dither_8   ,8,8,   ALPHA  ,DITHER    ,NONE   ,PUT)
//no notrans
//no alpha-notrans
//no dither-notrans
//no alpha-dither-notrans

#ifdef ALLEGRO_COLOR16
MAKE_PUTFUNC( _aa_put_15                       ,16,15, NONE   ,NO_DITHER ,NONE   ,PUT)
MAKE_PUTFUNC( _aa_put_alpha_15                 ,16,15, ALPHA  ,NO_DITHER ,NONE   ,PUT)
MAKE_PUTFUNC( _aa_put_dither_15                ,16,15, NONE   ,DITHER    ,NONE   ,PUT)
MAKE_PUTFUNC( _aa_put_alpha_dither_15          ,16,15, ALPHA  ,DITHER    ,NONE   ,PUT)
MAKE_PUTFUNC( _aa_put_notrans_15               ,16,15, NONE   ,NO_DITHER ,NO_TRANS,PUT)
MAKE_PUTFUNC( _aa_put_alpha_notrans_15         ,16,15, ALPHA  ,NO_DITHER ,NO_TRANS,PUT)
MAKE_PUTFUNC( _aa_put_dither_notrans_16        ,16,16, NONE   ,DITHER    ,NO_TRANS,PUT)
MAKE_PUTFUNC( _aa_put_alpha_dither_notrans_15  ,16,15, ALPHA  ,DITHER    ,NO_TRANS,PUT)

MAKE_PUTFUNC( _aa_put_16                       ,16,16, NONE   ,NO_DITHER ,NONE   ,PUT)
MAKE_PUTFUNC( _aa_put_alpha_16                 ,16,16, ALPHA  ,NO_DITHER ,NONE   ,PUT)
MAKE_PUTFUNC( _aa_put_dither_16                ,16,16, NONE   ,DITHER    ,NONE   ,PUT)
MAKE_PUTFUNC( _aa_put_alpha_dither_16          ,16,16, ALPHA  ,DITHER    ,NONE   ,PUT)
MAKE_PUTFUNC( _aa_put_notrans_16               ,16,16, NONE   ,NO_DITHER ,NO_TRANS,PUT)
MAKE_PUTFUNC( _aa_put_alpha_notrans_16         ,16,16, ALPHA  ,NO_DITHER ,NO_TRANS,PUT)
MAKE_PUTFUNC( _aa_put_dither_notrans_15        ,16,15, NONE   ,DITHER    ,NO_TRANS,PUT)
MAKE_PUTFUNC( _aa_put_alpha_dither_notrans_16  ,16,16, ALPHA  ,DITHER    ,NO_TRANS,PUT)
#endif

#ifdef ALLEGRO_COLOR24
MAKE_PUTFUNC( _aa_put_24                        ,24,24, NONE   ,NO_DITHER ,NONE   ,PUT)
MAKE_PUTFUNC( _aa_put_alpha_24                  ,24,24, ALPHA  ,NO_DITHER ,NONE   ,PUT)
//no dither
//no dither-alpha
MAKE_PUTFUNC( _aa_put_notrans_24                ,24,24, NONE   ,NO_DITHER ,NO_TRANS,PUT)
MAKE_PUTFUNC( _aa_put_alpha_notrans_24          ,24,24, ALPHA  ,NO_DITHER ,NO_TRANS,PUT)
//no dither-notrans
//no dither-alpha-notrans
#endif

#ifdef ALLEGRO_COLOR32
MAKE_PUTFUNC( _aa_put_32                        ,32,32, NONE   ,NO_DITHER ,NONE   ,PUT)
MAKE_PUTFUNC( _aa_put_alpha_32                  ,32,32, ALPHA  ,NO_DITHER ,NONE   ,PUT)
//no dither
//no dither-alpha
MAKE_PUTFUNC( _aa_put_notrans_32                ,32,32, NONE   ,NO_DITHER ,NO_TRANS,PUT)
MAKE_PUTFUNC( _aa_put_alpha_notrans_32          ,32,32, ALPHA  ,NO_DITHER ,NO_TRANS,PUT)
//no dither-notrans
//no dither-alpha-notrans
#endif

static PUT_TYPE *put_array[num_bpps] =
{
	&_aa_put_8,
	&_aa_put_15,
	&_aa_put_16,
	&_aa_put_24,
	&_aa_put_32
};

static PUT_TYPE *put_array_alpha[num_bpps] =
{
	&_aa_put_alpha_8,
	&_aa_put_alpha_15,
	&_aa_put_alpha_16,
	&_aa_put_alpha_24,
	&_aa_put_alpha_32
};

//dithered modes: (no 24/32)

static PUT_TYPE *put_array_dither[num_bpps] =
{
	&_aa_put_dither_8,
	&_aa_put_dither_15,
	&_aa_put_dither_16,
	&_aa_put_24,
	&_aa_put_32
};
static PUT_TYPE *put_array_alpha_dither[num_bpps] =
{
	&_aa_put_alpha_dither_8,
	&_aa_put_alpha_dither_15,
	&_aa_put_alpha_dither_16,
	&_aa_put_alpha_24,
	&_aa_put_alpha_32
};

//no transparent out modes: (no 8)

static PUT_TYPE *put_array_notrans[num_bpps] =
{
	&_aa_put_8,
	&_aa_put_notrans_15,
	&_aa_put_notrans_16,
	&_aa_put_notrans_24,
	&_aa_put_notrans_32
};

static PUT_TYPE *put_array_alpha_notrans[num_bpps] =
{
	&_aa_put_alpha_8,
	&_aa_put_alpha_notrans_15,
	&_aa_put_alpha_notrans_16,
	&_aa_put_alpha_notrans_24,
	&_aa_put_alpha_notrans_32
};

static PUT_TYPE *put_array_dither_notrans[num_bpps] =
{
	&_aa_put_dither_8,
	&_aa_put_dither_notrans_15,
	&_aa_put_dither_notrans_16,
	&_aa_put_notrans_24,
	&_aa_put_notrans_32
};

static PUT_TYPE *put_array_alpha_dither_notrans[num_bpps] =
{
	&_aa_put_alpha_dither_8,
	&_aa_put_alpha_dither_notrans_15,
	&_aa_put_alpha_dither_notrans_16,
	&_aa_put_alpha_notrans_24,
	&_aa_put_alpha_notrans_32
};

void _aa_put_raw_16a ( unsigned long _addr, int _x )
{
	bmp_write32(_addr+_x*2, makeacol16a(_aa.r, _aa.g, _aa.b, _aa.trans) );
}


void _aa_put_raw_32a ( unsigned long _addr, int _x )
{
	bmp_write32(_addr+_x*4,
		(_aa.r<<_rgb_r_shift_32) |
		(_aa.g<<_rgb_g_shift_32) |
		(_aa.b<<_rgb_b_shift_32) |
		(_aa.trans<<_rgb_a_shift_32) );
}


static PUT_TYPE **master_put_array[8] =
{
	put_array,
	put_array_alpha,
	put_array_dither,
	put_array_alpha_dither,

	put_array_notrans,
	put_array_alpha_notrans,
	put_array_dither_notrans,
	put_array_alpha_dither_notrans
};

PUT_TYPE *get_aa_put_function(BITMAP *destination, int options)
{
	int i, j, bpp;

	bmp_select(destination);	 //in case it's a video bitmap
								 //for use by N-bpp modes
	_aa.destination = destination;
	_aa.r_left = _aa.g_left =	 //reset dithering
		_aa.b_left = 0;
	_aa.transparent = 0;		 //reset transparency
	if (options & AA_BLEND)		 //disable constant background if blending
		_aa.current.i = 0;
	else _aa.current.i = _aa.background.i;

	bpp = bitmap_color_depth(destination);

	if (bpp == 0) return NULL;
	else if (bpp ==  8) _aa_prepare_for_8bpp();
	else if (bpp == 24) _aa_prepare_for_24bpp();

	for (i = 0; i < num_bpps; i += 1) if (BPP_ARRAY[i] == bpp) break;
	if (i == num_bpps) return NULL;

	j = 0;
	if (options & AA_BLEND) j += 1;
	if (options & AA_DITHER)  j += 2;
	if (options & AA_MASKED_DEST) j += 4;

	#if 0
	#   ifdef GFX_MODEX
	if (is_planar_bitmap(destination)) {
		if (bpp != 8) return NULL;
		return put_array_modex[j & 3];
	}
	#   endif
	#endif

	if (options & AA_RAW_ALPHA) {
		if (bpp == 32) return &_aa_put_raw_32a;
		if (bpp == 16) return &_aa_put_raw_16a;
	}

	return master_put_array[j][i];
}


void _aa_masked_add_bpp_independant_calculations()
{
	if (_aa.total-(_aa.trans>>8) <= 1) {
		_aa.transparent = -1;
		return;
	}
	else _aa.transparent = 0;

	if (_aa.current.i) {
		_aa.r += _aa.current.rgba.r * (_aa.trans>>8);
		_aa.g += _aa.current.rgba.g * (_aa.trans>>8);
		_aa.b += _aa.current.rgba.b * (_aa.trans>>8);
	}
	if (_aa.total == aa_PIXEL) {
		_aa.r = _aa.r >> (2 * aa_BITS);
		_aa.g = _aa.g >> (2 * aa_BITS);
		_aa.b = _aa.b >> (2 * aa_BITS);
		_aa.trans = _aa.trans >> (2 * aa_BITS);
	} else {
		/*		tmp = 1 + (0xffffffffUL /  _aa.total );
				_aa.r = (int)((_aa.r * (Uint64)tmp) >> 32);
				_aa.g = (int)((_aa.g * (Uint64)tmp) >> 32);
				_aa.b = (int)((_aa.b * (Uint64)tmp) >> 32);
				_aa.trans = (int)((_aa.trans * (Uint64)_aa.inverse) >> 32);*/
		_aa.r = (int)((_aa.r * (Uint64)_aa.inverse) >> 32);
		_aa.g = (int)((_aa.g * (Uint64)_aa.inverse) >> 32);
		_aa.b = (int)((_aa.b * (Uint64)_aa.inverse) >> 32);
		_aa.trans = (int)((_aa.trans * (Uint64)_aa.inverse) >> 32);
	}
}


void aa_put_dummy(unsigned long _addr, int _x)
{
}


void aa_add_dummy(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2)
{
}


#ifdef ALLEGRO_COLOR32

//32 bit color with alpha channel, NOT premultiplied, probably buggy
void
_aa_add_rgba32 (BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2)
{
	unsigned long *sline;
	int sx, sx1i, sx1f, sx2i, sx2f;
	int sy, sy1i, sy1f, sy2i, sy2f;
	unsigned long r1, g1, b1, a1, t1, ta;
	unsigned long r2, g2, b2, a2, t2;
	long scolor;

	sy1i = _sy1 >> aa_BITS;
	sy = sy1i;

	// First line.
	sx1i = _sx1 >> aa_BITS;
	sx = sx1i;
	sline = (unsigned long*) (_src->line[sy]) + sx;

	sx1f = aa_SIZE - (_sx1 & aa_MASK);
	scolor = *sline;
	if (scolor != _aa.mask_color) {
		ta = geta32(scolor);
		r1 = MUL (getr32 (scolor), sx1f * ta);
		g1 = MUL (getg32 (scolor), sx1f * ta);
		b1 = MUL (getb32 (scolor), sx1f * ta);
		a1 = MUL (ta, sx1f);
		t1 = 0;
	} else {
		r1 = g1 = b1 = a1 = 0;
		t1 = sx1f;
	}

	sx2i = _sx2 >> aa_BITS;
	for (sline++, sx++; sx < sx2i; sline++, sx++) {
		scolor = *sline;
		if (scolor != _aa.mask_color) {
			ta = geta32(scolor);
			r1 += (ta*getr32 (scolor)) << aa_BITS;
			g1 += (ta*getg32 (scolor)) << aa_BITS;
			b1 += (ta*getb32 (scolor)) << aa_BITS;
			a1 += (ta) << aa_BITS;
		}
		else t1 += aa_SIZE;
	}

	sx2f = _sx2 & aa_MASK;
	if (sx2f != 0) {
		scolor = *sline;
		if (scolor != _aa.mask_color) {
			ta = geta32(scolor);
			r1 += MUL (getr32 (scolor), sx2f*ta);
			g1 += MUL (getg32 (scolor), sx2f*ta);
			b1 += MUL (getb32 (scolor), sx2f*ta);
			a1 += MUL (ta, sx2f);
		}
		else t1 += sx2f;
	}

	sy1f = aa_SIZE - (_sy1 & aa_MASK);
	r1 = MUL (r1, sy1f) >> 8;
	g1 = MUL (g1, sy1f) >> 8;
	b1 = MUL (b1, sy1f) >> 8;
	a1 = MUL (a1, sy1f);
	t1 = MUL (t1, sy1f);

	// Middle lines.
	sy2i = _sy2 >> aa_BITS;
	if (++sy < sy2i) {
		r2 = g2 = b2 = t2 = a2 = 0;
		do {
			sx = sx1i;
			sline = (unsigned long*) (_src->line[sy]) + sx;

			scolor = *sline;
			if (scolor != _aa.mask_color) {
				ta = geta32(scolor);
				r2 += MUL (getr32 (scolor), sx1f*ta);
				g2 += MUL (getg32 (scolor), sx1f*ta);
				b2 += MUL (getb32 (scolor), sx1f*ta);
				a2 += MUL (ta, sx1f);
			}
			else t2 += sx1f;

			for (sline++, sx++; sx < sx2i; sline++, sx++) {
				scolor = *sline;
				if (scolor != _aa.mask_color) {
					ta = geta32(scolor);
					r2 += (getr32 (scolor) * ta) << aa_BITS;
					g2 += (getg32 (scolor) * ta) << aa_BITS;
					b2 += (getb32 (scolor) * ta) << aa_BITS;
					a2 += (ta) << aa_BITS;
				}
				else t2 += aa_SIZE;
			}

			if (sx2f != 0) {
				scolor = *sline;
				if (scolor != _aa.mask_color) {
					ta = geta32(scolor);
					r2 += MUL (getr32 (scolor), sx2f * ta);
					g2 += MUL (getg32 (scolor), sx2f * ta);
					b2 += MUL (getb32 (scolor), sx2f * ta);
					a2 += MUL (ta, sx2f);
				}
				else t2 += sx2f;
			}
		}
		while (++sy < sy2i);

		r1 += (r2 << aa_BITS) >> 8;
		g1 += (g2 << aa_BITS) >> 8;
		b1 += (b2 << aa_BITS) >> 8;
		a1 += a2 << aa_BITS;
		t1 += t2 << aa_BITS;
	}

	// Last line.
	sy2f = _sy2 & aa_MASK;
	if (sy2f != 0) {
		sx = sx1i;
		sline = (unsigned long*) (_src->line[sy]) + sx;

		scolor = *sline;
		if (scolor != _aa.mask_color) {
			ta = geta32(scolor);
			r2 = MUL (getr32 (scolor), sx1f * ta);
			g2 = MUL (getg32 (scolor), sx1f * ta);
			b2 = MUL (getb32 (scolor), sx1f * ta);
			a2 = MUL (ta, sx1f);
			t2 = 0;
		} else {
			r2 = g2 = b2 = a2 = 0;
			t2 = sx1f;
		}

		for (sline++, sx++; sx < sx2i; sline++, sx++) {
			scolor = *sline;
			if (scolor != _aa.mask_color) {
				ta = geta32(scolor);
				r2 += (getr32 (scolor) * ta) << aa_BITS;
				g2 += (getg32 (scolor) * ta) << aa_BITS;
				b2 += (getb32 (scolor) * ta) << aa_BITS;
				a2 += ta << aa_BITS;
			}
			else t2 += aa_SIZE;
		}

		if (sx2f != 0) {
			scolor = *sline;
			if (scolor != _aa.mask_color) {
				ta = geta32(scolor);
				r2 += MUL (getr32 (scolor), sx2f * ta);
				g2 += MUL (getg32 (scolor), sx2f * ta);
				b2 += MUL (getb32 (scolor), sx2f * ta);
				a2 += MUL (ta, sx2f);
			}
			else t2 += sx2f;
		}
		r1 += MUL (r2, sy2f) >> 8;
		g1 += MUL (g2, sy2f) >> 8;
		b1 += MUL (b2, sy2f) >> 8;
		a1 += MUL (a2, sy2f);
		t1 += MUL (t2, sy2f);
	}

	t1 = a1 + (t1 << 8);

	_aa.r = r1;
	_aa.g = g1;
	_aa.b = b1;
	_aa.trans = t1 + ((_aa.total - (_sx2 - _sx1) * (_sy2 - _sy1)) << 8);
	_aa_masked_add_bpp_independant_calculations();
	return;
}


//32 bit color with alpha channel, premultiplied.  should work
void
_aa_add_rgba8888 (BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2)
{
	unsigned long *sline;
	int sx, sx1i, sx1f, sx2i, sx2f;
	int sy, sy1i, sy1f, sy2i, sy2f;
	unsigned long r1, g1, b1, a1, t1;
	unsigned long r2, g2, b2, a2, t2;
	union
	{
		struct RGBA rgba;
		int i;
	} color;

	sy1i = _sy1 >> aa_BITS;
	sy = sy1i;

	// First line.
	sx1i = _sx1 >> aa_BITS;
	sx = sx1i;
	sline = (unsigned long*) (_src->line[sy]) + sx;

	sx1f = aa_SIZE - (_sx1 & aa_MASK);
	color.i = *sline;
	if (color.i != _aa.mask_color) {
		r1 = MUL (color.rgba.r, sx1f);
		g1 = MUL (color.rgba.g, sx1f);
		b1 = MUL (color.rgba.b, sx1f);
		a1 = MUL (color.rgba.a, sx1f);
		t1 = 0;
	} else {
		r1 = g1 = b1 = a1 = 0;
		t1 = sx1f;
	}

	sx2i = _sx2 >> aa_BITS;
	for (sline++, sx++; sx < sx2i; sline++, sx++) {
		color.i = *sline;
		if (color.i != _aa.mask_color) {
			r1 += color.rgba.r << aa_BITS;
			g1 += color.rgba.g << aa_BITS;
			b1 += color.rgba.b << aa_BITS;
			a1 += color.rgba.a << aa_BITS;
		}
		else t1 += aa_SIZE;
	}

	sx2f = _sx2 & aa_MASK;
	if (sx2f != 0) {
		color.i = *sline;
		if (color.i != _aa.mask_color) {
			r1 += MUL (color.rgba.r, sx2f);
			g1 += MUL (color.rgba.g, sx2f);
			b1 += MUL (color.rgba.b, sx2f);
			a1 += MUL (color.rgba.a, sx2f);
		}
		else t1 += sx2f;
	}

	sy1f = aa_SIZE - (_sy1 & aa_MASK);
	r1 = MUL (r1, sy1f);
	g1 = MUL (g1, sy1f);
	b1 = MUL (b1, sy1f);
	a1 = MUL (a1, sy1f);
	t1 = MUL (t1, sy1f);

	// Middle lines.
	sy2i = _sy2 >> aa_BITS;
	if (++sy < sy2i) {
		r2 = g2 = b2 = t2 = a2 = 0;
		do {
			sx = sx1i;
			sline = (unsigned long*) (_src->line[sy]) + sx;

			color.i = *sline;
			if (color.i != _aa.mask_color) {
				r2 += MUL (color.rgba.r, sx1f);
				g2 += MUL (color.rgba.g, sx1f);
				b2 += MUL (color.rgba.b, sx1f);
				a2 += MUL (color.rgba.a, sx1f);
			}
			else t2 += sx1f;

			for (sline++, sx++; sx < sx2i; sline++, sx++) {
				color.i = *sline;
				if (color.i != _aa.mask_color) {
					r2 += color.rgba.r << aa_BITS;
					g2 += color.rgba.g << aa_BITS;
					b2 += color.rgba.b << aa_BITS;
					a2 += color.rgba.a << aa_BITS;
				}
				else t2 += aa_SIZE;
			}

			if (sx2f != 0) {
				color.i = *sline;
				if (color.i != _aa.mask_color) {
					r2 += MUL (color.rgba.r, sx2f);
					g2 += MUL (color.rgba.g, sx2f);
					b2 += MUL (color.rgba.b, sx2f);
					a2 += MUL (color.rgba.a, sx2f);
				}
				else t2 += sx2f;
			}
		}
		while (++sy < sy2i);

		r1 += r2 << aa_BITS;
		g1 += g2 << aa_BITS;
		b1 += b2 << aa_BITS;
		a1 += a2 << aa_BITS;
		t1 += t2 << aa_BITS;
	}

	// Last line.
	sy2f = _sy2 & aa_MASK;
	if (sy2f != 0) {
		sx = sx1i;
		sline = (unsigned long*) (_src->line[sy]) + sx;

		color.i = *sline;
		if (color.i != _aa.mask_color) {
			r2 = MUL (color.rgba.r, sx1f);
			g2 = MUL (color.rgba.g, sx1f);
			b2 = MUL (color.rgba.b, sx1f);
			a2 = MUL (color.rgba.a, sx1f);
			t2 = 0;
		} else {
			r2 = g2 = b2 = a2 = 0;
			t2 = sx1f;
		}

		for (sline++, sx++; sx < sx2i; sline++, sx++) {
			color.i = *sline;
			if (color.i != _aa.mask_color) {
				r2 += color.rgba.r << aa_BITS;
				g2 += color.rgba.g << aa_BITS;
				b2 += color.rgba.b << aa_BITS;
				a2 += color.rgba.a << aa_BITS;
			}
			else t2 += aa_SIZE;
		}

		if (sx2f != 0) {
			color.i = *sline;
			if (color.i != _aa.mask_color) {
				r2 += MUL (color.rgba.r, sx2f);
				g2 += MUL (color.rgba.g, sx2f);
				b2 += MUL (color.rgba.b, sx2f);
				a2 += MUL (color.rgba.a, sx2f);
			}
			else t2 += sx2f;
		}
		r1 += MUL (r2, sy2f);
		g1 += MUL (g2, sy2f);
		b1 += MUL (b2, sy2f);
		a1 += MUL (a2, sy2f);
		t1 += MUL (t2, sy2f);
	}

	if (_rgb_r_shift_32 == 0) {
		_aa.r = r1;
		_aa.g = g1;
		_aa.b = b1;
		t1 = a1 + (t1 << 8);
	}
	else if (_rgb_r_shift_32 == 16) {
		_aa.r = b1;
		_aa.g = g1;
		_aa.b = r1;
		if (a1 != 0) {
			t1 = a1 + (t1 << 8);
		}
		else t1 = t1 << 8;
	} else {
		//uh.. what do we do now?
		_aa.r = rand() & 255;	 //graphics
		t1 = 0;
	}
	_aa.trans = t1 + ((_aa.total - (_sx2 - _sx1) * (_sy2 - _sy1)) << 8);
	_aa_masked_add_bpp_independant_calculations();
	return;
}
#endif
#ifdef ALLEGRO_COLOR16
//16 bit color with alpha channel, premultiplied.  should work
void
_aa_add_rgba4444 (BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2)
{
	unsigned short *sline;
	int sx, sx1i, sx1f, sx2i, sx2f;
	int sy, sy1i, sy1f, sy2i, sy2f;
	unsigned long r1, g1, b1, a1, t1;
	unsigned long r2, g2, b2, a2, t2;

	int scolor;

	sy1i = _sy1 >> aa_BITS;
	sy = sy1i;

	// First line.
	sx1i = _sx1 >> aa_BITS;
	sx = sx1i;
	sline = (unsigned short*) (_src->line[sy]) + sx;

	sx1f = aa_SIZE - (_sx1 & aa_MASK);
	scolor = *sline;
	if (scolor != _aa.mask_color) {
		r1 = MUL (getr16a(scolor), sx1f);
		g1 = MUL (getg16a(scolor), sx1f);
		b1 = MUL (getb16a(scolor), sx1f);
		a1 = MUL (geta16a(scolor), sx1f);
		t1 = 0;
	} else {
		r1 = g1 = b1 = a1 = 0;
		t1 = sx1f;
	}

	sx2i = _sx2 >> aa_BITS;
	for (sline++, sx++; sx < sx2i; sline++, sx++) {
		scolor = *sline;
		if (scolor != _aa.mask_color) {
			r1 += getr16a(scolor) << aa_BITS;
			g1 += getg16a(scolor) << aa_BITS;
			b1 += getb16a(scolor) << aa_BITS;
			a1 += geta16a(scolor) << aa_BITS;
		}
		else t1 += aa_SIZE;
	}

	sx2f = _sx2 & aa_MASK;
	if (sx2f != 0) {
		scolor = *sline;
		if (scolor != _aa.mask_color) {
			r1 += MUL (getr16a(scolor), sx2f);
			g1 += MUL (getg16a(scolor), sx2f);
			b1 += MUL (getb16a(scolor), sx2f);
			a1 += MUL (geta16a(scolor), sx2f);
		}
		else t1 += sx2f;
	}

	sy1f = aa_SIZE - (_sy1 & aa_MASK);
	r1 = MUL (r1, sy1f);
	g1 = MUL (g1, sy1f);
	b1 = MUL (b1, sy1f);
	a1 = MUL (a1, sy1f);
	t1 = MUL (t1, sy1f);

	// Middle lines.
	sy2i = _sy2 >> aa_BITS;
	if (++sy < sy2i) {
		r2 = g2 = b2 = a2 = t2 = 0;
		do {
			sx = sx1i;
			sline = (unsigned short*) (_src->line[sy]) + sx;

			scolor = *sline;
			if (scolor != _aa.mask_color) {
				r2 += MUL (getr16a(scolor), sx1f);
				g2 += MUL (getg16a(scolor), sx1f);
				b2 += MUL (getb16a(scolor), sx1f);
				a2 += MUL (geta16a(scolor), sx1f);
			}
			else t2 += sx1f;

			for (sline++, sx++; sx < sx2i; sline++, sx++) {
				scolor = *sline;
				if (scolor != _aa.mask_color) {
					r2 += getr16a(scolor) << aa_BITS;
					g2 += getg16a(scolor) << aa_BITS;
					b2 += getb16a(scolor) << aa_BITS;
					a2 += geta16a(scolor) << aa_BITS;
				}
				else t2 += aa_SIZE;
			}

			if (sx2f != 0) {
				scolor = *sline;
				if (scolor != _aa.mask_color) {
					r2 += MUL (getr16a(scolor), sx2f);
					g2 += MUL (getg16a(scolor), sx2f);
					b2 += MUL (getb16a(scolor), sx2f);
					a2 += MUL (geta16a(scolor), sx2f);
				}
				else t2 += sx2f;
			}
		}
		while (++sy < sy2i);

		r1 += r2 << aa_BITS;
		g1 += g2 << aa_BITS;
		b1 += b2 << aa_BITS;
		a1 += a2 << aa_BITS;
		t1 += t2 << aa_BITS;
	}

	// Last line.
	sy2f = _sy2 & aa_MASK;
	if (sy2f != 0) {
		sx = sx1i;
		sline = (unsigned short*) (_src->line[sy]) + sx;

		scolor = *sline;
		if (scolor != _aa.mask_color) {
			r2 = MUL (getr16a(scolor), sx1f);
			g2 = MUL (getg16a(scolor), sx1f);
			b2 = MUL (getb16a(scolor), sx1f);
			a2 = MUL (geta16a(scolor), sx1f);
			t2 = 0;
		} else {
			r2 = g2 = b2 = a2 = 0;
			t2 = sx1f;
		}

		for (sline++, sx++; sx < sx2i; sline++, sx++) {
			scolor = *sline;
			if (scolor != _aa.mask_color) {
				r2 += getr16a(scolor) << aa_BITS;
				g2 += getg16a(scolor) << aa_BITS;
				b2 += getb16a(scolor) << aa_BITS;
				a2 += geta16a(scolor) << aa_BITS;
			}
			else t2 += aa_SIZE;
		}

		if (sx2f != 0) {
			scolor = *sline;
			if (scolor != _aa.mask_color) {
				r2 += MUL (getr16a(scolor), sx2f);
				g2 += MUL (getg16a(scolor), sx2f);
				b2 += MUL (getb16a(scolor), sx2f);
				a2 += MUL (geta16a(scolor), sx2f);
			}
			else t2 += sx2f;
		}
		r1 += MUL (r2, sy2f);
		g1 += MUL (g2, sy2f);
		b1 += MUL (b2, sy2f);
		a1 += MUL (a2, sy2f);
		t1 += MUL (t2, sy2f);
	}

	_aa.r = r1 + (r1 >> 4);
	_aa.g = g1 + (g1 >> 4);
	_aa.b = b1 + (b1 >> 4);
	t1 = a1 + (a1 >> 4) + (t1 << 8);

	_aa.trans = t1 + ((_aa.total - (_sx2 - _sx1) * (_sy2 - _sy1)) << 8);
	_aa_masked_add_bpp_independant_calculations();
	return;
}
#endif

#define _READ_8(a)  (*(Uint8*)(a))
#define _READ_16(a) (*(Uint16*)(a))
#define _READ_32(a) (*(Uint32*)(a))

#if 0
#elif defined _READ24_AS_MISALIGNED_OVERSIZE
#   if defined ALLEGRO_LITTLE_ENDIAN
#       define _READ_24(a) ((*(Uint32*)(a)) & 0x00FFffFF)
#   elif defined ALLEGRO_BIG_ENDIAN
#       define _READ_24(a) ((*(Uint32*)(a)) >> 8)
#   else
#       error endianness not defined!
#   endif
#elif defined _READ24_AS_CHARS
#   if defined ALLEGRO_LITTLE_ENDIAN
#       define _READ_24(a) (((unsigned long)((Uint8*)(a))[0]) | ((unsigned long)((Uint8*)(a))[1] << 8) | ((unsigned long)((Uint8*)(a))[2] << 16))
#   elif defined ALLEGRO_BIG_ENDIAN
#       define _READ_24(a) (((unsigned long)((Uint8*)(a))[0] << 16) | ((unsigned long)((Uint8*)(a))[1] << 8) | ((unsigned long)((Uint8*)(a))[2]))
#   else
#       error endianness not defined!
#   endif
#elif defined _READ24_MISALIGNED_MIX
#   if defined ALLEGRO_LITTLE_ENDIAN
#       define _READ_24(a) (((unsigned long)((Uint16*)(a))[0]) | ((unsigned long)((Uint8*)(a))[2] << 16))
#   elif defined ALLEGRO_BIG_ENDIAN
#       define _READ_24(a) (((unsigned long)((Uint16*)(a))[0] << 8) | ((unsigned long)((Uint8*)(a))[2]))
#   else
#       error endianness not defined!
#   endif
#else
#   error _READ24 method not specified
#endif

#define DECLARE_GET_FUNC(name, bpp, bpp2) \
	void \
	_aa_get_##name (BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2)\
	{ \
		int sy, sx;\
		int scolor;\
		unsigned long _address;\
		sy = (_sy1 + _sy2) >> (1+aa_BITS);\
		sx = (_sx1 + _sx2) >> (1+aa_BITS);\
		_address = (sx * (32 / 8)) + (unsigned long)(_src->line[sy]);\
		scolor = _READ_##bpp(_src->line[sy]+(sx * (bpp/8)));\
		if (scolor == _aa.mask_color) _aa.transparent = -1;\
		else _aa.transparent = 0;\
		_aa.r = getr##bpp2(scolor);\
		_aa.g = getg##bpp2(scolor);\
		_aa.b = getb##bpp2(scolor);\
		_aa.trans = geta##bpp2(scolor);\
		return;\
	}

#define DECLARE_GET_FUNC2(name, bpp, bpp2) \
	void \
	_aa_get_##name (BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2)\
	{ \
		int sy, sx;\
		int scolor;\
		int a;\
		unsigned long _address;\
		sy = (_sy1 + _sy2) >> (1+aa_BITS);\
		sx = (_sx1 + _sx2) >> (1+aa_BITS);\
		_address = (sx * (32 / 8)) + (unsigned long)(_src->line[sy]);\
		scolor = _READ_##bpp(_src->line[sy]+(sx * (bpp/8)));\
		if (scolor == _aa.mask_color) {_aa.transparent = -1; return;} \
		else _aa.transparent = 0;\
		a = (_sx2-_sx1) * (_sy2-_sy1);\
		_aa.r = getr##bpp2(scolor) * a;\
		_aa.g = getg##bpp2(scolor) * a;\
		_aa.b = getb##bpp2(scolor) * a;\
		_aa.trans = geta##bpp2(scolor) * a;\
		_aa_masked_add_bpp_independant_calculations();\
		return;\
	}

#define DECLARE_ADD_FUNC(name, bpp, bpp2) \
	void \
	_aa_add_##name (BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2)\
	{ \
		unsigned long _address;\
		int sx, sx1i, sx1f, sx2i, sx2f;\
		int sy, sy1i, sy1f, sy2i, sy2f;\
		unsigned long r1, g1, b1, t1;\
		unsigned long r2, g2, b2, t2;\
		int scolor;\
		sy1i = _sy1 >> aa_BITS;\
		sy = sy1i;\
		sx1i = _sx1 >> aa_BITS;\
		sx = sx1i;\
		_address = (sx * (bpp / 8)) + (unsigned long)(_src->line[sy]);\
		sx1f = aa_SIZE - (_sx1 & aa_MASK);\
		scolor = _READ_##bpp(_address);\
		if (scolor != _aa.mask_color)\
		{ \
			r1 = MUL (getr##bpp2(scolor), sx1f);\
			g1 = MUL (getg##bpp2(scolor), sx1f);\
			b1 = MUL (getb##bpp2(scolor), sx1f);\
			t1 = 0;\
		}\
		else\
		{ \
			r1 = g1 = b1 = 0;\
			t1 = sx1f;\
		}\
		sx2i = _sx2 >> aa_BITS;\
		for ((_address += (bpp / 8)), sx++; sx < sx2i; (_address += (bpp / 8)), sx++)\
		{ \
			scolor = _READ_##bpp(_address);\
			if (scolor != _aa.mask_color)\
			{ \
				r1 += getr##bpp2(scolor) << aa_BITS;\
				g1 += getg##bpp2(scolor) << aa_BITS;\
				b1 += getb##bpp2(scolor) << aa_BITS;\
			}\
			else t1 += aa_SIZE;\
		}\
		sx2f = _sx2 & aa_MASK;\
		if (sx2f != 0)\
		{ \
			scolor = _READ_##bpp(_address);\
			if (scolor != _aa.mask_color)\
			{ \
				r1 += MUL (getr##bpp2(scolor), sx2f);\
				g1 += MUL (getg##bpp2(scolor), sx2f);\
				b1 += MUL (getb##bpp2(scolor), sx2f);\
			}\
			else t1 += sx2f;\
		}\
		sy1f = aa_SIZE - (_sy1 & aa_MASK);\
		r1 = MUL (r1, sy1f);\
		g1 = MUL (g1, sy1f);\
		b1 = MUL (b1, sy1f);\
		t1 = MUL (t1, sy1f);\
		sy2i = _sy2 >> aa_BITS;\
		if (++sy < sy2i)\
		{ \
			r2 = g2 = b2 = t2 = 0;\
			do\
			{ \
				sx = sx1i;\
				_address = (sx * (bpp / 8)) + (unsigned long)(_src->line[sy]);\
				scolor = _READ_##bpp(_address);\
				if (scolor != _aa.mask_color)\
				{ \
					r2 += MUL (getr##bpp2(scolor), sx1f);\
					g2 += MUL (getg##bpp2(scolor), sx1f);\
					b2 += MUL (getb##bpp2(scolor), sx1f);\
				}\
				else t2 += sx1f;\
				for ((_address += (bpp / 8)), sx++; sx < sx2i; (_address += (bpp / 8)), sx++)\
				{ \
					scolor = _READ_##bpp(_address);\
					if (scolor != _aa.mask_color)\
					{ \
						r2 += getr##bpp2(scolor) << aa_BITS;\
						g2 += getg##bpp2(scolor) << aa_BITS;\
						b2 += getb##bpp2(scolor) << aa_BITS;\
					}\
					else t2 += aa_SIZE;\
				}\
				if (sx2f != 0)\
				{ \
					scolor = _READ_##bpp(_address);\
					if (scolor != _aa.mask_color)\
					{ \
						r2 += MUL (getr##bpp2(scolor), sx2f);\
						g2 += MUL (getg##bpp2(scolor), sx2f);\
						b2 += MUL (getb##bpp2(scolor), sx2f);\
					}\
					else t2 += sx2f;\
				}\
			}\
			while (++sy < sy2i);\
			r1 += r2 << aa_BITS;\
			g1 += g2 << aa_BITS;\
			b1 += b2 << aa_BITS;\
			t1 += t2 << aa_BITS;\
		}\
		sy2f = _sy2 & aa_MASK;\
		if (sy2f != 0)\
		{ \
			sx = sx1i;\
			_address = (sx * (bpp / 8)) + (unsigned long)(_src->line[sy]);\
			scolor = _READ_##bpp(_address);\
			if (scolor != _aa.mask_color)\
			{ \
				r2 = MUL (getr##bpp2(scolor), sx1f);\
				g2 = MUL (getg##bpp2(scolor), sx1f);\
				b2 = MUL (getb##bpp2(scolor), sx1f);\
				t2 = 0;\
			}\
			else\
			{ \
				r2 = g2 = b2 = 0;\
				t2 = sx1f;\
			}\
			for ((_address += (bpp / 8)), sx++; sx < sx2i; (_address += (bpp / 8)), sx++)\
			{ \
				scolor = _READ_##bpp(_address);\
				if (scolor != _aa.mask_color)\
				{ \
					r2 += getr##bpp2(scolor) << aa_BITS;\
					g2 += getg##bpp2(scolor) << aa_BITS;\
					b2 += getb##bpp2(scolor) << aa_BITS;\
				}\
				else t2 += aa_SIZE;\
			}\
			if (sx2f != 0)\
			{ \
				scolor = _READ_##bpp(_address);\
				if (scolor != _aa.mask_color)\
				{ \
					r2 += MUL (getr##bpp2(scolor), sx2f);\
					g2 += MUL (getg##bpp2(scolor), sx2f);\
					b2 += MUL (getb##bpp2(scolor), sx2f);\
				}\
				else t2 += sx2f;\
			}\
			r1 += MUL (r2, sy2f);\
			g1 += MUL (g2, sy2f);\
			b1 += MUL (b2, sy2f);\
			t1 += MUL (t2, sy2f);\
		}\
		_aa.r = r1;\
		_aa.g = g1;\
		_aa.b = b1;\
		t1 = (t1 << 8);\
		_aa.trans = t1 + ((_aa.total - (_sx2 - _sx1) * (_sy2 - _sy1)) << 8);\
		_aa_masked_add_bpp_independant_calculations();\
		return;\
	}

																	//	_aa.trans = _aa.total - ((_sx2 - _sx1) * (_sy2 - _sy1) - t1);

																												//#define DECLARE_ADD_FUNC(name, bpp, bpp2, rshift, gshift, bshift)
																												DECLARE_ADD_FUNC(rgb8,   8,  8)
																												DECLARE_ADD_FUNC(rgb15, 16, 15)
																												DECLARE_ADD_FUNC(rgb16, 16, 16)
																												DECLARE_ADD_FUNC(rgb24, 24, 24)
																												DECLARE_ADD_FUNC(rgb32, 32, 32)

																												ADD_TYPE *add_array[5] =
																												{
																													_aa_add_rgb8,
																													_aa_add_rgb15,
																													_aa_add_rgb16,
																													_aa_add_rgb24,
																													_aa_add_rgb32
																												};

DECLARE_GET_FUNC(rgb8,   8,  8)
DECLARE_GET_FUNC(rgb15, 16, 15)
DECLARE_GET_FUNC(rgb16, 16, 16)
DECLARE_GET_FUNC(rgb24, 24, 24)
DECLARE_GET_FUNC(rgb32, 32, 32)

DECLARE_GET_FUNC(rgb16a, 16, 16a)
DECLARE_GET_FUNC(rgb32a, 32, 32a)
DECLARE_GET_FUNC2(rgb16a_t, 16, 16a)
DECLARE_GET_FUNC2(rgb32a_t, 32, 32a)

DECLARE_GET_FUNC2(rgb8_t,   8,  8)
DECLARE_GET_FUNC2(rgb15_t, 16, 15)
DECLARE_GET_FUNC2(rgb16_t, 16, 16)
DECLARE_GET_FUNC2(rgb24_t, 24, 24)
DECLARE_GET_FUNC2(rgb32_t, 32, 32)

ADD_TYPE *get_array[5] =
{
	_aa_get_rgb8,
	_aa_get_rgb15,
	_aa_get_rgb16,
	_aa_get_rgb24,
	_aa_get_rgb32
};

ADD_TYPE *get_array2[5] =
{
	_aa_get_rgb8_t,
	_aa_get_rgb15_t,
	_aa_get_rgb16_t,
	_aa_get_rgb24_t,
	_aa_get_rgb32_t
};

ADD_TYPE *get_aa_add_function(BITMAP *source, int mode)
{
	int bpp, i;

	if (mode & AA_MASKED) _aa.mask_color = bitmap_mask_color(source);
	else _aa.mask_color =  -1;
	if (!is_memory_bitmap(source)) return NULL;

	bpp = bitmap_color_depth(source);
	if (bpp == 0) return NULL;
	else if (bpp == 24) _aa_prepare_for_24bpp();

	for (i = 0; i < num_bpps; i += 1) if (BPP_ARRAY[i] == bpp) break;
	if (i == num_bpps) return NULL;

	if (AA_ALPHA & mode) {
		if (bpp == 32) {
			if (_aa.mask_color == -1) _aa.mask_color = 0xff000000;
			else _aa.mask_color = MASK_COLOR_32a;
			if (AA_NO_AA & mode) return &_aa_get_rgb32a_t;
			return &_aa_add_rgba8888;
		}
		if (bpp == 16) {
			if (_aa.mask_color == -1) _aa.mask_color = 0xf000;
			else _aa.mask_color = MASK_COLOR_16a;
			if (AA_NO_AA & mode) return &_aa_get_rgb16a_t;
			return &_aa_add_rgba4444;
		}
	}
	if (AA_NO_AA & mode) {
		if ((aa_get_trans() != 0) || (mode & AA_NO_ALIGN))
			return get_array2[i];
		else return get_array[i];
	}
	return add_array[i];
}


void invert_alpha ( BITMAP *bob, int masked )
{
	int x, y, mask;
	//32 bit RGBA 8888
	if (bitmap_color_depth(bob) == 32) {
		if (masked) mask = bitmap_mask_color(bob); else mask = -1;
		for (y = 0; y < bob->h; y += 1) {
			for (x = 0; x < bob->w; x += 1) {
				int c = ((int*)bob->line[y])[x];
				if (c == mask) continue;
				((int*)bob->line[y])[x] = c ^ 0xff000000;
			}
		}
	}
	//16 bit RGBA 4444
	if (bitmap_color_depth(bob) == 16) {
		if (masked) mask = 0x0F0F; else mask = -1;
		for (y = 0; y < bob->h; y += 1) {
			for (x = 0; x < bob->w; x += 1) {
				int c = ((short*)bob->line[y])[x];
				if (c == mask) continue;
				((short*)bob->line[y])[x] = c ^ 0xf000;
			}
		}
	}
	return;
}


void premultiply_alpha ( BITMAP *bob, int masked )
{
	int x, y, a, mask;
	union
	{
		struct RGBA rgba;
		int i;
	} c;
	if (bitmap_color_depth(bob) != 32) return;
	if (masked) mask = bitmap_mask_color(bob); else mask = -1;
	//32 bit RGBA 8888
	for (y = 0; y < bob->h; y += 1) {
		for (x = 0; x < bob->w; x += 1) {
			c.i = ((int*)bob->line[y])[x];
			if (c.i == mask) continue;
			a = (c.rgba.a ^ 255) + 1;
			c.rgba.r = (c.rgba.r * a) >> 8;
			c.rgba.g = (c.rgba.g * a) >> 8;
			c.rgba.b = (c.rgba.b * a) >> 8;
			((int*)bob->line[y])[x] = c.i;
		}
	}
	return;
}


void un_premultiply_alpha ( BITMAP *bob, int masked )
{
	int x, y, a, mask;
	union
	{
		struct RGBA rgba;
		int i;
	} c;
	if (bitmap_color_depth(bob) != 32) return;
	if (masked) mask = bitmap_mask_color(bob); else mask = -1;
	//32 bit RGBA 8888
	for (y = 0; y < bob->h; y += 1) {
		for (x = 0; x < bob->w; x += 1) {
			c.i = ((int*)bob->line[y])[x];
			if (c.i == mask) continue;
			a = (c.rgba.a ^ 255) + 1;
			c.rgba.r = (c.rgba.r << 8) / a;
			c.rgba.g = (c.rgba.g << 8) / a;
			c.rgba.b = (c.rgba.b << 8) / a;
			((int*)bob->line[y])[x] = c.i;
		}
	}
	return;
}


void convert_alpha ( BITMAP *bob, int masked )
{
	invert_alpha ( bob, masked );
	//	premultiply_alpha ( bob, masked );
}


void un_convert_alpha ( BITMAP *bob, int masked )
{
	//	un_premultiply_alpha ( bob, masked );
	invert_alpha ( bob, masked );
}


void rgba4444_as_rgb16 (BITMAP *bob)
{
	int x, y, i;
	if (bitmap_color_depth(bob) != 16) return;
	for (y = 0; y < bob->h; y += 1) {
		for (x = 0; x < bob->w; x += 1) {
			i = ((unsigned short*)bob->line[y])[x];
			if (_rgb_r_shift_16 == 11) {
				i = (i >> 11) | (i << 11) | (i & 0x07e0);
			}
			((unsigned short*)bob->line[y])[x] = i;
		}
	}
	return;
}


void make_alpha ( BITMAP *bob )
{
	int x, y, a;
	union
	{
		struct RGBA rgba;
		int i;
	} c;
	if (bitmap_color_depth(bob) != 32) return;
	//32 bit RGBA 8888
	for (y = 0; y < bob->h; y += 1) {
		for (x = 0; x < bob->w; x += 1) {
			c.i = ((int*)bob->line[y])[x];
			a = c.rgba.r;
			if (c.rgba.g > a) a = c.rgba.g;
			if (c.rgba.b > a) a = c.rgba.b;
			a ^= 255;
			c.rgba.a = a;
			((int*)bob->line[y])[x] = c.i;
		}
	}
	return;
}


void make_alpha_scale ( BITMAP *bob, int nmin, int nmax )
{
	int omin = 255, omax = 0;
	int x, y, a;
	union
	{
		struct RGBA rgba;
		int i;
	} c;
	if (bitmap_color_depth(bob) != 32) return;
	//32 bit RGBA 8888
	for (y = 0; y < bob->h; y += 1) {
		for (x = 0; x < bob->w; x += 1) {
			c.i = ((int*)bob->line[y])[x];
			if (c.i != MASK_COLOR_32) {
				a = c.rgba.r;
				if (c.rgba.g > a) a = c.rgba.g;
				if (c.rgba.b > a) a = c.rgba.b;
				a ^= 255;
				if (a < omin) omin = a;
				if (a > omax) omax = a;
				c.rgba.a = a;
				((int*)bob->line[y])[x] = c.i;
			}
		}
	}
	for (y = 0; y < bob->h; y += 1) {
		for (x = 0; x < bob->w; x += 1) {
			c.i = ((int*)bob->line[y])[x];
			if (c.i != MASK_COLOR_32) {
				a = c.rgba.a;
				a = (a - omin) * (nmax-nmin) / (omax-omin) + nmin;
				if (a > 255) a = 255;
				if (a < 0) a = 0;
				c.rgba.a = a;
				((int*)bob->line[y])[x] = c.i;
			}
		}
	}
	return;
}


unsigned long _blender_premultiplied_alpha24 (unsigned long x, unsigned long y, unsigned long n)
{
	unsigned long res, g;

	n = geta32(x) + 1;

	res = (x & 0xFF00FF) * n / 256;
	g = (x & 0xFF00) * n / 256;

	res = (res & 0xFF00FF) | g;

	return res + y;
}


unsigned long _blender_premultiplied_alpha24_bgr(unsigned long x, unsigned long y, unsigned long n)
{
	unsigned long res, g;

	n = x >> 24;

	n++;

	x = ((x>>16)&0xFF) | (x&0xFF00) | ((x<<16)&0xFF0000);

	res = (x & 0xFF00FF) * n / 256;
	g = (x & 0xFF00) * n / 256;

	res = (res & 0xFF00FF) | g;

	return res + y;
}


/*
 * aautil.c ends here
 */
