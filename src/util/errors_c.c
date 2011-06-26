#include <string.h>
#include <stdio.h>
#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif
#include <stdarg.h>

//#include "base.h"
#include "errors.h"

int __error_flag = 0;

static void default_error_handler ( int in_catch_statement, const char * file, int line, const char * message )
{
	log_debug("!!!\nDefault error handler is terminating the program\n");
	tw_exit(1);
}


void (*_error_handler) ( int in_catch_statement, const char *file, int line, const char *message) = &default_error_handler;

static const char *_error_file = NULL;
static int         _error_line = -1;

static void __error(const char *format, ...)
{
	char error_string[2048];
	int line;
	const char * file;
	line = _error_line;
	file = _error_file;
	_error_line = -1;
	_error_file = NULL;

	if (format) {
		va_list those_dots;
		va_start(those_dots, format);
		vsprintf(error_string, format, those_dots);
		va_end(those_dots);
	}
	else error_string[0] = 0;
	_error_handler( 0, file, line, error_string);
}


// A helper function used by tw_error()
void _prep_error(const char *file, int line)
{
	_error_file = file;
	_error_line = line;
}


//the new version of that helper function:
ERROR_FUNC_TYPE _prep_error_func ( const char *file, int line )
{
	_prep_error(file, line);
	return &__error;
}


void _error(const char *format, ...)
{
	if (_error_line == -1) return;
	else {
		va_list those_dots;
		va_start(those_dots, format);
		//vsprintf(error_string, format, those_dots);
		__error(format, those_dots);
		va_end(those_dots);
	}
}


void error(const char *format, ...)
{
	char error_string[2048];

	if (format) {
		va_list those_dots;
		va_start(those_dots, format);
		vsprintf(error_string, format, those_dots);
		va_end(those_dots);
	}
	else error_string[0] = 0;

	_error_handler(0, NULL, -1, error_string);
}


static FILE *debug_log_file = NULL;

void tw_exit(int errorcode)
{
	allegro_exit();
	log_debug("Shutting Down (%d)\n", errorcode);
	if (debug_log_file)
		fclose(debug_log_file);
	exit(1);
}


void log_debug(const char *format, ...)
{
	char buffy[4096];

	if (debug_log_file && format) {
		va_list those_dots;
		va_start(those_dots, format);
		//vfprintf(debug_log_file, format, those_dots);
		vsprintf(buffy, format, those_dots);
		fprintf(debug_log_file, "%s", buffy);
		printf("%s", buffy);
		va_end(those_dots);
		fflush(debug_log_file);
	}

	if (!debug_log_file && !format)
		debug_log_file = fopen("tw_sys.log", "wt");

	return;
}


void error_oom()
{
	if (debug_log_file) {
		fprintf(debug_log_file, "\nCritical error: OUT OF MEMORY\n");
		fflush(debug_log_file);
		fclose(debug_log_file);
	}
	tw_exit(1);
}


#if defined(USE_ALLEGRO) && defined(DO_STACKTRACE)
#else

void init_error()
{
	log_debug(NULL);
}


void deinit_error()
{
}
#endif
