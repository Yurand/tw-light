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
#include <float.h>
#include <allegro.h>
#include <stdarg.h>

#include "melee.h"
REGISTER_FILE
#include "scp.h"
#include "frame.h"

#include "mview.h"
#include "mgame.h"
#include "mcbodies.h"
#include "mshppan.h"
#include "mcontrol.h"
#include "mship.h"

#include "other/twconfig.h"

#include "util/aastr.h"

int FULL_REDRAW = 0;
int camera_hides_cloakers = 0;

// units
View  *space_view;

Vector2 space_view_size;		 // pixels (should be an integer value)

double space_zoom;				 // 1
int    space_mip_i;				 // ?
double space_mip;				 // ?

Vector2 space_size;				 ///< game-pixels

Vector2 space_corner = 0;		 ///< game-pixels : uppper left corner
Vector2 space_center_nowrap = 0; ///< game-pixels : center, non-wrapped
Vector2 space_vel = 0;			 ///< game-pixels / millisecond : center
Vector2 space_center = 0;		 ///< game-pixels : center

message_type message;

View * ViewType::create( View * old )
{
	STACKTRACE;
	View * r = _create();
	r->preinit();
	r->type = this;
	r->init ( old );
	return r;
}


static View *_default_view = NULL;

int num_views = 0;
char **view_name = NULL;
ViewType *viewtypelist = NULL;

void set_view( View * new_default )
{
	STACKTRACE;
	if (!new_default) {tw_error( "new default view is NULL");}
	if (!new_default->type) {tw_error("new default view has no type info");}
	if (_default_view) delete _default_view;
	_default_view = new_default;
	return;
}


View * get_view(const char *name, View *old)
{
	int i;
	if ((name == NULL) && _default_view) {
		name = _default_view->type->name;
	}
	if (!old)
		old = _default_view;

	i = get_view_num ( name );
	if ( i < 0 ) return NULL;
	View *v = viewtypelist[i].create(old);
	return v;
}


int get_view_num( const char *name )
{
	STACKTRACE;
	int i;
	if (!name) return -1;
	for (i = 0; i < num_views; i += 1) {
		if (strncmp(viewtypelist[i].name, name, 64) == 0) {
			return i;
		}
	}
	return -1;
}


void View::preinit()
{
	STACKTRACE;
	frame = NULL;
	window = NULL;
	type = NULL;
}


void View::refresh()
{
	STACKTRACE;
	if (frame)
		frame->full_redraw = true;
	return;
}


void View::prepare( Frame *frame, int time )
{
	STACKTRACE;

	Vector2 oc = camera.pos;
	camera.pos += camera.vel * time;

	frame->prepare();

	::space_view = this;

	view_size.x = window->w;
	view_size.y = window->h;

	::space_view_size = view_size;
	double tz = magnitude(view_size) / 1.41421356237309504880168872;
	::space_zoom = tz / camera.z;
	::space_mip = -log(space_zoom) / log(2.0);
	::space_mip_i = iround_down(::space_mip);
	::space_size  = view_size * space_zoom;

	::space_corner = normalize2(camera.pos - space_size /2, map_size);
	::space_center = normalize2(camera.pos, map_size);
	::space_vel = camera.vel;
	::space_center_nowrap = camera.pos;

	camera.pos = oc;

	return;
}


void View::animate(Game *game)
{
	STACKTRACE;
	if (FULL_REDRAW) frame->full_redraw = true;
	frame->erase();
	prepare(frame, 0);

	if (frame->surface) {
		if (frame->surface)
			game->animate(frame);
		message.animate(frame);
	}

	scare_mouse();
	frame->draw();
	unscare_mouse();
	return;
}


void View::config()
{
	STACKTRACE;
	return;
}


bool View::screen2game(Vector2 *_pos)
{
	STACKTRACE;
	Vector2 pos = *_pos;
	Vector2 opos = pos;

	pos.x -= window->x;
	pos.y -= window->y;

	pos /= space_zoom;

	pos += camera.pos - space_size / 2;
	pos = normalize (pos, map_size);
	*_pos = pos;
	if ((opos.x < window->x) ||
		(opos.x >= window->x + window->w) ||
		(opos.y < window->y) ||
		(opos.y >= window->y + window->h))
		return false;
	return true;
}


