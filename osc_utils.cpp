/*
	#############################################################################
	___  ___  ___   _   __ _____ 
	|  \/  | / _ \ | | / /|  ___|
	| .  . |/ /_\ \| |/ / | |__  
	| |\/| ||  _  ||    \ |  __| 
	| |  | || | | || |\  \| |___ 
	\_|  |_/\_| |_/\_| \_/\____/                         
                             
	#############################################################################

	Hilfsfunktionen, auch in main() gebraucht

*/

// #############################################################################
// ###                          H X 3   U T I L S                            ###
// #############################################################################

#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "osc_utils.h"


extern EEPromData eePromData;

int ap_timeout = c_webpage_timeout;	// x Minuten Konfigurationszeit
const int resend_timeout = RESEND_TIMEOUT;	    // 500 ms
int resend_timer = RESEND_TIMEOUT;	// wird in 10ms-Tick dekrementiert	
int packet_sent_counter = 0;	    // Zählt gesendete UDP-Pakete, für Debug-Zwecke

unsigned long last_millis_1s, last_millis_100ms, last_millis_10ms;	// Für Timer-Ticks gebraucht

const int c_clients_max = 4;

// #############################################################################

// -----------------------------------------------------------------------------
void write_eeprom() {
    EEPROM.begin(sizeof(EEPromData));
    EEPROM.put(0,eePromData);
	EEPROM.commit();    // Only needed for ESP8266 to get data written
    EEPROM.end();
	delay(200);
}

// -----------------------------------------------------------------------------


int val_in_range(int value, int min_val, int max_val) {
	return (value >= min_val) && (value <= max_val);
}

// -----------------------------------------------------------------------------

String extract_value(String data, char separator, int index) {
	// Werte-Strings extrahieren, Trennzeichen in "separator"
	// OSC-Message mit Trenner "/": /<1>/<2>/<3>/..., da erstes Trennzeichen am Anfang
	// Text-Message mit Trenner "=": <0>=<1>
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length()-1;
    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || data.charAt(i) == 0 || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
} 

