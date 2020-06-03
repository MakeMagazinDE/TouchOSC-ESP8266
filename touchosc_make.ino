/*
	#############################################################################
	___  ___  ___   _   __ _____ 
	|  \/  | / _ \ | | / /|  ___|
	| .  . |/ /_\ \| |/ / | |__  
	| |\/| ||  _  ||    \ |  __| 
	| |  | || | | || |\  \| |___ 
	\_|  |_/\_| |_/\_| \_/\____/                         
                             
	#############################################################################

	Wireless TouchOSC/Serial bridge using UDP on ESP8266
	Creates Access Point on 192.168.4.1 or connects to WIFI station
	Interpreter for OSC messages intended for HX3.5
	Carsten Meyer & KeyboardPartner 02/2020

	Flash size: 4MByte (FS 2 MByte, OTA 1019 KB)
	
*/
// D E B U G - #define in hx3_utils.h!

// TouchOSC sendet beim Einschalten und alle 45 Sekunden ein UDP-Ping:
//  /  p  i  n  g  #  #  #  ,  #  #  #  
//  2F 70 69 6E 67 00 00 00 2C 00 00 00 

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>	// haut bei Android nicht hin
#include <ESP8266WebServer.h>
#include <FS.h>   			// Include the SPIFFS library
#include <Wire.h>

#include <Adafruit_NeoPixel.h>

#include "webpages.h"
#include "osc_utils.h"


// TouchOSC-Label-Nr. zum Anzeigen der Device-ID, hier "/label/123"
#define LABEL_ID 1023
#define LED1_ID 1020
#define LED2_ID 1021
#define MULTIFADER_ID 1004
#define MULTIFADER_LABEL_ID 1024
#define DIG_INPUT_INDICATOR_ID 1010
#define MULTIBUTTON_ID 1006

unsigned int localPort = 8000; // local port to listen for UDP packets

//#define KNIWWELINO

#ifdef KNIWWELINO
	// Neopixel-LED als Indikator
	#define RGB_PIN 			15
	// definitions for the HT16K33 LED Matrix Driver (Kniwwelino)
	#define HT16K33_ADDRESS         0x70
	#define HT16K33_BLINK_CMD       0x80
	#define HT16K33_CMD_BRIGHTNESS  0xE0
	#define HT16K33_DISP_REGISTER   0x00
	#define HT16K33_KEYS_REGISTER   0x40
	#define HT16K33_KEYINT_REGISTER 0x60
	#define HT16K33_BLINK_DISPLAYON 0x01
	#define MATRIX_STATIC 			0
	#define MATRIX_MIN_BRIGHTNESS	0
	#define MATRIX_MAX_BRIGHTNESS	15
	#define MATRIX_DEFAULT_BRIGHTNESS 10
#endif

#define D0 16
#define D5 14
#define D6 12
#define D7 13


MDNSResponder mdns;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

EEPromData eePromData;

IPAddress CurrentClientIP(0,0,0,0);	// Default, kann sich nach Empfang ändern
const IPAddress c_UnsetIP(0,0,0,0);
const int c_clients_max = 4;

IPAddress ClientIPs[c_clients_max];
int ClientIPs_timer[c_clients_max];
int ClientIPs_idx = 0;

char UDP_rcv_buffer[256]; 			// hier kommen UDP-Daten an
char UDP_send_buffer[256];			// zu sendende Daten


// int OSC_values_idx = 0;	// Zähler für Value-Tabellen, +1000 = param
int ValuesIndex = 0;		// Round-Robin-Zähler für Value-Tabellen, +1000 = param
int ResendIndex = 0;		// Round-Robin-Zähler für Resent-Tabelle, +1000 = param

int LED_toggle = 0;			// wechselt alle 333ms
unsigned long last_millis_led_toggle, last_millis_led_gn_timeout, last_millis_led_or_on;	// Für LED-Blink gebraucht
String ssid_list;

int PageParamStart = 0;
int PagePresetStart = 0;
int CurrentPage = 0;
int random_walk = 64;
int random_dest = 64;

#ifdef KNIWWELINO
	Adafruit_NeoPixel pixels(1, RGB_PIN, NEO_GRB + NEO_KHZ800);
#endif

int MatrixBuffer[8];
int MatrixUsed = 0;
int NeoPixel_red = 0;
int NeoPixel_green = 0;
int NeoPixel_blue = 0;

