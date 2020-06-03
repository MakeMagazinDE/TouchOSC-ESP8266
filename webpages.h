/*
	#############################################################################
       __ ________  _____  ____  ___   ___  ___
      / //_/ __/\ \/ / _ )/ __ \/ _ | / _ \/ _ \
     / ,< / _/   \  / _  / /_/ / __ |/ , _/ // /
    /_/|_/___/_  /_/____/\____/_/_|_/_/|_/____/
      / _ \/ _ | / _ \/_  __/ |/ / __/ _ \
     / ___/ __ |/ , _/ / / /    / _// , _/
    /_/  /_/ |_/_/|_| /_/ /_/|_/___/_/|_|

	#############################################################################


	Wireless TouchOSC/Serial bridge using UDP on ESP8266
	Creates Access Point on 192.168.4.1
	Interpreter for OSC messages intended for HX3.5
	Carsten Meyer & KeyboardPartner 02/2020
	
	Web Server und Hilfsfunktionen, auch in main gebraucht

	Configuration service on http://192.168.4.1

*/

#ifndef WEBPAGES_H
#define WEBPAGES_H

const String webpages_version_str = "Ver #1.05";
const String osc_version_str = "/label_wifi_version=\"WiFi Interface Version 1.05\"";
const String webpages_copyright_str = "(c) Keyboardpartner & Carsten Meyer 02/2020";

const String webpages_header = "<!DOCTYPE html><html lang='de'>"
	"<meta charset='UTF-8'>"
	"<meta name='viewport' content='width=device-width, initial-scale=1'>"
	"<link rel='stylesheet' href='style.css'>"
	"<title>HX3.5 WiFi Configuration</title></head>\r\n"
	"<body>\r\n"
	"<table class='header'><tr><td width = '200px'><img src='kplogo_kl.gif' alt='kbp logo'></td><td><h1> HX3.5 WiFi<br>Configuration</h1></td></tr></table>\r\n";
	 
const String webpages_footer = "</p><li>HX3.5 Server " + webpages_version_str + "<br>" + webpages_copyright_str + "</li></body></html>\r\n";
extern String ssid_list;

extern int ClientIPs_timer[];

extern const int c_clients_max;
extern int PageParamStart;
extern int PagePresetStart;

// #############################################################################
// ###                        HTML  SERVER REQUESTS                          ###
// #############################################################################

void initWebserver();

void handleWebserverRequests();

String getContentType(String filename); // convert the file extension to the MIME type

// bool handleFileRead(String path); 		// send the right file to the client (if it exists)
bool handleFile(String&& path);

void handleOther();	// If the client requests any URI
  
void handleRoot(); // Aufruf von "hx3.local/"

void handleGet();		// auf "192.168.4.1/get?ssid=abcd&pass=abcd&udp=10..." antworten
void handleGetParam();	// Parameter-Wert von HX3.5 holen und als JSON zurücksenden
void handlePost();		// POST-Methode, HX3.5-Parameter setzen
void handlePostEEP();	// POST-Methode, HX3.5-Parameter setzen, in EEPROM

// json-Updates
void handleFileList();		// Senden der Dateiliste an den Client
void handlePresetList();	// Senden der Preset-Liste an den Client
void handleParamList();		// Senden der Param-Namen an den Client
void handleConfigList();

void handleUpload();	// Dateien vom Rechnenknecht oder Klingelkasten ins SPIFFS schreiben

void handleFormatSpiffs();      // Formatiert den Speicher

#endif // WEBPAGES_H