double View::in_view(Vector2 pos, Vector2 size)
{
	STACKTRACE;
	pos = corner(pos, size);
	size = size * space_zoom;

	double a = size.x, b = size.y;
	double c;
	c = pos.x + size.x - space_view_size.x;
	if (c > 0) a -= c;
	c = pos.x;
	if (c < 0) a += c;
	if (a < 0) return 0;
	c = pos.y + size.y - space_view_size.y;
	if (c > 0) b -= c;
	c = pos.y;
	if (c < 0) b += c;
	if (b < 0) return 0;
	return a * b / (size.x * size.y);
}


int View::focus(CameraPosition *pos, SpaceLocation *la, SpaceLocation *lb)
{
	STACKTRACE;
	if (!la && !lb) return 0;
	if (!la) la = lb;
	if (!lb) lb = la;
	Vector2 p, p2;
	p = la->normal_pos();
	p2 = lb->normal_pos();
	double c, d;

	Vector2 a = Vector2(
		min_delta(p.x, p2.x, map_size.x),
		min_delta(p.y, p2.y, map_size.y)
		);

	p = normalize(p-a/2, map_size);

	c = fabs(a.x) + 1;
	d = fabs(a.y) + 1;
	//if (c < d) c = d;
	c = sqrt(c*c + d*d);

	pos->pos = p;
	if (lb != la) {
		pos->z = c;
		return 2;
	}
	else return 1;
}


/*void View::see_also(SpaceLocation *o) {
	STACKTRACE;
  STACKTRACE;
  if (!o) return;
  double x2, y2;
  x2 = b->normal_x();
  y2 = b->normal_y();
  double a, b, c, d;
  a = min_delta(x, x2, X_MAX);
  b = min_delta(y, y2, Y_MAX);
  c = view_x / (fabs(a) + 1);
  d = view_y / (fabs(b) + 1);
  if (c > d) c = d;
  x = normalize(x-a/2, X_MAX);
  y = normalize(y-b/2, Y_MAX);
  z = c;
  return;
  }*/
//make these static...

void View::track(const CameraPosition &target, CameraPosition *origin)
{
	STACKTRACE;
	if (!origin) origin = &this->camera;
	Vector2 d;
	d = target.pos - origin->pos;

	d = normalize2(d + map_size/2, map_size) - map_size/2;

	origin->pos += d;
	// debug GEO.
	// origin isn't normalized, this can grow very big. When a normalization
	// is used, this normalization must then handle a very large value; it
	// then encounters the limit 999. So, add a normalization ?
	normalize(origin->pos, map_size);

	origin->vel = d / frame_time;
	origin->z = target.z;
}


void View::track(const CameraPosition &target, double smooth_time, CameraPosition *origin)
{
	STACKTRACE;
	if (!origin) origin = &this->camera;
	Vector2 d;
	d.x = -min_delta( origin->pos.x, target.pos.x, map_size.x);
	d.y = -min_delta( origin->pos.y, target.pos.y, map_size.y);
	Vector2 dd;
	dd = d - origin->vel * frame_time;

	origin->z = target.z;
	//if (origin->z < 100) origin->z = 100;
	//if (origin->z > 10000) origin->z = 10000;

	/*	double r1 = (dx * dx + dy * dy) / (frame_time * frame_time);
	  double r2 = (ddx * ddx + ddy * ddy) / (frame_time * frame_time);

	  if ( 1 ) {
	  double d = (log(origin->z) / log(10) -2) /2;
	  d = 0.5 - 0.4 * d;
	  d = 0.5;
	  dx = origin->vx * d + dx * (1.0 - d);
	  dy = origin->vy * d + dy * (1.0 - d);
	  dx = ddx * (1.0 - d);
	  dx += origin->vx * (1.0 - d);
	  dy -= ddy * (1.0 - d);
	  dy += origin->vy * d;*/
	/*	}*/

	origin->vel = d / frame_time;
	origin->pos += d;

	// debug GEO.
	// origin isn't normalized, this can grow very big. When a normalization
	// is used, this normalization must then handle a very large value; it
	// then encounters the limit 999. So, add a normalization ?
	normalize(origin->pos, map_size);

	return;
}