boolean ButtonA, ButtonAclicked;
boolean ButtonA_old = false;
boolean ButtonB, ButtonBclicked;
boolean ButtonB_old = false;
int BTN_toggle = 0;
int RandomIntegrator[4];
int RandomDestinations[4];

// Control-Typ (enum) für jeden Parameter,
// 0 = t_none, 1 = t_page, 2 = t_label, 3 = t_led, 4 = t_toggle, 5 = t_push, 6 = t_fader, 
// 7 = t_rotary, 8 = t_multitoggle, 9 = t_multipush, 10 = t_multifader, 11 = t_xypad
byte ControlTypes[700];	
char ControlNames[12][16] = {
	"/none", "/page", "/label", "/led", "/toggle", "/push", "/fader", 
	"/rotary", "/multitoggle", "/multipush", "/multifader", "/xypad" };
enum { t_none, t_page, t_label, t_led, t_toggle, t_push, t_fader, 
	t_rotary, t_multitoggle, t_multipush, t_multifader, t_xypad };

// #############################################################################
// ###                            SETUP & UTILS                              ###
// #############################################################################

void init_eeprom_ap() {
	strcpy(eePromData.ssid, "Make TouchOSC Bridge");
	strcpy(eePromData.password, "password");
	eePromData.udp_fb_others = 1;	// default
	eePromData.udp_fb_self = 1;
	eePromData.udp_delay = 2;	    
	eePromData.udp_timeout = 300;	// 5 Minuten
	eePromData.ap_mode = 1;			// ESP8266 ist selbst Access Point    
}

