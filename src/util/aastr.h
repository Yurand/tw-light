/*
 * aastr.h --- anti-aliased stretching and rotation for Allegro
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
 *
 * Improved & modified by orz for Star Control: TimeWarp
 */

#ifndef __bma_aastr_h
#define __bma_aastr_h

#include <allegro.h>

/*
 * Sub-pixel precision
 * 8 or 4 are probably the fastest for i386+.
 * Recompile the package after changing aa_BITS.
 */
#define aa_BITS     8

#ifdef __cplusplus
extern "C"
{
	#endif

	//trans is on a scale from 0 (opaque) to 255 (transparent)
	//transparency is not yet supported by the rotation routines
	//default is 0
	void aa_set_trans ( int mode );
	int  aa_get_trans ( );

	//default is black
	void aa_set_background ( RGB color );
	RGB  aa_get_background ( );

	//use the mode options listed below (they can be ORed togethor)
	//default is 0 (all options off)
	void aa_set_mode ( int mode );
	int  aa_get_mode ( );

	//mode options:
	enum {
		AA_ALPHA        = 0x0001,//treat source as an alpha bitmap (16bpp / 32bpp only)
		AA_BLEND        = 0x0002,//blend with destination image
		AA_MASKED       = 0x0004,//treat mask-color pixels in the source as transparent
		AA_VFLIP        = 0x0008,//flip source vertically
		AA_HFLIP        = 0x0010,//flip source horizontally

		AA_NO_ALIGN     = 0x0020,//use non-integer coordinates for source & destination rectangles
		//otherwise, coordinates will be rounded to integer positions
		AA_RAW_ALPHA    = 0x0040,//treat destination as an alpha bitmap (16bpp / 32bpp only), overwrite alpha channel
		AA_DITHER       = 0x0080,//dither (spatially)
		AA_MASKED_DEST  = 0x0100,//do not write any transparent (mask color) pixels in the ouput
		AA_NO_FILTER    = 0x2000,//disables bi-linear filtering

		//	AA_BLENDER_A    = 0x0400, //use Allegro blenders w/ alpha channel
		//	AA_BLENDER_N    = 0x0800, //use Allegro blenders w/ constant

		AA_NO_AA        = 0x8000,//disables anti-aliasing

		AA_BLAH = 0
	};

	/*notes on options:

	You bitwise-OR some option flags togethor, and pass them to the AA engines.

	Masking: (AA_MASKED)
		This option is used for sprites that have a specific
		transparent color.

	Non-integer destinations: (AA_NO_ALIGN)
		If this option is enabled, the destination rectangle will
		NOT be lowered to integer precision.  I'll explain this more
	later when I have time.  Yeah right.  It can make
	slow-moving sprites moving at certain angles look much
	better.  This option is not yet implemented for rotations,
	only for stretching.  This option makes drawing small
	destination rectangles substancially slower, but has no
	significant impact upon larger destination rectangles.

	No bi-linear filtering: (AA_NO_FILTER)
	If this option is enabled, then sprites drawn with large
	scaling factors will look blocky instead of blurry.

	Dithering: (AA_DITHER)
	If you are drawing a 32 bpp image onto an 8 bpp surface,
	quality will generally suffer because of the lower precision
	in color.  Normally, the closest color will be chosen for
	each pixel in the destination.  However, it is possible to
	achieve better image quality, particularly at higher
	resolutions, through a process called dithering that makes
	regions have more precise colors even if individual pixels
	have less precise colors.  If dithering is enabled, this
	library will attempt to use dithering when drawing onto a
	destination bitmap of 8, 15, or 16 bpp.  The dithering
	algorithm used is somewhat primitive.  Image quality
	improvements are usually only substancial at 8 bpp.

	No mask-color in output: (AA_MASKED_DEST)
	This option makes sure that the transparent color is not
	drawn on the destination.  It is a good idea if the
	destination is a sprite.  If the destination is not a sprite,
	this just slows things down and reduce image quality, though
	not significantly.

	Blending & Alpha Blending: (AA_ALPHA, AA_BLEND, AA_RAW_ALPHA)

	AA_ALPHA, AA_BLEND, and AA_RAW_ALPHA are related.
	AA_BLEND and AA_RAW_ALPHA are incompatible.

	If AA_ALPHA is set, it will read the source bitmap as an RGBA
	bitmap, (either RGBA8888 or RGBA4444).  If the source is
	neither 32 bpp or 16 bpp, then the AA_OPTION_ALPHA flag will
	be ignored.
	WARNING!: AASTR2 operates on a different internal understanding
	of alpha bitmaps.  You need to use convert_alpha() on your
	bitmap before you can expect its alpha channel stuff to work
	with AASTR2
	RGBA4444 is implemented, but not yet tested.

	AA_BLEND and AA_RAW_ALPHA effect how the destination
	bitmap is treated.  They are mutually exclusive - if both are
	specified at once, the result is undefined.

	If AA_BLEND is specified, partially transparent
	portions of the bitmap will be alpha-blended with the
	background.  This is the slowest blending mode.

	If AA_RAW_ALPHA is in effect, the destination will be treated
	as an RGBA bitmap (once again, either RGBA8888 or RGBA4444 or
	the flag ignored).  The output to it will set the alpha
	channel appropriately.

	If neither AA_OPTION_BLEND nor AA_OPTION_RAW_ALPHA is
	specified, then it will use the default mode, in which
	partially transparent pixels are blended with the background
	color set in the last aa_set_background call (or black, if no
	background color has been set).  This is equivalent to
	alpha-blending if the destination is a solid color equal
	to the currently set background color.  The default mode is
	the fastest mode.  The default mode is fastest if the
	background color is black.

	*/

	//Alpha conversion:
	/*
		AASTR2 uses a different alpha-channel format than Allegro.
		Therefore, convert_bitmap() and un_convert_bitmap() are
		provided.  convert_bitmap(), given a 32 bpp Allegro bitmap
		with an allegro-style alpha channel, will convert the bitmap
		to one with an AASTR2-style alpha channel.
		un_convert_alpha() will do the reverse.  However, calling
		convert_alpha() and then unconvert_alpha() will produce a
		small loss in precision of the colors.

		For completeness, the functions premultiply_alpha(),
	un_premultiply_alpha(), and invert_alpha() are also exposed,
	even though you shouldn't have to use them.

	Technical note: AASTR2 uses pre-multiplied alpha with
	0 == opaque... 255 = transparent.  Allegro uses
	non-premultiplied alpha channels with 0 = transparent...
	255 = opaque.
	*/

								 //32 bit only
	void convert_alpha         ( BITMAP *bob, int masked );
								 //32 bit only
	void un_convert_alpha      ( BITMAP *bob, int masked );
								 //32 bit only
	void premultiply_alpha    ( BITMAP *bob, int masked );
								 //32 bit only
	void un_premultiply_alpha ( BITMAP *bob, int masked );
								 //32 bit only
	void invert_alpha          ( BITMAP *bob, int masked );
								 //32 bit only
	void make_alpha ( BITMAP *bob ) ;
								 //16 bit only
	void rgba4444_as_rgb16 (BITMAP *bob) ;
	unsigned long _blender_premultiplied_alpha24 (unsigned long x, unsigned long y, unsigned long n) ;
	unsigned long _blender_premultiplied_alpha24_bgr(unsigned long x, unsigned long y, unsigned long n) ;

	// Stretching

	void aa_stretch_blit (BITMAP* src, BITMAP* dst,
		double sx, double sy, double sw, double sh,
		double dx, double dy, double dw, double dh);
	void aa_stretch_sprite (BITMAP* dst, BITMAP* src,
		double dx, double dy, double dw, double dh);

	// Rotation.
	void aa_rotate_scaled_bitmap (BITMAP* src, BITMAP* dst,
		int x, int y, fixed angle,
		fixed scalex, fixed scaley);
	void aa_rotate_scaled_sprite (BITMAP* dst, BITMAP* src,
		int x, int y, fixed angle,
		fixed scalex, fixed scaley);
	void aa_rotate_bitmap (BITMAP* src, BITMAP* dst,
		int x, int y, fixed angle);
	void aa_rotate_sprite (BITMAP* dst, BITMAP* src,
		int x, int y, fixed angle);

	// Engines

	void _aa_stretch_blit (BITMAP* src, BITMAP* dest,
		int sx, int sy, int sw, int sh,
		int dx, int dy, int dw, int dh,
		int mode);
	//does NOT perform checking/clipping on the source rectangle
	//parameters are long point (32-aa_BITS.aa_BITS) numbers

	void _aa_rotate_bitmap (BITMAP *_src, BITMAP *_dst,
		int _x, int _y, fixed _angle,
		fixed _scalex, fixed _scaley,
		int _mode);
	//destination is in integer format
	//rotation does not yet support the following new features:
	// (aa_set_trans, aa_get_trans) transparency
	// (AA_NO_FILTER) disabling of bi-linear filtering
	// (AA_NO_ALIGN) non-integer source/destination rectangles
	// (AA_VFLIP and/or AA_HFLIP) inverting axis

	#ifdef __cplusplus
}
#endif
#endif							 /* !__bma_aastr_h */

/*
 * aastr.h ends here
 */