void View::init(View *old)
{
	STACKTRACE;
	if (window || frame) {
		tw_error("View::init - hmm...");
	}
	if (old) {
		camera = old->camera;
		key_zoom_in = old->key_zoom_in;
		key_zoom_out = old->key_zoom_out;
		key_alter1 = old->key_alter1;
		key_alter2 = old->key_alter2;
		frame = new Frame(1024);
		window = frame->window;
	} else {
		frame = new Frame(1024);
		window = frame->window;
		camera.pos = Vector2(0,0);
		camera.z = 960;
		camera.vel = Vector2(0,0);

		tw_set_config_file ( "client.ini" );
		key_zoom_in  = name_to_key(get_config_string("View", "Key_zoomin",  "EQUALS"));
		key_zoom_out = name_to_key(get_config_string("View", "Key_zoomout", "MINUS"));
		key_alter1   = name_to_key(get_config_string("View", "Key_alter1",  "0"));
		key_alter2   = name_to_key(get_config_string("View", "Key_alter2",  "BACKSLASH"));
	}
	window->add_callback(this);
	//if (window->surface) ;
	return;
}


void View::replace( View * v )
{
	STACKTRACE;
	if (frame) {
		window->remove_callback(this);
		delete frame;
		frame = NULL;
		window = NULL;
	}
	frame = v->frame;
	window = frame->window;
	view_size = v->view_size;

	window->remove_callback(v);
	window->add_callback(this);

	v->frame = NULL;
	v->window = NULL;
	delete v;

	return;
}


View::~View()
{
	if (frame) {
		window->remove_callback(this);
		delete frame;
	}
}


void View::calculate(Game *game)
{
	STACKTRACE;
}


void message_type::out(char *string, int dur, int c)
{
	STACKTRACE;
	ASSERT (c < 256);
	if (num_messages == max_messages - 1) {
		messages[0].end_time = -1;
		clean();
	}
	if (num_messages >= max_messages - 1) throw "bad dog!";
	messages[num_messages].string = strdup(string);
	if (game) messages[num_messages].end_time = game->game_time + dur;
	else messages[num_messages].end_time = 0 + dur;
	messages[num_messages].color = palette_color[c];
	num_messages += 1;

	clean();
	return;
}


void message_type::print(int dur, int c, const char *format, ...)
{
	STACKTRACE;
	char buf[1024];
	va_list those_dots;
	va_start (those_dots, format);
	#ifdef ALLEGRO_MSVC
	_vsnprintf(buf, 1000, format, those_dots);
	//#elif NO_VSNPRINTF
	#elif defined VSNPRINTF
	vsnprintf(buf, 1000, format, those_dots);
	#else
	vsprintf(buf, format, those_dots);
	//vsnprintf(buf, 1000, format, those_dots); //it would be nice to use this line...
	#endif
	va_end (those_dots);
	out(buf, dur, c);
	return;
}


void message_type::clean()
{
	STACKTRACE;
	int kill_time;
	if (game) kill_time = game->game_time;
	else kill_time = 0;
	for (int i = 0; i < num_messages; i += 1) {
		if (messages[i].end_time <= kill_time) {
			free (messages[i].string);
			num_messages -= 1;
			memmove (&messages[i], &messages[i+1], (num_messages - i) * sizeof(entry_type));
			i -= 1;
		}
	}
	return;
}


void message_type::flush()
{
	STACKTRACE;
	for (int i = 0; i < num_messages; i += 1) {
		if (messages[i].string)
			free (messages[i].string);
	}
	num_messages = ox = oy = 0;
	return;
}