void setup() {
	int i, temp_ap_mode, len;
	char line_buf[100];
	int idx;
	String temp;
#ifdef KNIWWELINO
	pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
	pixels.clear(); // Set all pixel colors to 'off'
#endif
	Serial.begin(115200);
	pinMode(D0, OUTPUT);
	pinMode(D5, INPUT_PULLUP);
	pinMode(D6, INPUT_PULLUP);
	pinMode(D7, INPUT_PULLUP);
	// Serial1.begin(230400);
	MON_MSGLN("");
	MON_MSGLN("Starting ESP8266...");
	delay(100);
	while (Serial.available()>0)
		Serial.read();
	Serial.setTimeout(50);

	MON_MSGLN("Make TouchOSC Bridge " + webpages_version_str);
	MON_MSGLN(webpages_copyright_str);

	SPIFFS.begin();		// Start the SPI Flash File System
	// Datei mit Control-Zuordnungen einlesen
	for (i=0; i<sizeof(ControlTypes); i++) ControlTypes[i] = t_none;
	if (SPIFFS.exists("control_types.ini")) {				// Datei vorhanden?
		File file = SPIFFS.open("control_types.ini", "r");	// zum Lesen öffnen
		while (file.available()) {
			len = file.readBytesUntil('\n', line_buf, sizeof(line_buf));
			line_buf[len-1] = 0;
			temp = extract_value(line_buf, ';', 0);
			idx = temp.toInt();
			temp = extract_value(line_buf, ';', 1);
			if (val_in_range(idx, 1000, 1699)) {
				ControlTypes[idx % 1000] = temp.toInt();	// modulo 1000 als Index
			}
		} 
    	file.close();
	}

#ifdef KNIWWELINO
	// Matrix-LEDs initialisieren
	Wire.begin();
	Wire.beginTransmission(HT16K33_ADDRESS);
	Wire.write(0x21);  // turn on oscillator
	Wire.endTransmission();
	Wire.beginTransmission(HT16K33_ADDRESS);
	Wire.write(HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON);
	Wire.endTransmission();
	Wire.beginTransmission(HT16K33_ADDRESS);
	Wire.write(HT16K33_CMD_BRIGHTNESS | MATRIX_MAX_BRIGHTNESS);
	Wire.endTransmission();
	drawPixel(0,0,0,0);	// Buffer schreiben
#endif

	// Read EEPROM
	EEPROM.begin(sizeof(EEPromData));
	EEPROM.get(0,eePromData);
	EEPROM.end();
	ssid_list = "";
	int ssids_found = WiFi.scanNetworks();
	if (ssids_found) {
		MON_MSGLN("Found SSIDs: ");
		for(i=0; i<ssids_found; i++){
			temp = WiFi.SSID(i);
			MON_MSGLN(temp);
			if (i) ssid_list += ",\r\n";	// mehr als eine Zeile?
			else ssid_list += "\r\n";
			ssid_list += "\"" + temp + "\"";
#ifdef KNIWWELINO
			pixels.setPixelColor(0, pixels.Color(64, 0, 64));
			pixels.show();   // Send the updated pixel colors to the hardware.
			delay(150);
			pixels.setPixelColor(0, pixels.Color(0, 0, 0));
			pixels.show();
			delay(150);
#endif
		}
	}

    if (eePromData.flag != 0x55) {
		init_eeprom_ap();
	    strcpy(eePromData.ssid_station, "Your Network");	// "Your Network"
	    strcpy(eePromData.password_station, "password");	// "password"
	    eePromData.flag = 0x55;			// initialized
	    write_eeprom();
		MON_MSGLN("WiFi name reset to SSID 'Make TouchOSC Bridge' and PW 'password'");
		SPIFFS.format();
 		MON_MSGLN("SPIFFS formatted, upload HTML/CSS files!");
   }
	
	temp_ap_mode= eePromData.ap_mode;
	if (temp_ap_mode) {
		// ESP8266 ist selbst Access Point
		MON_MSGLN("WiFi Acces Point Mode, IP 192.168.4.1");
		WiFi.mode(WIFI_AP);			// Only Access point
		WiFi.softAP(eePromData.ssid, eePromData.password);	// access point
		NeoPixel_red = 64;
	} else {		
	    // Externen Access Point bzw. Router verwenden
		MON_MSGLN("WiFi Station Mode ");
	    WiFi.mode(WIFI_STA);
	    WiFi.begin(eePromData.ssid_station, eePromData.password_station);
	    i = 0;
		LED_toggle = 0;
		MON_MSG("Connecting ...");
		while (WiFi.status() != WL_CONNECTED) {
		// 10 Sekunden Warten auf Verbindung
			delay(333);
			COM_MSG(".");
			LED_toggle ^= 64;	// invertieren
			NeoPixel_blue = LED_toggle;
			setNeoPixel();	// Neopixel aktualisieren
			i++;
			if (i > 30) {
				temp_ap_mode = 1;
				break;
			}
		}
		NeoPixel_blue = 0;
		MON_MSGLN("");
		if (temp_ap_mode) {
			MON_MSGLN("Station mode failed, will setup Acces Point Mode, IP 192.168.4.1");
			WiFi.mode(WIFI_AP);			// Only Access point
			WiFi.softAP(eePromData.ssid, eePromData.password);	// access point
			NeoPixel_red = 64;
		} else {
			MON_MSG("Connected with IP ");
			MON_MSGLN(WiFi.localIP());
			NeoPixel_green = 64;
		}
	}
	setNeoPixel();	// Neopixel aktualisieren

	//Start UDP
	udp.begin(localPort);
	MON_MSG("UDP started, local port: ");
	MON_MSGLN(udp.localPort());
	
	initWebserver();
	
	MON_MSGLN("HTTP started");
	
	if (mdns.begin("makeosc", WiFi.localIP())) {
 		mdns.addService("http", "tcp", 80);
		MON_MSGLN("mDNS started, URL 'makeosc.local'");
	}
    
	for(i=0; i<5; i++) MatrixBuffer[i] = 0;
	for(i=0; i<4; i++) RandomIntegrator[i] = 0;
    for(i=0; i<c_clients_max; i++) {
		ClientIPs[i] = c_UnsetIP;
		ClientIPs_timer[i] = eePromData.udp_timeout;
	} 

	delay(1000);
	NeoPixel_red = 0;
	NeoPixel_green = 0;
	NeoPixel_blue = 0;
	setNeoPixel();	// Neopixel ausschalten
	MON_MSGLN("Ready.");
    last_millis_1s = millis();
    last_millis_10ms = last_millis_1s;
    last_millis_led_toggle = last_millis_1s;
	last_millis_led_gn_timeout = last_millis_1s;
	last_millis_led_or_on = last_millis_1s;
}

