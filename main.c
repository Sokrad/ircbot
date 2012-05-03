#include <stdio.h>
#include <string.h>

#include "include/libircclient.h"


typedef struct
{
	char* channel;
	char* nick;
} irc_ctx_t;

void event_connect (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	irc_ctx_t * ctx = (irc_ctx_t *) irc_get_ctx (session);
	irc_cmd_join (session, ctx->channel, 0);
}

void event_join (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	irc_cmd_user_mode (session, "+i");
	irc_cmd_msg (session, params[0], "Hi all");
}

void event_channel (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	//printf("'%s' said in channel %s: %s\n",origin ? origin : "someone",params[0],params[1]);
		
	if ( !strcmp (params[1], "!quit") )
		irc_cmd_quit (session, "of course, Master!");
}

int main(int argc, char** argv)
{
	irc_callbacks_t callbacks;
	irc_ctx_t ctx;
	irc_session_t *s;

	/*if(argc != 4)
	{
		printf ("bot brauch das\n");
		return 1;
	}*/



	memset(&callbacks, 0, sizeof(callbacks));
	
	//-- Events --
	callbacks.event_connect = event_connect;
	callbacks.event_join = event_join;
	callbacks.event_channel = event_channel;
	//--

	ctx.channel = "#Bot";
	ctx.nick = "FH-BoT";

	s = irc_create_session(&callbacks);
	
	if(!s)
	{
		printf("Error");
		return 1;
	}

	irc_set_ctx (s,&ctx);
	irc_option_set (s, LIBIRC_OPTION_STRIPNICKS);

	if( irc_connect (s, "localhost",6667, 0,ctx.nick,0,0))
	{
		printf("Konnte keine verbindung zum Server aufbauen...\n");
		return 1;
	}	
	
	irc_run (s);

	printf("bot laeuft\n");

	return 0;
}