void message_type::animate(Frame *frame)
{
	STACKTRACE;

	if (num_messages <= 0)
		return;

	int i, x = 0, y = 0, tmp;
	BITMAP *bmp;
	if (frame) {
		text_mode(-1);
		bmp = frame->surface;
	} else {
		text_mode(0);
		videosystem.window.lock();
		bmp = videosystem.window.surface;
	}
	clean();
	if (!frame) rectfill(bmp, 0, 0, ox, oy, 0);
	for (i = 0; i < num_messages; i += 1) {
		textprintf(bmp, font, 0, y, messages[i].color, "%s", messages[i].string);
		tmp = text_length(font, messages[i].string);
		if (x < tmp) x = tmp;
		y += text_height(font);
	}
	if (frame && !frame->full_redraw) frame->add_box(0, 0, x, y);
	if (!frame)
		videosystem.window.unlock();
	ox = x;
	oy = y;
	return;
}


void View::_event( Event *e )
{
	STACKTRACE;
	if ( e->type == Event::VIDEO ) {
		const VideoEvent *ve = (const VideoEvent*) e;
		const VideoWindow *w = ve->window;
		if (w != window) return;
		//if (w->surface) set_window(w->surface, w->x, w->y, w->w, w->h);
	}
}


class View_Everything : public View
{
	public:
		virtual void calculate(Game *game);
};

void View_Everything::calculate(Game *game)
{
	STACKTRACE;
	double a, b, c;
	//sqrt(view_w * view_w + view_h * view_h) / 1.41421356237309504880168872
	c = 1.414 / magnitude(view_size);
	a = map_size.x * view_size.x * c;
	b = map_size.y * view_size.y * c;
	if (b > a) a = b;
	camera.z = a;
	return;
}


/**
 * Weighted-average camera class.  Takes average of values and weights
 * of values fed into it.
 */
class CCameraPosition:public CameraPosition
{
	double totalweight;
	public:
		inline CCameraPosition() { Reset(); }

		inline CCameraPosition(const CameraPosition &cp) {
			totalweight=1.0f;
			pos=cp.pos;
			vel=cp.vel;
			z=cp.z;
		}

		inline void Set(Vector2 ipos, Vector2 ivel, float iz, float weight) {
			totalweight=weight;
			pos=ipos*weight;
			vel=ivel*weight;
			z=iz*weight;
		}

		inline void Accumulate(Vector2 ipos, Vector2 ivel,
		float iz, float weight) {
			pos+=(ipos*weight);
			vel+=(ivel*weight);
			z+=(iz*weight);
			totalweight+=weight;
		}

		/* Take weighted average to get final values */
		inline void Finalize() {
			pos/=totalweight;
			vel/=totalweight;
			z  /=totalweight;
			totalweight=1.0f;
		}

		inline void operator=(const CameraPosition &cp) {
			totalweight=1.0f;
			pos=cp.pos;
			vel=cp.vel;
			z=cp.z;
		}

		inline void Reset() {
			totalweight=0.0f;
			pos.x=pos.y=0.0f;
			vel.x=vel.y=0.0f;
			z          =0.0f;
		}
};

/**
 * "Dynamic" view:  hero-centered intelligent zooming for multiple
 * ships.
 *
 * It has a 'default' zoom level that it gravitates toward, which is
 * fairly large.  Nearby ships draw the focus down to just it and the
 * player in a linear way -- when there are several nearby ships, the
 * focii smoothly balance.
 */
class View_Dynamic : public View
{
	double cutoff;
	double f,power;
	double max, min;

	// Z-value as a function of distance from hero
	static inline float zcurve(float dist) { return dist*2.0f; }
	// Weight/influence as a function of distance from hero
	static inline float wcurve(float dist, float cut) {
		// Logarithmic curves seem the most natural-looking.
		// We use the negative part of the curve from (0,1], scaled to
		// (0,cutoff]
		return((-50.0f)*log(dist/cut));
	}
	public:
		virtual void calculate(Game *game);
		virtual void init(View *old);
};

void View_Dynamic::init(View *old)
{
	STACKTRACE;
	View::init(old);
	power=5;
	f = 2000;
	min = 480;					 //480;
	max = 480000;
	cutoff=2500;
	return;
}


