#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "lib/libircclient/libircclient.h"
#include <sqlite3.h>
//#include "lib/sqlite/sqlite3.h"


typedef struct
{
	char* channel;
	char* nick;	
} irc_ctx_t;

typedef struct
{
	char channel[32];
	unsigned int settings;
}channel_settings;

//----------------- SQLite Variablen

#define SQLITEFILE "sqlite.sqlite"
sqlite3 *sqlitedb = NULL;

//----------------- Gloabel Variablen

char* LOGFILE = "log.txt";

#define MAX_CHANNELS 10

irc_ctx_t ctx;
char * server = NULL;
short unsigned int port = 6667;
char botnick[100];
unsigned int currentChannelCount=0;

//-- Configfile Vars

#define BUFFER_SIZE 1000

char* filename = NULL;
char configLine[BUFFER_SIZE];
FILE *configFile;

//--- IRC Commands Defines
#define TOPIC_CHANGE	1
#define NICK_CHANGE	2
#define JOIN_PART	4
#define QUIT		8
#define GET_TIME	16
#define LOG_TXT 	32
#define URL_SQL		64

//--- IRCC DEFAULT SETTINGS
#define DEFAULT_CHANNEL_SETTINGS 31
#define DEFAULT_PRIVMSG_SETTINGS 127

unsigned int privmsg_settings = DEFAULT_PRIVMSG_SETTINGS;
channel_settings chansettings[MAX_CHANNELS];


//-- Time Vars

#define GMT (+2)

time_t rawtime;
struct tm * ptm;


void log_file(const char name[],const char channel[],const char text[])
{
	time ( &rawtime );
	ptm = gmtime ( &rawtime );
	
	FILE * log = fopen(LOGFILE,"a");
	
	fprintf(log,"%02d.%02d.%4d - %02d:%02d:%02d - <%s> %s: %s\n",ptm->tm_mday,ptm->tm_mon,ptm->tm_year+1900,(ptm->tm_hour+GMT)%24, ptm->tm_min,ptm->tm_sec,channel,name,text);
	fclose(log);
	
}


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

	char params[10][100];
	int i=0;

	char *ptr =strtok(tmp,";");

	while(ptr !=NULL)
	{
		sprintf(params[i],"%s",ptr);
		ptr = strtok(NULL, ";");
		i++;
	}
	
	server = params[0];
	ctx.nick = params[1];
	ctx.channel = params[2];
	
	return 1;
}

void setChannelSettings(const char channel[],const unsigned int settings)
{
	
	int i;
	for(i=0;i<MAX_CHANNELS;i++)
	{
		if(strstr(chansettings[i].channel,channel))
		{
			chansettings[i].settings = settings;
			break;
		}
	}

}

void rmChannel(const char channel[])
{
	int i;
	for(i=0;i<MAX_CHANNELS;i++)
	{
		if(strstr(chansettings[i].channel,channel))
		{
			chansettings[i].channel[0] = 'X';
			break;
		}
	}

}

void addChannelSettings(const char channel[],const unsigned int settings)
{
	strcpy(chansettings[(currentChannelCount-1)].channel,channel);
	chansettings[(currentChannelCount-1)].settings = settings;
	printf(" : - %s config set = %d\n",chansettings[(currentChannelCount-1)].channel,chansettings[(currentChannelCount-1)].settings);
}

unsigned int getChannelSettings(const char channel[])
{
	int i;
	for(i=0;i<MAX_CHANNELS;i++)
	{
		//printf("%c-\n",chansettings[i].channel[0] );
		if(chansettings[i].channel[0] != '#')
		{
			addChannelSettings(channel,DEFAULT_CHANNEL_SETTINGS);
			return DEFAULT_CHANNEL_SETTINGS;
		}
//		printf("%s=%s",channel,chansettings[i].channel);

		if(strcmp(channel,chansettings[i].channel) == 0)
		{
			return chansettings[i].settings;
		}			
	}
	
}


void ircCommands(irc_session_t * session,const char * origin,const char ** params,unsigned int settings)
{
	
	if (settings & QUIT && !strcmp (params[1], "!quit") )
		irc_cmd_quit (session, "Bot wird beendet...");
		
	if(strstr(params[1],botnick))
	{
		irc_cmd_msg(session, params[0][0] =='#' ? params[0] : origin ,"me?");
	}
	
	if (settings & NICK_CHANGE && strstr (params[1], "!nick") == params[1] )
	{
		sprintf(botnick,"%s",params[1] + 6);
		irc_cmd_nick (session, params[1] + 6);
	}

	if(settings & JOIN_PART)
	{	
		if(currentChannelCount < MAX_CHANNELS)
			if (strstr (params[1], "!join") == params[1] )
			{
				irc_cmd_join (session, params[1] + 6,0);
				currentChannelCount++;
			}

		if (strstr (params[1], "!part") == params[1] )
		{
			if(strstr(params[0],ctx.channel) == 0)
			{
				irc_cmd_part (session, params[0]);
				rmChannel(params[0]);
				currentChannelCount--;
			}
			else
			{
				irc_cmd_msg(session,params[0],"Hier geh ich net raus");
			}	
		
		}
	}

	if(settings & GET_TIME && strstr(params[1],"!time"))
	{	
		char tmp[60];
		time ( &rawtime );
		ptm = gmtime ( &rawtime );
	
		sprintf(tmp,"Es ist %2d:%02d\n", (ptm->tm_hour+GMT)%24, ptm->tm_min);
		irc_cmd_msg(session, params[0][0] =='#' ? params[0] : origin ,tmp);
	}

	if(settings & TOPIC_CHANGE)
	{
		if ( !strcmp (params[1], "!topic") )
			irc_cmd_topic (session, params[0], 0);
		else if ( strstr (params[1], "!topic ") == params[1] )
			irc_cmd_topic (session, params[0], params[1] + 7);
	}


	if(strstr(params[1],"!status"))
	{
		char tmp[80];
		sprintf(tmp,"Channelsettings = %d",getChannelSettings(params[0]));
		irc_cmd_msg(session,params[0],tmp);
	}

	if(strstr(params[1],"!set"))
	{
		int settings;
		char line[200];
		char parts[32][32];
		int i = 0;
		
		strcpy(line,params[1]);

		char *ptr =strtok(line," ");

		while(ptr !=NULL)
		{
			sprintf(parts[i],"%s",ptr);
			ptr = strtok(NULL, " ");
			i++;
		}
		
		settings = atoi(parts[2]);

		if(strstr(parts[2],"privmsg"))
		{		
			privmsg_settings = settings;	
		}
		else
		{
			setChannelSettings(parts[1],settings);
		}
	}

	if(strstr(params[1],"!debug"))
	{	
		int i;
		for(i=0;i<MAX_CHANNELS;i++)
			printf("%d=%s=%d\n",i,chansettings[i].channel,chansettings[i].settings);
	}
}