void drawPixel(int x, int y, int on, int rotation) {
#ifdef KNIWWELINO
	if ((y < 0) || (y >= 5)) return;
		if ((x < 0) || (x >= 5)) return;
		// kniwwelino hardware specific: mirror cols
		int x1 = 4 - x;
		int y1 = y;

		if (rotation == 0) {
			x = y1;
			y = x1;
		} else if (rotation == 1) {
			x = 4-x1;
			y = y1;
		} else if (rotation == 2) {
			x = 4-y1;
			y = 4-x1;
		} else if (rotation == 3) {
			x = x1;
			y = 4-y1;
		}

		if (on) {
			MatrixBuffer[y] |= 1 << x;
		} else {
			MatrixBuffer[y] &= ~(1 << x);
		}

    	Wire.beginTransmission(HT16K33_ADDRESS);
    	Wire.write(HT16K33_DISP_REGISTER); // start at address $00
    	for (uint8_t i = 0; i < 8; i++) {
    	  Wire.write(MatrixBuffer[i] & 0xFF);
    	  Wire.write(MatrixBuffer[i] >> 8);
    	}
    	Wire.endTransmission();
#endif
}

void setNeoPixel() {
#ifdef KNIWWELINO
	pixels.setPixelColor(0, pixels.Color(NeoPixel_red, NeoPixel_green, NeoPixel_blue));
	pixels.show();
#endif
}

// #############################################################################
// ###                         UDP BUFFER UTILS                              ###
// #############################################################################

float udprcvbuf_extract_float(int idx) {
	// holt Float-Bytes aus UDP_rcv_buffer und wandelt sie in Integer
	union { byte b[4]; float f; } u;	
	int n;
	// Reihenfolge umkehren
	for ( n=0 ; n<4 ; n++ ) u.b[n] = UDP_rcv_buffer[idx + 3 - n];
	return(u.f);	// in Integer 0..127 wandeln
}

int udprcvbuf_extract_int(int idx) {
	// holt Int-Bytes aus UDP_rcv_buffer und wandelt sie in Integer
	union { byte b[4]; int32 i; } u;	
	int n;
	// Reihenfolge umkehren
	for ( n=0 ; n<4 ; n++ ) u.b[n] = UDP_rcv_buffer[idx + 3 - n];
	return(u.i);	// in Integer 0..127 wandeln
}


void int_to_udpsendbuf(int *idx, int val) {
	// zum Umsetzen von Int32 in Bytes, schreibt in UDP_send_buffer
	union { byte b[4]; int32 i32; } u;	
	int i;	
	u.i32 = val;
	// Reihenfolge umdrehen
	for ( i=0 ; i<4 ; i++ ) UDP_send_buffer[*idx + 3 - i] = u.b[i];
	*idx +=4;
}

void float_to_udpsendbuf(int *idx, float val) {
	// zum Umsetzen von Float in Bytes, schreibt in UDP_send_buffer
	union { byte b[4]; float f32; } u;	
	int i;	
	u.f32 = val;
	// Reihenfolge umdrehen
	for ( i=0 ; i<4 ; i++ ) UDP_send_buffer[*idx + 3 - i] = u.b[i];
	*idx +=4;
}

void format_to_udpsendbuf(int *idx, int count, char format_char) {
	// schreibt Format-Info in UDP_send_buffer
	if (count > 0) {
		UDP_send_buffer[*idx] = ',';
		UDP_send_buffer[*idx + 1] = format_char;
		if (count > 1)
			UDP_send_buffer[*idx + 2] = format_char;
		UDP_send_buffer[*idx + 3] = 0;
		*idx +=4;
	}
}

void clear_udpsendbuf() {
	int i;
	for (i = 0; i < 128; i++) 
		UDP_send_buffer[i] = 0;
}

#ifdef DEBUG_HEXMSG
void serhex_udpbuf(int valid, int buf_end) {
	int i;
	if (valid) {
	    COM_MSG("cmd: ");
	    COM_MSGLN(UDP_send_buffer);
	    COM_MSG("HEX: ");
	    for (i = 0; i <= buf_end; i++) {
			COM_MSG(UDP_send_buffer[i], HEX);
		    COM_MSG(" ");
		}
		COM_MSGLN("");
	} else
		COM_MSGLN("Cmd: none");
}
#endif


// #############################################################################
// ###                                                                       ###
// ###                            U D P  PARSER                              ###
// ###                    eingehende Pakete von TouchOSC                     ###
// ###                                                                       ###
// #############################################################################

