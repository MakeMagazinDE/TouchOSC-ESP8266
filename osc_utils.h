/*
	#############################################################################
	___  ___  ___   _   __ _____ 
	|  \/  | / _ \ | | / /|  ___|
	| .  . |/ /_\ \| |/ / | |__  
	| |\/| ||  _  ||    \ |  __| 
	| |  | || | | || |\  \| |___ 
	\_|  |_/\_| |_/\_| \_/\____/                         
                             
	#############################################################################

	Hilfsfunktionen, auch in main gebraucht

*/

// Betrieb ohne angeschlossenen HX3:
#define DEBUG


 // Startup-/Connect-Informationen:
#define DEBUG_MON

#ifdef DEBUG
    // Kommunikation:
    #define DEBUG_COM
    // Webseiten-Anfragen:
    #define DEBUG_WEB
    // Hexdump vom UDP Send Buffer:
    // #define DEBUG_HEXMSG	
#endif

#ifdef DEBUG_MON
    #define MON_MSGLN(...) Serial.println( __VA_ARGS__ )
    #define MON_MSG(...) Serial.print( __VA_ARGS__ )
 #else
    #define MON_MSGLN(...)
    #define MON_MSG(...)
 #endif

#ifdef DEBUG_COM
    #define COM_MSGLN(...) Serial.println( __VA_ARGS__ )
    #define COM_MSG(...) Serial.print( __VA_ARGS__ )
  #else
    #define COM_MSGLN(...)
    #define COM_MSG(...)
#endif

#ifdef DEBUG_WEB
    #define WEB_MSGLN(...) Serial.println( __VA_ARGS__ )
    #define WEB_MSG(...) Serial.print( __VA_ARGS__ )
  #else
    #define WEB_MSGLN(...)
    #define WEB_MSG(...)
 #endif


// 200ms nach Loslassen bis zum Resend der Daten (10ms-Tick)
#define RESEND_TIMEOUT 20

#ifndef OSC_UTILS_H
#define OSC_UTILS_H

#define JSON_BUFLEN 15000

const int c_webpage_timeout = 9999;

struct EEPromData
{
    // here you could add additional configuration settings for your application
    // if you do so, you also need to edit ConfigService::ConnectionHandler::run()
    char password[50];
    char ssid[50];
    int udp_delay;		// Delay nach seriellem Empfang
    int udp_timeout;  	// Sekunden nach 1. Anmeldung	
    int udp_fb_others;	// Feedback an andere Clients
    int udp_fb_self;	// Feedback an sendenden Client
	int ap_mode;
	IPAddress station_ip;	// Dummy!
    char password_station[50];
    char ssid_station[50];
    char flag;
};

enum { not_complete, complete_binary_single, complete_binary_bulk, complete_text};

enum {none, fader, button, multifader, rotaryswitch, multibutton_hor, multibutton_vert, xypad, page, led, progress};

 // Timeouts und Timer 
extern int ap_timeout;			        // 5 Minuten Konfigurationszeit
extern int resend_timer;	            // wird in 10ms-Tick dekrementiert	
extern int packet_sent_counter;	        // Zählt gesendete UDP-Pakete, für Debug-Zwecke
extern int ClientIPs_timer[];           // in main, Timeouts für UDP-Clients
extern unsigned long last_millis_1s, last_millis_100ms, last_millis_10ms;	// Für Timer-Ticks gebraucht


int val_in_range(int value, int min_val, int max_val); // Wert zwischen min und max?


String extract_value(String data, char separator, int index);
	// Werte-Strings extrahieren, Trennzeichen in "separator"
	// OSC-Message mit Trenner "/": /<1>/<2>/<3>/..., da erstes Trennzeichen am Anfang
	// Text-Message mit Trenner "=": <0>=<1>


void write_eeprom();

#endif /* HX3_UTILS_H */
