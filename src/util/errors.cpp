#include <string.h>
#include <stdio.h>
#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif
#include <stdarg.h>

#include "melee.h"

#include "other/dialogs.h"

static DIALOG *tw_alert_dialogs[5] =
{
	tw_alert_dialog1,
	tw_alert_dialog2,
	tw_alert_dialog3,
	tw_alert_dialog4,
	tw_alert_dialog5
};

char find_shortcut_key(const char *s)
{
	while (true) {
		s = strchr(s, '&');
		if (s && (s[1] != '&')) {
			s++;
			break;
		}
		if (!s)
			return 0;
	}
	return *s;
}


int _tw_alert (
bool popup,
char *message,
const char *b1,
const char *b2 = NULL,
const char *b3 = NULL,
const char *b4 = NULL,
const char *b5 = NULL
)
{
	int l = 1;
	if (b2)
		l = 2;
	if (b3)
		l = 3;
	if (b4)
		l = 4;
	if (b5)
		l = 5;
	DIALOG *dialog = tw_alert_dialogs[l-1];
	dialog[1].dp = (void*)message;
	if (b1) {
		dialog[2].dp = (void*)b1;
		dialog[2].key = find_shortcut_key(b1);
	}
	if (b2) {
		dialog[3].dp = (void*)b2;
		dialog[3].key = find_shortcut_key(b2);
	}
	if (b3) {
		dialog[4].dp = (void*)b3;
		dialog[4].key = find_shortcut_key(b3);
	}
	if (b4) {
		dialog[5].dp = (void*)b4;
		dialog[5].key = find_shortcut_key(b4);
	}
	if (b5) {
		dialog[6].dp = (void*)b5;
		dialog[6].key = find_shortcut_key(b5);
	}

	int i;

	if (popup)
		i = tw_popup_dialog(&videosystem.window, dialog, 2) - 2;
	else
		i = tw_do_dialog   (&videosystem.window, dialog, 2) - 2;
	return i + 1;
}


int tw_alert(const char *message, const char *b1, const char *b2, const char *b3, const char *b4)
{
	char *s1 = strdup(message);
	int r = _tw_alert( true, s1, b1, b2, b3, b4 );
	free(s1);
	return r;
}


static void tw_error_handler (
int in_catch_statement,
const char *file,
int line,
const char *message
)
{
	char error_string[32000];
	int i;

	if (__error_flag & 2) return;

	char *cp = &error_string[0];
	if (file) {
		char *_file = (char*)strstr(file, "source");
		if (_file) file = _file;
	}

	log_debug("tw_error_handler invoked: ");
	int len = strlen(message);
	memcpy(error_string, message, len);
	cp += len;
	if (line >= 0) {
		log_debug(        "(from %s, Line %d)\n", file, line);
		cp += sprintf(cp, "\n(from %s, line %d)\n", file, line);
	} else {
		log_debug("(from unspecified file & linenumber)\n");
		cp += sprintf(cp, "\n");
	}
	cp += sprintf(cp, "\nStack Trace:\n");
	cp += sprintf(cp, "%s", UserStackTraceHelper::get_stack_trace_string().c_str());

	log_debug("begin error message:\n%s\nend error message\n", message);
	log_debug(UserStackTraceHelper::get_stack_trace_string().c_str());

	if (videosystem.width <= 0) {
		allegro_message("Critical Error$: %s\n", error_string);
		log_debug("\nUnable to display messge, shutting down\n");
		throw -1;
	}

	log_debug("Pressenting graphical error prompt\n");

	const char *es[] = {"&Abort", "&Retry", "&Debug", "&Ignore", "&Stack Trace"};
	enum {
		ES_ABORT = 0,
		ES_RETRY,
		ES_DEBUG,
		ES_IGNORE,
		ES_STACKTRACE
	};
	int selection = -1;

	i = _tw_alert( false, error_string, es[ES_ABORT], es[ES_RETRY],
		es[ES_DEBUG], es[ES_IGNORE], es[ES_STACKTRACE] );
	if (i > 0)
		selection = i - 1;

	if (selection < 0)
		selection = ES_ABORT;
	log_debug("Option \"%s\" selected\n", es[selection]);

	videosystem.screen_corrupted = true;

	if (selection == ES_DEBUG) { //"Debug"
		__error_flag |= 1;
		if (in_catch_statement) {
			return;
		}
		#       if defined ALLEGRO_MSVC
		__asm int 3;
		#       elif defined __GNUC__ && defined __i386__
		asm("int $0x03");
		#       else
		if (1) (*((int*)NULL)) = 0;
		#       endif
		return;
	}

								 //"Ignore"
	if (selection == ES_IGNORE) {
		__error_flag |= 2;
		return;
	}

	if (selection == ES_RETRY) { //"Retry"
		return;
	}

	if (selection == ES_STACKTRACE) {
		show_text(UserStackTraceHelper::get_stack_trace_string().c_str());
		tw_error_handler ( in_catch_statement, file, line, message );
		return;
	}
	//"Abort"
	if (in_catch_statement)
		return;
	else throw 0;
}


static void _register_tw_error_hanlde() {_error_handler = &tw_error_handler;}
CALL_BEFORE_MAIN(_register_tw_error_hanlde);

void tw_error_exit(const char* message)
{
	log_debug("\nCritical Error!: %s\n\n", message);
	if ((videosystem.width > 0) && (strlen(message) < 1000)) {
		char buf[1024];
		sprintf(buf, "Critical Error!: %s", message);
		tw_alert (buf, "Quit");
	}
	else
		allegro_message("Critical Error!: %s\n", message);

	tw_exit(1);
}


//const int tw_error_str_len = 2048;	// should be pleny of room.
//char tw_error_str[tw_error_str_len];

void caught_error(const char *format, ...)
{
	char error_string[4096];
	if (format) {
		va_list those_dots;
		va_start(those_dots, format);
		vsprintf(error_string, format, those_dots);
		va_end(those_dots);
	}

	tw_error_handler( 1, NULL, -1, error_string);
	// tw_error_handler( 1, NULL, -1, format);

	return;
}


std::stack<SOURCE_LINE*> UserStackTraceHelper::call_stack;
UserStackTraceHelper::UserStackTraceHelper( SOURCE_LINE* srcline)
{
	#ifdef DEBUG
	//  debug_log(GetStackNodeString(srcline).c_str());
	#endif
	call_stack.push(srcline);
}


UserStackTraceHelper::~UserStackTraceHelper()
{
	call_stack.pop();
}


std::string UserStackTraceHelper::GetStackNodeString(SOURCE_LINE* stNode)
{
	char line[20] = {0};
	sprintf(line, "%d", stNode->line);

	std::string strName;
	if (stNode->name!=NULL)
		strName = std::string(" ") + stNode->name;

	return std::string("from: ") + stNode->file + ", line " + line + " funct: " + stNode->funct + strName + "\n";
}


std::string UserStackTraceHelper::get_stack_trace_string()
{
	std::string stack_string = "";

	std::stack<SOURCE_LINE*> tmp;
	tmp = call_stack;
	while(!tmp.empty()) {
		SOURCE_LINE* stNode = tmp.top();
		stack_string += GetStackNodeString(stNode);
		tmp.pop();
	}
	return stack_string;
}


void init_error()
{
}


void deinit_error()
{
}