void parse_udp(int packet_size) {
	// liefert Anzahl der gültigen Bytes in UDP_rcv_buffer
	// und in param_ret, val_ret die gefundenen Parameter		
	int i, j, idx, send_udp, udp_buf_len;
	// String	controlstr;	// wird hier nicht gebraucht
	String	tempstr;

	int param = 0;
	int idx_1 = 0;
	int idx_2 = 0;
	float val_1 = 0;
	float val_2 = 0;
	int arg_count = 0;
	int arg_count_f = 0;
	int arg_count_i = 0;
	int val_1_i = 0;
	int val_2_i = 0;

	// Position der Argumente, Komma suchen
	for (i = 0; (i <= packet_size) && (UDP_rcv_buffer[i] != 0x2C); i++);	// ","
	idx = i;
	
	if (UDP_rcv_buffer[idx + 1] == 'f') arg_count_f++;
	if (UDP_rcv_buffer[idx + 1] == 'i') arg_count_i++;
	if (UDP_rcv_buffer[idx + 2] == 'f') arg_count_f++;	// "ff"
	if (UDP_rcv_buffer[idx + 2] == 'i') arg_count_i++;	// "ii"
	arg_count = arg_count_f + arg_count_i;
	
	if (arg_count) {
		// erster Teil des Strings gibt Typ des Controls an, z.B. "fader" bei "/fader/1000/9"
		// controlstr = extract_value(UDP_rcv_buffer,'/', 1);	// hier nicht gebraucht
		// Parameter-Nummer	
		tempstr = extract_value(UDP_rcv_buffer,'/', 2);	
		param = tempstr.toInt();
		COM_MSG("Param: ");	
		COM_MSG(param);	
		// erster Index bei Multi-Fadern und Y-Position bei Multi-Buttons, sonst 0		
		tempstr = extract_value(UDP_rcv_buffer,'/', 3);	// evt. nicht vorhanden, dann 0	
		idx_1 = tempstr.toInt();
		if (idx_1){
			COM_MSG("  Idx_1: ");	
			COM_MSG(idx_1);	
		}
		// X-Position bei Multi-Buttons, somst 0	
		tempstr = extract_value(UDP_rcv_buffer,'/', 4);	// evt. nicht vorhanden, dann 0	
		idx_2 = tempstr.toInt();
		if (idx_2){
			COM_MSG("  Idx_2: ");	
			COM_MSG(idx_2);	
		}
		idx +=4;
		if (arg_count_f)
			val_1 = udprcvbuf_extract_float(idx); // Reihenfolge Bytes umkehren
		else
			val_1 = (float)udprcvbuf_extract_int(idx);

	    COM_MSG("  Val_1: ");	
		COM_MSG(val_1);	
		if (arg_count_f == 2){
			// Zweiter Wert X (!), auf Anfang 2. Float
			idx +=4;
			val_2 = udprcvbuf_extract_float(idx); // Reihenfolge Bytes umkehren
		    COM_MSG("  Val_2: ");	
		    COM_MSG(val_2);	

		}	
		if (arg_count_i == 2){
			// Zweiter Wert, auf Anfang 2. Float
			idx +=4;
			val_2 = (float)udprcvbuf_extract_int(idx);
		    COM_MSG(", ");	
		    COM_MSG(val_2);	

		}	
	}
    COM_MSGLN("");	

	val_1_i = (int)val_1;
	val_2_i = (int)val_2;
	
	// Auswertung der empfangenen Daten nach Parameternummer
	switch (param) {
		case 100:	// Fader 1
			NeoPixel_red = val_1_i;
			setNeoPixel();
			break;
		case 101:	// Fader 2
			NeoPixel_green= val_1_i;
			setNeoPixel();
			break;
		case 102:	// Fader 3
			NeoPixel_blue = val_1_i;
			setNeoPixel();
			break;
		case 105:	// Pushbutton, Reset Matrix
			for(i=0; i<5; i++) MatrixBuffer[i] = 0;
			drawPixel(0, 0, 0, 0);	// nur Update
			// Button Row 1..5, Column 1..5 addressieren und löschen
			for (i=1; i<6; i++) // 5 Zeilen, 5 Spalten
				for (j=1; j<6; j++) {
					udp_buf_len = setup_udp_send_buffer(t_multitoggle, MULTIBUTTON_ID, i, j, 0, -1); 
					send_udp_buffer(udp_buf_len);
				}
			udp_buf_len = setup_udp_send_buffer(t_led, LED2_ID, -1, -1, 0, -1);	// LED OFF
			send_udp_buffer(udp_buf_len);
			MatrixUsed = 0;
			break;
		case 106:	// Multibutton
			drawPixel(idx_2-1, 5-idx_1, val_1_i, 0);
			// LED ON wenn erstmals benutzt
			if (!MatrixUsed) {
				udp_buf_len = setup_udp_send_buffer(t_led, LED2_ID, -1, -1, 127, -1);	// LED ON
				send_udp_buffer(udp_buf_len);
			}
			MatrixUsed = 1;
			break;
		case 107:	// XY-Pad
			for (i=0; i<5; i++) MatrixBuffer[i] = 0;
			drawPixel(val_2_i/26, 4-val_1_i/26, 1, 0);
			// LED ON wenn erstmals benutzt
			if (!MatrixUsed) {
				udp_buf_len = setup_udp_send_buffer(t_led, LED2_ID, -1, -1, 127, -1);	// LED ON
				send_udp_buffer(udp_buf_len);
			}
			MatrixUsed = 1;
			break;
		case 108:	// Toggle Button an Ausgang D0 (16)
			digitalWrite(D0, val_1_i);
			break;
	}
}