void View_Dynamic::calculate(Game *game)
{
	STACKTRACE;
	int i;
	CCameraPosition cpos=camera;
	SpaceLocation *c=NULL;

	if (game->num_focuses) c=game->focus[game->focus_index]->get_focus();

	// Zoom in/out
	if (key_pressed(key_zoom_in))  cutoff /= 1 + 0.002 * frame_time;
	if (key_pressed(key_zoom_out)) cutoff *= 1 + 0.002 * frame_time;

	if (c==NULL) {
		// Keep camera moving steadily until we've got a target again
		cpos.pos+=(cpos.vel * frame_time);
		cpos.vel /= 1.0f + (0.002f * frame_time);
		// Zoom out slowly
		cpos.z *= 1.0f + (0.0002f * frame_time);
	} else {
		cpos.Set(c->pos,c->vel,f,power);

		for(i=0; i<=game->num_focuses; i++) {
			float dist;
			float cut=cutoff;
			SpaceLocation *obj;

			if (i==game->num_focuses)
				obj=c->nearest_planet();
			else
				obj=game->focus[i]->get_focus();

			// Ignore NULL, or your own ship
			if ((obj==NULL)||(obj==c)) continue;
			// Ignore cloaked ships
			if (!(obj->detectable())) continue;

			// Selected target is visible farther
			if (obj == c->target)
				cut*=2.0f;

			dist=c->distance(obj);
			if (dist > cut) continue;

			cpos.Accumulate( c->pos - (c->rel_pos(obj)/2),
				(c->vel + obj->vel)/2,
				zcurve(dist),
				wcurve(dist,cut));
		}

		cpos.Finalize();
	}

	if (cpos.z < min) cpos.z=min;
	else if (cpos.z > max) cpos.z=max;

	focus(&cpos,NULL);
	track(cpos,frame_time);
}


class View_Hero : public View
{
	double f;
	double max, min;
	public:
		virtual void calculate(Game *game);
		virtual void init(View *old);
		//virtual void set_window (BITMAP *dest, int x, int y, int w, int h);
};
void View_Hero::init(View *old)
{
	STACKTRACE;
	View::init(old);
	f = 0;
	min = 700;
	max = 3000;
	return;
}


void View_Hero::calculate(Game *game)
{
	STACKTRACE;
	CameraPosition n = camera;
	if (key_pressed(key_zoom_in))  n.z /= 1 + 0.002 * frame_time;
	if (key_pressed(key_zoom_out)) n.z *= 1 + 0.002 * frame_time;
	if (n.z < min) n.z = min;
	if (n.z > max) n.z = max;
	if (key_pressed(key_alter1)) f += 0.006 * frame_time;
	else f -= 0.006 * frame_time;
	if (f < 0) f = 0;
	if (f > 1.2) f = 1.2;
	SpaceLocation *c = NULL;
	if (game->num_focuses) c = game->focus[game->focus_index]->get_focus();
	focus ( &n, c );
	if (c) {
		n.pos += (f) * n.z / 4 * unit_vector(c->get_angle_ex());
	}
	track ( n, frame_time );
	return;
}


class View_Enemy : public View
{
	public:
		virtual void calculate(Game *game);
};
void View_Enemy::calculate(Game *game)
{
	STACKTRACE;
	SpaceLocation *c = NULL;
	if (game->num_focuses) c = game->focus[game->focus_index]->get_focus();
	if (!c) return;
	CameraPosition n = camera;
	if (c->target && !(camera_hides_cloakers && c->target->isInvisible())) {
		if (c->distance(c->target) < 3000) {
			focus(&n, c, c->target);
			n.z *= 1.4;
		}
		else focus(&n, c);
	}
	else focus(&n, c);
	if (n.z < 480) n.z = 480;
	track(n);
	return;
}


class View_Enemy_Discrete : public View
{
	public:
		virtual void calculate(Game *game);
};
void View_Enemy_Discrete::calculate(Game *game)
{
	STACKTRACE;
	SpaceLocation *c = NULL;
	if (game->num_focuses) c = game->focus[game->focus_index]->get_focus();
	if (!c) return;
	CameraPosition n = camera;
	if (c->target && !(camera_hides_cloakers && c->target->isInvisible())) {
		focus(&n, c, c->target);
		n.z *= 1.4;
	} else {
		focus(&n, c);
		// but, if the target is invisible, you usually want more zoom to plan where to go, right...
		n.z = 900;
	}

	if (n.z < 480) n.z = 480;

	double ref_size = 480;
	n.z = ref_size * pow( 2, iround( ceil(log(n.z/ref_size) / log(2.0))) );
	track(n);
	return;
}


