#include <stdio.h>
#include <string.h>
#include <time.h>

#include "lib/libircclient/libircclient.h"
#include "lib/sqlite/sqlite3.h"


typedef struct
{
	char* channel;
	char* nick;	
} irc_ctx_t;

//----------------- Gloabel Variablen

irc_ctx_t ctx;
char * server = NULL;
short unsigned int port = 6667;
char * botnick = NULL;

//-- Configfile Vars

#define BUFFER_SIZE 1000

char* filename = NULL;
char configLine[BUFFER_SIZE];
FILE *configFile;

//-- Time Vars

#define GMT (+2)

time_t rawtime;
struct tm * ptm;


int configfileLaden(char * filen)
{

	printf(" : - Lade aus %s \n",filen);
	
	configFile = fopen(filen,"r");
		
	if(configFile  == NULL)
	{
		printf("X: - Bot konnte Configfile nicht lesen... \n");
		fclose(configFile);
		return 0;
	}

	char tmp[BUFFER_SIZE];

 	while(fgets(configLine, BUFFER_SIZE, configFile) != NULL)
	{
		sprintf(tmp,"%s",configLine);
	}

	fclose(configFile);

	//printf("%s",tmp);

	char params[10][100];
	int i=0;

	char *ptr =strtok(tmp,";");

	while(ptr !=NULL)
	{
		sprintf(params[i],"%s",ptr);
		ptr = strtok(NULL, ";");
		i++;
	}
	
//	strcpy(server,params[0]);
//	strcpy(params[0],server);
	server = params[0];
	ctx.nick = params[1];
	ctx.channel = params[2];
	
	return 1;
}

void ircCommands(irc_session_t * session,const char * origin,const char ** params)
{
	if ( !strcmp (params[1], "!quit") )
		irc_cmd_quit (session, "Bot wird beendet...");
	
	if ( strstr (params[1], "!nick") == params[1] )
	{
		//strcpy(params[1] + 6, nick );
		irc_cmd_nick (session, params[1] + 6);
	}
	
	if ( strstr (params[1], "!join") == params[1] )
	{
		irc_cmd_join (session, params[1] + 6,0);
	}

	if ( strstr (params[1], "!part") == params[1] )
	{
		if(strstr(params[0],ctx.channel) == 0)
		{
			irc_cmd_part (session, params[0]);
		}
		else
		{
			irc_cmd_msg(session,params[0],"Hier geh ich net raus");
		}	
	
	}

	if(strstr(params[1],"!time"))
	{	
		char tmp[60];
		sprintf(tmp,"Es ist %2d:%02d\n", (ptm->tm_hour+GMT)%24, ptm->tm_min);
		irc_cmd_msg(session, params[0][0] =='#' ? params[0] : origin ,tmp);
	}

	if ( !strcmp (params[1], "!topic") )
		irc_cmd_topic (session, params[0], 0);
	else if ( strstr (params[1], "!topic ") == params[1] )
		irc_cmd_topic (session, params[0], params[1] + 7);

	if(strstr(params[1],ctx.nick))
	{
		irc_cmd_msg(session, params[0][0] =='#' ? params[0] : origin ,"me?");
	}

}


void event_connect (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	irc_ctx_t * ctx = (irc_ctx_t *) irc_get_ctx (session);
	
	irc_cmd_join (session, ctx->channel, 0);
}

void event_join (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	irc_cmd_user_mode (session, "+i");
	//irc_cmd_msg (session, params[0], "Hi all");
}

void event_privmsg (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	printf ("'%s' said me (%s): %s\n", origin ? origin : "someone",	params[0], params[1] );

	ircCommands(session,origin,params);
}

void event_channel (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	printf("'%s' said in channel %s: %s\n",origin ? origin : "someone",params[0],params[1]);
		
	 ircCommands(session,origin,params);
}

int main(int argc, char** argv)
{
	irc_callbacks_t callbacks;
	irc_session_t *s;
	
	//Time init
	
	time ( &rawtime );
	ptm = gmtime ( &rawtime );


	printf(" : - Bot wird gestartet...\n");

	switch(argc)
	{
		case 2:
		{
			int tmp = -1;
			tmp = configfileLaden(argv[1]);

			if(tmp==0)
			{
				return 1;
			}
			printf(" : - Bot Configfile geladen \n");
		}break;


		case 4:
			ctx.channel 	= argv[3];
			ctx.nick 	= argv[2];
			server		= argv[1];
		
			printf(" : - Bot configuriert (Parameter) \n");

			break;
		default:
			printf ("!: - Parameter fehlen\n");
			printf ("!: - %s <server> <nick> '<#channel>' \n",argv[0]);
			printf ("!: - %s <configfile> \n",argv[0]);

			return 1;

	}
	
	printf(" : -   Server: %s \n",server);
	printf(" : -   Nick: %s \n",ctx.nick);
	printf(" : -   Channel: %s \n",ctx.channel);

	memset(&callbacks, 0, sizeof(callbacks));
	
	//-- Events --
	callbacks.event_connect = event_connect;
	callbacks.event_join = event_join;
	callbacks.event_channel = event_channel;
	callbacks.event_privmsg = event_privmsg;
	//--

	
	//	ctx.channel = "#Bot";
	//	ctx.nick = "FH-BoT";
	
	if(server == NULL)
	{
		printf("?: - setzt Standartserver\n");
		server = "localhost";
	}

	s = irc_create_session(&callbacks);
	
	if(!s)
	{
		printf("X: - Konnte keine IRC Session aufbauen...\n");
		return 1;
	}

	irc_set_ctx (s,&ctx);
	irc_option_set (s, LIBIRC_OPTION_STRIPNICKS);

	if( irc_connect (s, server,port, 0,ctx.nick,0,0))
	{
		printf("X: - Konnte keine Verbindung zum Server aufbauen...\n");
		return 1;
	}	
	
	if( irc_run (s) )
	{
		printf (" : - Bot wurde beendet. \n");
		return 1;
	}

	return 1;
}