// #############################################################################
// ###                         UDP BUFFER SETUP                              ###
// #############################################################################

#ifdef DEBUG_HEXMSG
void serhex_udpbuf(int buf_end) {
	int i;
	COM_MSG("Cmd: ");
	COM_MSGLN(UDP_send_buffer);
	COM_MSG("HEX: ");
	for (i = 0; i <= buf_end; i++) {
		COM_MSG(UDP_send_buffer[i], HEX);
		COM_MSG(" ");
	}
	COM_MSGLN("");
}
#endif

int setup_udp_send_buffer(int type, int param, int idx_1, int idx_2, int val_1, int val_2) {
	// Buffer für UDP Send aufbereiten, liefert endgültige Länge des UDP-Buffers
	String sendstr = ControlNames[type];
	int buf_len = 0;
	int val_count = 1;
	clear_udpsendbuf();
	if (val_2 >= 0) 
		val_count++;
	if (param >= 0) 
		sendstr += "/" + String(param);
	// Indexe bei Multibuttons und Multifadern 
	if (idx_1 >= 1) 
		sendstr += "/" + String(idx_1);
	if (idx_2 >= 1) 
		sendstr += "/" + String(idx_2);
	COM_MSG("WIF->UDP: " + sendstr);
	
	buf_len = sendstr.length();
	sendstr.toCharArray(UDP_send_buffer, buf_len + 1);
	// auf Long-Grenze bringen
	buf_len = buf_len + 4 - (buf_len % 4);
	// String ist im Buffer, nun ",i" und Integer einbauen
	format_to_udpsendbuf(&buf_len, val_count, 'i'); // 2 Werte wenn XY
	int_to_udpsendbuf(&buf_len, val_1); // umgek. Reihenfolge in Send-Buffer
	if (val_2 >= 0) {
		int_to_udpsendbuf(&buf_len, val_2); // nur wenn val_2 >= 0
		COM_MSGLN(" = " + String(val_1) + ", " + String(val_2));
	} else
		COM_MSGLN(" = " + String(val_1));	
#ifdef DEBUG_HEXMSG
	serhex_udpbuf(buf_len);
#endif
	return(buf_len);
}

int setup_udp_send_buffer_text(int type, int param, String textstr) {
	// UDP_send_buffer nach Text-Befehl in HX3_text_buffer vorbereiten
	String sendstr = ControlNames[type];
	int i;
	int buf_len = 0;
	clear_udpsendbuf();
	if (param >= 0) 
		sendstr += "/" + String(param);
	COM_MSG("WIF->UDP: " + sendstr);	
	COM_MSGLN(" = " + textstr);	
	buf_len = sendstr.length();
	sendstr.toCharArray(UDP_send_buffer, buf_len + 1);
	// auf Long-Grenze bringen
	buf_len = buf_len + 4 - (buf_len % 4);
	format_to_udpsendbuf(&buf_len, 1, 's'); // 1 String
	// String in den Send-Buffer
	for (i=0; i<textstr.length(); i++) {
		UDP_send_buffer[buf_len] = textstr.charAt(i);
		buf_len++;
	}
	// auf Long-Grenze bringen
	buf_len = buf_len + 4 - (buf_len % 4);
#ifdef DEBUG_HEXMSG
	serhex_udpbuf(buf_len);
#endif
	return(buf_len);
}

// -----------------------------------------------------------------------------