class View_Split2a : public View
{
	double max, min;
	enum {num_windows = 2};
	CameraPosition cam[num_windows];
	Frame *frames[num_windows];
	public:
		virtual void calculate(Game *game);
		virtual void init(View *old);
		virtual void animate(Game *game);
		virtual ~View_Split2a();
};

void View_Split2a::init(View *old)
{
	STACKTRACE;
	View::init(old);

	min = 480;
	max = 4800;

	int i;
	for (i = 0; i < num_windows; i += 1) cam[i] = camera;
	for (i = 0; i < num_windows; i += 1) frames[i] = new Frame(1024);
	frames[0]->window->locate(0,0.0, 0,0, 0,0.5, 0, 1);
	frames[1]->window->locate(0,0.5, 0,0, 0,0.5, 0, 1);
	return;
}


View_Split2a::~View_Split2a()
{
	int i;
	for (i = 0; i < num_windows; i += 1) {
		delete frames[i];
	}
}


void View_Split2a::animate(Game *game)
{
	STACKTRACE;
	VideoWindow *tmpw;
	Frame *tmpf;
	CameraPosition tmpc;
	int i;

	tmpw = window;
	tmpf = frame;
	tmpc = camera;
	for (i = 0; i < num_windows; i += 1) {
		frames[i]->window->init(window);
	}

	for (i = 0; i < num_windows; i += 1) {
		window = frames[i]->window;
		frame = frames[i];
		camera = cam[i];
		View::animate(game);
	}

	for (i = 0; i < num_windows; i += 1) {
		frames[i]->window->init(NULL);
	}

	frame = tmpf;
	window = tmpw;
	camera = tmpc;
	view_size.x = window->w;
	view_size.y = window->w;
}


void View_Split2a::calculate(Game *game)
{
	STACKTRACE;
	CameraPosition n;
	SpaceLocation *c;
	int i;

	for (i = 0; i < num_windows; i += 1) {
		n = cam[i];
		switch (i) {
			case 0:
				if (key_pressed(key_zoom_in))  n.z /= 1 + 0.002 * frame_time;
				if (key_pressed(key_zoom_out)) n.z *= 1 + 0.002 * frame_time;
				break;
			case 1:
				if (key_pressed(key_alter1)) n.z *= 1 + 0.002 * frame_time;
				if (key_pressed(key_alter2)) n.z /= 1 + 0.002 * frame_time;
				break;
			default:
				break;
		}
		if (n.z < min) n.z = min;
		if (n.z > max) n.z = max;
		c = NULL;
		if (game->num_focuses > i) c = game->focus[(game->focus_index + i) % game->num_focuses]->get_focus();
		focus ( &n, c );
		track ( n, frame_time, &cam[i] );
	}

	camera = cam[0];
	return;
}


class View_Split2b : public View
{
	double max, min;
	enum {num_windows = 2};
	CameraPosition cam[num_windows];
	Frame *frames[num_windows];
	public:
		virtual void calculate(Game *game);
		virtual void init(View *old);
		virtual void animate(Game *game);
		virtual ~View_Split2b();
};

void View_Split2b::init(View *old)
{
	STACKTRACE;
	View::init(old);

	min = 480;
	max = 4800;

	int i;
	for (i = 0; i < num_windows; i += 1) cam[i] = camera;
	for (i = 0; i < num_windows; i += 1) frames[i] = new Frame(1024);
	frames[0]->window->locate(0,0, 0,0.0, 0,1, 0, .5);
	frames[1]->window->locate(0,0, 0,0.5, 0,1, 0, .5);
	return;
}


View_Split2b::~View_Split2b()
{
	int i;
	for (i = 0; i < num_windows; i += 1) {
		delete frames[i];
	}
}


