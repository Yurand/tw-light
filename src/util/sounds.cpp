#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "base.h"
#include "sounds.h"
#include "errors.h"				 //only used for log_debug() function

//#if defined DIGI_DIRECTAMX
//#	define DIGI_TW DIGI_DIRECTAMX(0)
//#else
#   define DIGI_TW DIGI_AUTODETECT
//#endif

#define MIDI_TW MIDI_NONE

SoundSystem tw_soundsystem;
SoundSystem::SoundSystem()
{
	STACKTRACE;
	sound_volume = 255;
	music_volume = 255;
	sound_on = false;
	music_on = false;
	state = 0;
	fake_mod_playing = false;
	current_music = NULL;
}


int SoundSystem::is_music_supported() const
{
	return 1;
}


int SoundSystem::is_music_playing() const
{
	if (current_music) {
		if (current_voice == -1)
			return 0;
		if (voice_get_position(current_voice) == -1)
			return 0;
		return 1;
	}
	return 0;
}


void SoundSystem::disable()
{
	STACKTRACE;
	if (state & ENABLED) {
		::remove_sound();
		log_debug("Sound disabled\n");
	}
	state &= ~ENABLED;
	state |= DISABLED;
	sound_channels = 0;
	music_channels = 0;
}


void SoundSystem::load()
{
	STACKTRACE;
	int sv, mv, so, mo;
	so = get_config_int("Sound", "SoundOn", sound_on);
	mo = get_config_int("Sound", "MusicOn", music_on);
	sv = get_config_int("Sound", "SoundVolume", sound_volume);
	mv = get_config_int("Sound", "MusicVolume", music_volume);
	set_volumes(sv, mv, so, mo);
	return;
}


void SoundSystem::save()
{
	STACKTRACE;
	set_config_int("Sound", "SoundOn", sound_on);
	set_config_int("Sound", "MusicOn", music_on);
	set_config_int("Sound", "SoundVolume", sound_volume);
	set_config_int("Sound", "MusicVolume", music_volume);
	return;
}


void SoundSystem::init()
{
	STACKTRACE;
	if (state & (ENABLED | DISABLED))
		return;
	state |= DISABLED;

	if (install_sound(DIGI_TW, MIDI_TW, "") < 0) {
		log_debug("Sound initialization failed\n");
		log_debug("allegro_error: %s\n", allegro_error);
		sound_channels = 0;
		music_channels = 0;
		return;
	}
	log_debug("Sound initialization succeeded\n");
	//	::set_volume(255, 255);
	state &=~DISABLED;
	state |= ENABLED;
	return;
}


int SoundSystem::play (SAMPLE *spl, int vol, int pan, int freq, bool loop)
{
	STACKTRACE;
	if ((state & (ENABLED | SOUND_ON)) == (ENABLED | SOUND_ON)) {
		#ifdef TW_DEBUG_SOUND
		log_debug("SoundSystem::play %p, %d, %d, %d %s\n%s\n", spl, vol, pan, freq, ((loop)?"true":"false"), _stacktrace_.get_stack_trace_string().c_str());
		#endif
		return ::play_sample (spl, (vol * sound_volume) >> 8, pan, freq, loop);
	}
	return -1;
}


void SoundSystem::stop (SAMPLE *spl)
{
	STACKTRACE;
	if (state & ENABLED) {
		#ifdef TW_DEBUG_SOUND
		log_debug("SoundSystem::stop %p\n", spl);
		#endif
		::stop_sample (spl);
		return;
	}
}


void SoundSystem::play_music (Music *music, int loop)
{
	STACKTRACE;
	if ((state & (MUSIC_ON)) == MUSIC_ON) {
		if (current_music) {
			stop(current_music);
		}
		current_voice = play(music, music_volume, 128, 1000, loop);
		current_music = music;
	}
	return;
}


void SoundSystem::stop_music ()
{
	STACKTRACE;
	if ((state & (MUSIC_ON)) == MUSIC_ON) {
		if (current_music)
			stop(current_music);
		current_music = NULL;
		current_voice = -1;
	}
}


void SoundSystem::set_volumes(int sound_volume, int music_volume, int sound_on, int music_on)
{
	STACKTRACE;
	if (!(state & (ENABLED | DISABLED))) {
		//error("sound system not initialized");
		init();
	}
	this->sound_volume = sound_volume & 255;
	this->music_volume = music_volume & 255;
	this->sound_on = sound_on;
	this->music_on = music_on;
	state &= ~(SOUND_ON | MUSIC_ON);
	if (sound_on)
		state |= SOUND_ON;
	if (music_on)
		state |= MUSIC_ON;
}