void check_for_new_client(IPAddress udp_client_ip) {
	// setzt ClientIPs_idx neu, wenn neuer Client gefunden		
	int i, param, val_i, udp_buf_len;
	int found = 0;
	boolean send_info = false;
	String sendstr;
	
	// War IP schon einmal verbunden?
	for (i=0; i<c_clients_max; i++)
		if (ClientIPs[i] == udp_client_ip) {
			found = 1;
			if (ClientIPs_timer[i] == 0) {
				MON_MSG("WIF: Known client reconnect,  IP: ");
				MON_MSG(udp_client_ip);
				MON_MSG(", index ");
				MON_MSGLN(i);
				send_info = true;
			}
			ClientIPs_timer[i]= eePromData.udp_timeout;
			break;
		}
	// IP noch nicht bekannt? Dann bis zu 4 Clients neu eintragen		
	if (!found) {
		ClientIPs[ClientIPs_idx] = udp_client_ip;
		ClientIPs_timer[ClientIPs_idx]= eePromData.udp_timeout;
		MON_MSG("WIF: New client, IP: ");
		MON_MSG(udp_client_ip);
		MON_MSG(", index ");
		MON_MSGLN(ClientIPs_idx);
	}
	if (send_info) {
		// an ID-Label schicken, so vorhanden
		sendstr = "Device " + String(ClientIPs_idx + 1);
		send_info = true;
		udp_buf_len = setup_udp_send_buffer_text(t_label, LABEL_ID, sendstr);
		send_udp_buffer(udp_buf_len);
		ClientIPs_idx++;
		if (ClientIPs_idx >= c_clients_max)
			ClientIPs_idx = 0;
	}
}	

// #############################################################################


void resend_udp_buffer_others(int bytes_to_send) {
	// Werte an angemeldete Clients senden, außer an derzeit aktiven Client 
	int i;
	if (eePromData.udp_fb_others) {
		for (i = 0; i<4; i++)
			if (ClientIPs_timer[i] && (ClientIPs[i] != CurrentClientIP)) {
				udp.beginPacket(ClientIPs[i], 9000);
				udp.write(UDP_send_buffer, bytes_to_send); // empfangene Daten zurück an Client 
				udp.endPacket();
			}
		delay(eePromData.udp_delay);
	}
}

void resend_udp_buffer_self(int bytes_to_send) {
	// Werte nur an derzeit aktiven Client senden
	if (eePromData.udp_fb_self) {
		udp.beginPacket(CurrentClientIP, 9000);
		udp.write(UDP_send_buffer, bytes_to_send); // empfangene Daten zurück an Client 
		udp.endPacket();
		delay(eePromData.udp_delay);
	}
}

void send_udp_buffer(int bytes_to_send) {
	// Werte an alle angemeldete Clients senden
	int i;
	for (i=0; i<4; i++)
		if (ClientIPs_timer[i]) {
			udp.beginPacket(ClientIPs[i], 9000);
			udp.write(UDP_send_buffer, bytes_to_send); // Send data to Client 
			udp.endPacket();
		}
		delay(eePromData.udp_delay);
}

// #############################################################################
// ###                       LED- und BUTTON-TOGGLES                         ###
// ###                           SEKUNDEN-TIMER                              ###
// #############################################################################

#define C_1S 1000;
#define C_10MS 10;
#define C_100MS 100;
#define C_LEDBLINK 333;