void View_Split2b::animate(Game *game)
{
	STACKTRACE;
	VideoWindow *tmpw;
	Frame *tmpf;
	CameraPosition tmpc;
	int i;

	tmpw = window;
	tmpf = frame;
	tmpc = camera;
	for (i = 0; i < num_windows; i += 1) {
		frames[i]->window->init(window);
	}

	for (i = 0; i < num_windows; i += 1) {
		window = frames[i]->window;
		frame = frames[i];
		camera = cam[i];
		View::animate(game);
	}

	for (i = 0; i < num_windows; i += 1) {
		frames[i]->window->init(NULL);
	}

	frame = tmpf;
	window = tmpw;
	camera = tmpc;
	view_size.x = window->w;
	view_size.y = window->w;
}


void View_Split2b::calculate(Game *game)
{
	STACKTRACE;
	CameraPosition n;
	SpaceLocation *c;
	int i;

	for (i = 0; i < num_windows; i += 1) {
		n = cam[i];
		switch (i) {
			case 0:
				if (key_pressed(key_zoom_in))  n.z /= 1 + 0.002 * frame_time;
				if (key_pressed(key_zoom_out)) n.z *= 1 + 0.002 * frame_time;
				break;
			case 1:
				if (key_pressed(key_alter1)) n.z *= 1 + 0.002 * frame_time;
				if (key_pressed(key_alter2)) n.z /= 1 + 0.002 * frame_time;
				break;
			default:
				break;
		}
		if (n.z < min) n.z = min;
		if (n.z > max) n.z = max;
		c = NULL;
		if (game->num_focuses > i) c = game->focus[(game->focus_index + i) % game->num_focuses]->get_focus();
		focus ( &n, c );
		track ( n, frame_time, &cam[i] );
	}

	camera = cam[0];
	return;
}


class View_Split3 : public View
{
	double max, min;
	enum {num_windows = 3};
	CameraPosition cam[num_windows];
	Frame *frames[num_windows];
	public:
		virtual void calculate(Game *game);
		virtual void init(View *old);
		virtual void animate(Game *game);
		virtual ~View_Split3();
};

void View_Split3::init(View *old)
{
	STACKTRACE;
	View::init(old);

	min = 480;
	max = 4800;

	int i;
	for (i = 0; i < num_windows; i += 1) cam[i] = camera;
	for (i = 0; i < num_windows; i += 1) frames[i] = new Frame(1024);
	frames[0]->window->locate(0,0.0/3, 0,0, 0,1/3.0, 0,1);
	frames[1]->window->locate(0,1.0/3, 0,0, 0,1/3.0, 0,1);
	frames[2]->window->locate(0,2.0/3, 0,0, 0,1/3.0, 0,1);
	return;
}


View_Split3::~View_Split3()
{
	int i;
	for (i = 0; i < num_windows; i += 1) {
		delete frames[i];
	}
}


void View_Split3::animate(Game *game)
{
	STACKTRACE;
	VideoWindow *tmpw;
	Frame *tmpf;
	CameraPosition tmpc;
	int i;

	tmpw = window;
	tmpf = frame;
	tmpc = camera;
	for (i = 0; i < num_windows; i += 1) {
		frames[i]->window->init(window);
	}

	for (i = 0; i < num_windows; i += 1) {
		window = frames[i]->window;
		frame = frames[i];
		camera = cam[i];
		View::animate(game);
	}

	for (i = 0; i < num_windows; i += 1) {
		frames[i]->window->init(NULL);
	}

	frame = tmpf;
	window = tmpw;
	camera = tmpc;
	view_size.x = window->w;
	view_size.y = window->w;
}


void View_Split3::calculate(Game *game)
{
	STACKTRACE;
	CameraPosition n;
	SpaceLocation *c;
	int i;

	for (i = 0; i < num_windows; i += 1) {
		n = cam[i];
		switch (i) {
			case 0:
				if (key_pressed(key_zoom_in))  n.z /= 1 + 0.002 * frame_time;
				if (key_pressed(key_zoom_out)) n.z *= 1 + 0.002 * frame_time;
				break;
			case 1:
				if (key_pressed(key_alter1)) n.z *= 1 + 0.002 * frame_time;
				if (key_pressed(key_alter2)) n.z /= 1 + 0.002 * frame_time;
				break;
			default:
				break;
		}
		if (n.z < min) n.z = min;
		if (n.z > max) n.z = max;
		c = NULL;
		if (game->num_focuses > i) c = game->focus[(game->focus_index + i) % game->num_focuses]->get_focus();
		focus ( &n, c );
		track ( n, frame_time, &cam[i] );
	}

	camera = cam[0];
	return;
}