void sql_createtables()
{
	sqlite3_exec(sqlitedb, "CREATE TABLE urls (id integer primary key, nick text, channel text,  url text);", NULL, NULL, NULL);
}

void sql_addurl(const char name[],const char channel[],const char url[])
{
	char tmp[1200];
	sprintf(tmp,"INSERT INTO urls (nick,channel, url) VALUES ('%s', '%s', '%s');",name,channel,url);
	sqlite3_exec(sqlitedb, tmp, NULL, NULL, NULL);
}

void sql_geturls()
{
	sqlite3_stmt *vm;
        sqlite3_prepare(sqlitedb, "SELECT * FROM urls", -1, &vm, NULL);

        printf("ID:\tnick\tchannel\turl\n");

	while (sqlite3_step(vm) != SQLITE_DONE)
        {
                printf("%i\t%s\t%s\t%s\n", 
	                sqlite3_column_int(vm, 0), 
        	        sqlite3_column_text(vm, 1),
			sqlite3_column_text(vm, 2),
                	sqlite3_column_text(vm, 3)
                );
        }
        sqlite3_finalize(vm);
	
}

void sql_geturl(irc_session_t *session,const char name[],int counter)
{
	sqlite3_stmt *vm;
        sqlite3_prepare(sqlitedb, "SELECT * FROM urls ORDER BY id DESC", -1, &vm, NULL);
	
	int i = 0;	

	//printf("ID:\tnick\tchannel\turl\n");

	while (sqlite3_step(vm) != SQLITE_DONE)
        {
                char tmp[1200];
		sprintf(tmp,"%s (%s - %s)",sqlite3_column_text(vm, 3),sqlite3_column_text(vm, 1),sqlite3_column_text(vm, 2));
		irc_cmd_msg(session,name,tmp);
		i++;
		if(i==counter)
			break;
        }
        sqlite3_finalize(vm);
	
}



void event_connect (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	sprintf(botnick,"%s",ctx.nick);
	irc_ctx_t * ctx = (irc_ctx_t *) irc_get_ctx (session);
	irc_cmd_join (session, ctx->channel, 0);
	currentChannelCount++;
}

void event_join (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	irc_cmd_user_mode (session, "+i");
	//irc_cmd_msg (session, params[0], "Hi all");
}

void event_privmsg (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	//printf ("'%s' said me (%s): %s\n", origin ? origin : "someone",	params[0], params[1] );
	if(privmsg_settings & LOG_TXT)
		log_file(origin,"privmsg",params[1]);

	if(privmsg_settings & URL_SQL && (strstr(params[1],"http://") || strstr(params[1],"www.") ))
	{
		sql_addurl(origin,"privmsg",params[1]);
	}	

	ircCommands(session,origin,params,privmsg_settings);
}

void event_channel (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	//printf("'%s' said in channel %s: %s\n",origin ? origin : "someone",params[0],params[1]);

	int tmpsettings = getChannelSettings(params[0]);

	if(tmpsettings & LOG_TXT)
		log_file(origin,params[0],params[1]);

	if(tmpsettings & URL_SQL && (strstr(params[1],"http://") || strstr(params[1],"www.") ))
	{
		sql_addurl(origin,params[0],params[1]);
	}
	
	if(tmpsettings & URL_SQL && strstr(params[1],"!geturls"))
		sql_geturl(session,origin,5);

		
	ircCommands(session,origin,params,tmpsettings);
}


int main(int argc, char** argv)
{
	irc_callbacks_t callbacks;
	irc_session_t *s;
	
	//Time init
	
	time ( &rawtime );
	ptm = gmtime ( &rawtime );


	printf(" : - Bot wird gestartet...\n");

	//-- SQLite Init

	printf(" : - SQLite wird initialisiert...\n");

	sqlite3_open(SQLITEFILE, &sqlitedb);
	sql_createtables();
	
	

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
		printf (" : - SQLite wird geschlossen. \n");
		sqlite3_close(sqlitedb);
		printf (" : - Bot wurde beendet. \n");
		return 1;
	}

	sqlite3_close(sqlitedb);
	return 1;
}