void chores_and_timeouts() {
	// Timeouts in 10ms und 1000ms
    int udp_buf_len, adc_val;
	int i;
    int current_millis = millis();
	boolean ButtonsPressed;
	String sendstr;

	if (current_millis >= last_millis_led_toggle) {
		// wird ab hier jede 333 ms aufgerufen
		last_millis_led_toggle = current_millis + C_LEDBLINK;
		LED_toggle ^= 127;	// invertieren
		// "LED" blinken lassen
		udp_buf_len = setup_udp_send_buffer(t_led, LED1_ID, -1, -1, LED_toggle, -1);
		send_udp_buffer(udp_buf_len);

		// Zufallswerte für Fader 1..4 und Labels
		for (i=0; i<4; i++) RandomDestinations[i] = random(128);
    }

	if (current_millis >= last_millis_100ms) {
		// wird ab hier jede 100 ms aufgerufen
		last_millis_100ms = current_millis + C_100MS;

		// Zufallswerte an Fader 1..4 und Labels senden, könnten z.B. ADC-Werte sein
		for (i=0; i<4; i++) {
			RandomIntegrator[i] = RandomDestinations[i] + (RandomIntegrator[i]*7)/8;
			adc_val = RandomIntegrator[i]/8;
			udp_buf_len = setup_udp_send_buffer(t_multifader, MULTIFADER_ID, i+1, -1, adc_val, -1); // Fader mit Index i+1 der Gruppe
			send_udp_buffer(udp_buf_len);
			udp_buf_len = setup_udp_send_buffer(t_label, MULTIFADER_LABEL_ID, i+1, -1, adc_val, -1); // Label mit Index i+1 im Namen (!)
			send_udp_buffer(udp_buf_len);
		}

		// Digitale Eingänge an (horizontale Reihe 1) der Multibuttons 1..4 senden
		udp_buf_len = setup_udp_send_buffer(t_multitoggle, DIG_INPUT_INDICATOR_ID, 1, 1, digitalRead(D0), -1); // Button 1.1
		send_udp_buffer(udp_buf_len);
		udp_buf_len = setup_udp_send_buffer(t_multitoggle, DIG_INPUT_INDICATOR_ID, 1, 2, digitalRead(D5), -1); // Button 1.2
		send_udp_buffer(udp_buf_len);
		udp_buf_len = setup_udp_send_buffer(t_multitoggle, DIG_INPUT_INDICATOR_ID, 1, 3, digitalRead(D6), -1); // Button 1.3
		send_udp_buffer(udp_buf_len);
		udp_buf_len = setup_udp_send_buffer(t_multitoggle, DIG_INPUT_INDICATOR_ID, 1, 4, digitalRead(D7), -1); // Button 1.4
		send_udp_buffer(udp_buf_len);

	}

	if (current_millis >= last_millis_10ms) {
		// wird ab hier jede 10 ms aufgerufen
		last_millis_10ms = current_millis + C_10MS;
		if (resend_timer > 0) resend_timer--;	// wenn vorher negativ: abgeschaltet
	}

	// Timeout-Zähler für Clients etc.
	if (current_millis >= last_millis_1s) {
		// wird jede Sekunde aufgerufen
		last_millis_1s = current_millis + C_1S;

		for (i=0; i<c_clients_max; i++) {
			if (ClientIPs_timer[i] > 0) {
				ClientIPs_timer[i]--;
			}	
			if (ClientIPs_timer[i] == 1) {
				MON_MSG("WIF: Client timed out: #");
				MON_MSGLN(i);
			}
		}
		sendstr = "Device " + String(ClientIPs_idx + 1);
		udp_buf_len = setup_udp_send_buffer_text(t_label, LABEL_ID, sendstr);
		send_udp_buffer(udp_buf_len);
		// Aktive Zeit des Webservers
		if (ap_timeout) ap_timeout--;
	}	
}

// #############################################################################
// ###                                                                       ###
// ###                          M A I N   L O O P                            ###
// ###                                                                       ###
// #############################################################################

// Nach seriell empfangenem Datensatz (Textzeile oder ESC-Binärformat) wird
// setup_udp_send_buffer() bzw. setup_udp_send_buffer_text() aufgerufen
// und ein Buffer für den UDP-Versand an TouchOSC zusammengestellt.

// Empfangene UDP-Pakete werden nach Parametern und Werten durchsucht.
// Gültige OSC-Pakete gehen als Textbefehl oder im Binärformat an HX3.5 zurück,
// je nachdem, ob zuletzt ein Text- oder Binärbefehl empfangen wurde.

void loop() {
	int i, val_1, val_2;
	int packet_size;
	int udp_buf_len;
	
	// UDP-Paket empfangen?
	packet_size = udp.parsePacket();
	if (packet_size) {
		// UDP Paket empfangen, vorrangig behandeln
		udp.read(UDP_rcv_buffer, packet_size); 	// read the packet into the buffer		
		// Sendet UDP-Parameter an Serielle, erhält ggf. param und val zurück
		parse_udp(packet_size);

		CurrentClientIP = udp.remoteIP(); // IP der aktuellen Verbindung
		check_for_new_client(CurrentClientIP);		
		resend_timer = RESEND_TIMEOUT;
	} else {
		handleWebserverRequests();
		mdns.update();	// regelmäßig aufrufen!
		chores_and_timeouts(); // Timouts etc.
	}	
}