class View_Split4 : public View
{
	double max, min;
	enum {num_windows = 4};
	CameraPosition cam[num_windows];
	Frame *frames[num_windows];
	public:
		virtual void calculate(Game *game);
		virtual void init(View *old);
		virtual void animate(Game *game);
		virtual ~View_Split4();
};

void View_Split4::init(View *old)
{
	STACKTRACE;
	View::init(old);

	min = 480;
	max = 4800;

	int i;
	for (i = 0; i < num_windows; i += 1) cam[i] = camera;
	for (i = 0; i < num_windows; i += 1) frames[i] = new Frame(1024);
	frames[0]->window->locate(0,0.0/2, 0,0.0/2, 0,0.5, 0,0.5);
	frames[1]->window->locate(0,1.0/2, 0,0.0/2, 0,0.5, 0,0.5);
	frames[2]->window->locate(0,0.0/2, 0,1.0/2, 0,0.5, 0,0.5);
	frames[3]->window->locate(0,1.0/2, 0,1.0/2, 0,0.5, 0,0.5);
	return;
}


View_Split4::~View_Split4()
{
	int i;
	for (i = 0; i < num_windows; i += 1) {
		delete frames[i];
	}
}


void View_Split4::animate(Game *game)
{
	STACKTRACE;
	VideoWindow *tmpw;
	Frame *tmpf;
	CameraPosition tmpc;
	int i;

	tmpw = window;
	tmpf = frame;
	tmpc = camera;
	for (i = 0; i < num_windows; i += 1) {
		frames[i]->window->init(window);
	}

	for (i = 0; i < num_windows; i += 1) {
		window = frames[i]->window;
		frame = frames[i];
		camera = cam[i];
		View::animate(game);
	}

	for (i = 0; i < num_windows; i += 1) {
		frames[i]->window->init(NULL);
	}

	frame = tmpf;
	window = tmpw;
	camera = tmpc;
	view_size.x = window->w;
	view_size.y = window->w;
}


void View_Split4::calculate(Game *game)
{
	STACKTRACE;
	CameraPosition n;
	SpaceLocation *c;
	int i;

	for (i = 0; i < num_windows; i += 1) {
		n = cam[i];
		switch (i) {
			case 0:
				if (key_pressed(key_zoom_in))  n.z /= 1 + 0.002 * frame_time;
				if (key_pressed(key_zoom_out)) n.z *= 1 + 0.002 * frame_time;
				break;
			case 1:
				if (key_pressed(key_alter1)) n.z *= 1 + 0.002 * frame_time;
				if (key_pressed(key_alter2)) n.z /= 1 + 0.002 * frame_time;
				break;
			default:
				break;
		}
		if (n.z < min) n.z = min;
		if (n.z > max) n.z = max;
		c = NULL;
		if (game->num_focuses > i) c = game->focus[(game->focus_index + i) % game->num_focuses]->get_focus();
		focus ( &n, c );
		track ( n, frame_time, &cam[i] );
	}

	camera = cam[0];
	return;
}


REGISTER_VIEW ( View_Hero, "Hero" );
REGISTER_VIEW ( View_Enemy, "Enemy" );
REGISTER_VIEW ( View_Enemy_Discrete, "Enemy_Discrete" );
REGISTER_VIEW ( View_Split2a, "Split_2_Horizontal" );
REGISTER_VIEW ( View_Split2b, "Split_2_Vertical" );
REGISTER_VIEW ( View_Split3, "Split_3_Horizontal" );
REGISTER_VIEW ( View_Split4, "Split_4_Quad" );
REGISTER_VIEW ( View_Everything, "Everything" );
REGISTER_VIEW ( View_Dynamic, "Dynamic" );
