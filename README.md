######################################
#
### IRC BOT von Ralf Moter README
#
######################################


Inhaltsverzeichnis

1. Installation
2. Bot starten
3. Configfiles
4. Chatbefehle
5. Chat Settings

1. Installation

Für das Programm wird sqlite v3 auf dem betriebsystem benötigt.
Wenn dies Installiert ist. Einfach mit

	./make install

den IRCBOT installieren.

2. Bot starten

Den Bot kann man mit 2 Möglichkeiten starten.

	./ircbot server nick #channel

oder per Configfile (ziehe Abschnitt 3.)

	./ircbot configfile

3. Configfiles

Ein Configfile muss folgt aussehen.
(Achtung: Alles in einer Zeile)

	ircserver;Botname;#channel;

ziehe beispiel Configfiles.

4. Chatbefehle

Der Bot Reagiert auf folgene Befehle im MasterChannel(ist der erste 
Channel der angegeben wurde).

- !quit  			= Der Bot beendet sicht
- !nick <neur Name>		= Der Bot ändert seinen Namen.
- !join #channel		= Der Bot join einen gewissen Channel.
- !part				= Der Bot geht aus dem jeweiligen Channel 
				  wo es gesagt wird. Aber nicht aus dem 
				  MasterChannel.
- !topic <neues Topic> 		= Der Bot ändert das Topic.
- !time				= Gibt die Aktuelle Serveruhrzeit.
- !set #channel settings	= Setz im jeweiligen Channel die Settings fest.
- !status			= Gibt die jeweiligen Channelsettings aus.
- !geturls			= Gibt die letzten 5 URLs aus die der Bot gesehen hat.

Der Bot reagiert auserdem noch auf seinen Namen und das dient als Ping 
Funktion ob der Bot Funktioniert.

5. Chat Settings

Der Bot verfügt über ein Channel Setting System anhand einer Bitmaske.

Hier die einzelen Optionen

Topic ändern 		1
Nick ändern		2
Joinen/Leaven		4
Bot Beenden		8
Zeit ausgeben		16
In Textfile Loggen	32
URLs Crawlen		64

Also wenn der Bot in einem Channel nur das Topic ändern darf 
lautet der Befehl im Channel	

	!set #Channel 1	

oder wenn der Bot in einem Channel beendet werden darf und URLs Crawlen soll, 
lautet der Befehlt so:

	!set #Channel 72

(die 72 entsteht daraus 8 + 64 = 72)



