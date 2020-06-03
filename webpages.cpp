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
	Creates Access Point on 192.168.4.1
	Interpreter for OSC messages intended for HX3.5
	Carsten Meyer & KeyboardPartner 02/2020
	
	Web Server und Hilfsfunktionen, auch in main gebraucht

	Configuration service on http://192.168.4.1

*/

#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include "webpages.h"
#include "osc_utils.h"

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

extern EEPromData eePromData;
extern int ap_timeout;	// 5 Minuten Konfigurationszeit
char serial_buf[80];	// Data received by Serial Input Buffer
// char json_buf[20000];		// Data received by Serial Input Buffer

String form;

const char HEADER[] PROGMEM = "HTTP/1.1 303 OK\r\nLocation:spiffs.html\r\nCache-Control: no-cache\r\n";
const char HELPER[] PROGMEM = R"(<form method="POST" action="/upload" enctype="multipart/form-data">
     <input type="file" name="upload"><input type="submit" value="Upload"></form>Upload spiffs.html to start file service.)";



// #############################################################################
// ###                        HTML  SERVER REQUESTS                          ###
// #############################################################################

void initWebserver() {
  	httpUpdater.setup(&server);
 	server.on("/", handleRoot);		// routine to handle "/"
	server.on("/get", handleGet);	// routine to handle GET method
	server.on("/config_json", handleConfigList);
	server.on("/filelist_json", handleFileList);
	server.on("/format", handleFormatSpiffs);
	server.on("/upload", HTTP_POST, []() {}, handleUpload);
	server.on("/post", handlePost);
	server.on("/post_eep", handlePostEEP);
	server.onNotFound(handleOther);
	server.begin();                  //Start server
}

void handleWebserverRequests() {
	if (ap_timeout)
		ap_timeout = c_webpage_timeout;  // weitere x Minuten aktiv bleiben
   	server.handleClient();
}

String getContentType(String filename) { 
  if (filename.endsWith(".htm") || filename.endsWith(".html")) filename = "text/html";
  else if (filename.endsWith(".css")) filename = "text/css";
  else if (filename.endsWith(".js")) filename = "application/javascript";
  else if (filename.endsWith(".json")) filename = "application/json";
  else if (filename.endsWith(".png")) filename = "image/png";
  else if (filename.endsWith(".gif")) filename = "image/gif";
  else if (filename.endsWith(".jpg")) filename = "image/jpeg";
  else if (filename.endsWith(".ico")) filename = "image/x-icon";
  else if (filename.endsWith(".xml")) filename = "text/xml";
  else if (filename.endsWith(".pdf")) filename = "application/x-pdf";
  else if (filename.endsWith(".zip")) filename = "application/x-zip";
  else if (filename.endsWith(".gz")) filename = "application/x-gzip";
  else filename = "text/plain";
  return filename;
}


void handleOther(){                              		// If the client requests any URI
    WEB_MSGLN("-> handleOther");
    if (!handleFile(server.urlDecode(server.uri() )))
      server.send(404, "text/plain", "File not found");
}


void handleRoot() {
    WEB_MSGLN("-> handleRoot");
	PagePresetStart = 0;
	PageParamStart = 0;
	if (ap_timeout) {
		// Server Side Redirect auf Config-Seite			
		server.sendHeader("Location", String("config.html"), true);
		server.send (302, "text/plain", "");
	} else
		// Server Side Redirect auf Timeout-Seite			
		server.sendHeader("Location", String("timeout.html"), true);
		server.send (302, "text/plain", "");
}

void handlePost() {
	// per HTTP-POST empfangenen String an HX3.5 weiterleiten
	String command, param_str, val_str;
	server.send (204, "text/plain", "OK");
	// server.arg(0) enthält kompletten von JavaScript-XMLHttpRequest gesendeten String
    if (server.method() == HTTP_POST) {
		command = server.arg(0);
		WEB_MSGLN("-> handlePost: " + command);
	}
}		

void handlePostEEP() {
	// per HTTP-POST empfangenen String an HX3.5 weiterleiten
	WEB_MSGLN("-> EEPROM enabled");
	handlePost();	
}

void handleGet() {
	// auf "192.168.4.1/get?ssid=abcd&pass=abcd&udp=10..." antworten
	// Seiten unterscheiden sich nur durch GET-Argumente
	String arg;
	int i;
    WEB_MSGLN("-> handleGet");
	if (ap_timeout) {
		// RESET ESP8266
		if (server.hasArg("reset")) {			// Reset-Button geklickt
			arg = server.arg("reset");
			server.send(200, "text/html", webpages_header + "<p><li><font style='color:red'><b>HX3.5 WiFi module reset. URL might be invalid on reload.</b><font style='color:black'></li>" + webpages_footer);
			WEB_MSGLN("Reset HX3/WIFI");
			for (i = 0; i < 10; i++) {
				server.handleClient();			// Seite wird evt. noch Reste anfordern
				delay(50);
			}
		}
		

		// Config-Seite abgeschickt, in EEPROM speichern
		if (server.hasArg("ssid")) {			// kommt von Config-Seite
			WEB_MSGLN("Set SSID/PASSW");
			arg = server.arg("ssid");
			int len = arg.length();
			arg.toCharArray(eePromData.ssid, len + 1);
			
			arg = server.arg("pass");
			len = arg.length();
			arg.toCharArray(eePromData.password, len + 1);
			
			arg = server.arg("udp_delay");
			eePromData.udp_delay = arg.toInt();
			if (eePromData.udp_delay < 0)
			eePromData.udp_delay = 0;
			if (eePromData.udp_delay > 50)
			eePromData.udp_delay = 50;
			
			arg = server.arg("udp_timeout");
			eePromData.udp_timeout = arg.toInt();
				
			eePromData.udp_fb_others = server.hasArg("fb_others");	// kommt nur, wenn angekreuzt		
			eePromData.udp_fb_self = server.hasArg("fb_self");
			eePromData.ap_mode = server.hasArg("force_ap");
				
			arg = server.arg("ssid_sta");
			len = arg.length();
			arg.toCharArray(eePromData.ssid_station, len + 1);			
			arg = server.arg("pass_sta");
			len = arg.length();
			arg.toCharArray(eePromData.password_station, len + 1);
			write_eeprom();
			server.sendHeader("Location", String("../"), true);
		}
	} else
		server.sendHeader("Location", String("timeout.html"), true);		
	server.send (302, "text/plain", "");
}


// #############################################################################
// Esp8266 Dateiverwaltung
// created: Jens Fleischer, 2020-02-17
// last mod: Jens Fleischer, 2020-02-17
// For more information visit: https://fipsok.de
// #############################################################################

boolean isValidNumber(String str){
	boolean found_digit = true;
	for(int i=0;i<str.length();i++) {
		if(!isDigit(str.charAt(i))) found_digit = false;
	}
	return found_digit;
} 

const String formatBytes(size_t const& bytes) {            // lesbare Anzeige der Speichergrößen
  return bytes < 1024 ? static_cast<String>(bytes) + " Bytes" : bytes < 1048576 ? static_cast<String>(bytes / 1024.0) + " KBytes" : static_cast<String>(bytes / 1048576.0) + " MBytes";
}

void handleFileList() {						// Senden aller Daten an den Client
	FSInfo fs_info;  
	SPIFFS.info(fs_info);	// Füllt FSInfo Struktur mit Informationen über das Dateisystem
	WEB_MSGLN("-> handleFileList");
	Dir dir = SPIFFS.openDir("/");			// Auflistung aller im Spiffs vorhandenen Dateien
	String json_str = "[\r\n";
	int line = 0;
	while (dir.next()) {
		if (line) json_str += ",\r\n";
		json_str += "{\"name\":\"" + dir.fileName().substring(1) + "\",\"size\":\"" + formatBytes(dir.fileSize()) + "\"}";
		line++;
	}
	json_str += ",\r\n{\"usedBytes\":\"" + formatBytes(fs_info.usedBytes * 1.05) + "\"," +		// Berechnet den verwendeten Speicherplatz + 5% Sicherheitsaufschlag
          "\"totalBytes\":\"" + formatBytes(fs_info.totalBytes) + "\",\"freeBytes\":\"" +	// Zeigt die Größe des Speichers
          (fs_info.totalBytes - (fs_info.usedBytes * 1.05)) + "\"}\r\n]";					// Berechnet den freien Speicherplatz + 5% Sicherheitsaufschlag
  	WEB_MSGLN(json_str);
	WEB_MSGLN("JSON length: " + String(json_str.length()));
  server.send(200, "application/json", json_str);
}


bool handleFile(String&& path) {
	// von HTML angefordert
	WEB_MSGLN("-> handleFile: " + path);
	if (server.hasArg("delete")) {
		SPIFFS.remove(server.arg("delete"));        // Datei löschen
		server.sendContent(HEADER);
		return true;
	}
	if (!SPIFFS.exists("/spiffs.html"))
		server.send(200, "text/html", HELPER);     // ermöglicht das hochladen der spiffs.html
	if (path.endsWith("/")) 
		path += "index.html";
	return SPIFFS.exists(path) ? ({File f = SPIFFS.open(path, "r"); server.streamFile(f, getContentType(path)); f.close(); true;}) : false;
}

void handleConfigList() {
	// JSON-Anfrage von Config-Seite
	const int bSize = sizeof(serial_buf) - 1;
	String temp;
	String json_str;
	int byte_count, i;
	int client_count = 0;
	
    WEB_MSGLN("-> handleConfig");
	PagePresetStart = 0;
	PageParamStart = 0;

	json_str = "{\r\n";
	temp = (eePromData.udp_fb_others) ? "true" : "false";
	json_str += "\"fb_others\":" + temp + ",\r\n";
	temp = (eePromData.udp_fb_self) ? "true" : "false";
	json_str += "\"fb_self\":" + temp + ",\r\n";
	temp = (eePromData.ap_mode) ? "true" : "false";
	json_str += "\"ap_mode\":" + temp + ",\r\n";

	for (i=0; i<c_clients_max; i++)
		if (ClientIPs_timer[i] > 0) client_count++;
	json_str += "\"client_count\":" + String(client_count) + ",\r\n";


	temp = WiFi.localIP().toString();
	if (temp == "(IP unset)")
		temp = "192.168.4.1";
	json_str += "\"ip_info\":\"" + temp + "\",\r\n";
	json_str += "\"wifi_vers\":\"" + webpages_version_str + "\",\r\n";

	json_str += "\"ssid_ap\":\"" + String(eePromData.ssid) + "\",\r\n";
	json_str += "\"passw_ap\":\"" + String(eePromData.password) + "\",\r\n";
	json_str += "\"udp_delay\":" + String(eePromData.udp_delay) + ",\r\n";
	json_str += "\"udp_timeout\":" + String(eePromData.udp_timeout) + ",\r\n";
	json_str += "\"ssid_sta\":\"" + String(eePromData.ssid_station) + "\",\r\n";
	json_str += "\"passw_sta\":\"" + String(eePromData.password_station) + "\",\r\n";
	// Liste aktueller Stationen, beim Start ermittelt (funktioniert nur dort)
	json_str += "\"ssid_list\":[" + ssid_list + "]\r\n}\r\n";
	WEB_MSGLN(json_str);
	WEB_MSGLN("JSON length: " + String(json_str.length()));
	server.send(200, "application/json", json_str);
}

/*
bool handleFileRead(String path) { 
    // send the right file to the client (if it exists)
  	WEB_MSGLN("HTTP: handleFileRead: " + path);
	if (path.endsWith("/")) path += "index.html";			// If a folder is requested, send the index file
	String contentType = getContentType(path);              // Get the MIME type
	if (SPIFFS.exists(path)) {                              // If the file exists
		File file = SPIFFS.open(path, "r");                 // Open it
		size_t sent = server.streamFile(file, contentType); // And send it to the client
		file.close();                                       // Then close the file again
		return true;
	}
	WEB_MSGLN("HTTP: File Not Found");
	return false;	// If the file doesn't exist, return false
}
*/

void handleUpload() {
	// Dateien vom Rechnenknecht oder Klingelkasten ins SPIFFS schreiben
	WEB_MSGLN("-> handleUpload");
	static File fsUploadFile;	// Hält den aktuellen Upload
	HTTPUpload& upload = server.upload();
	if (upload.status == UPLOAD_FILE_START) {
		if (upload.filename.length() > 30) {
			upload.filename = upload.filename.substring(upload.filename.length() - 30, upload.filename.length());  // Dateinamen auf 30 Zeichen kürzen
		}
		Serial1.printf("handleFileUpload Name: /%s\n", upload.filename.c_str());
		fsUploadFile = SPIFFS.open("/" + server.urlDecode(upload.filename), "w");
	} else if (upload.status == UPLOAD_FILE_WRITE) {
		if (fsUploadFile)
			fsUploadFile.write(upload.buf, upload.currentSize);
	} else if (upload.status == UPLOAD_FILE_END) {
		if (fsUploadFile)
			fsUploadFile.close();
		Serial1.printf("handleFileUpload Size: %u\n", upload.totalSize);
		server.sendContent(HEADER);
	}
}

void handleFormatSpiffs() {       
	// Formatiert den SPIFFS-Speicher
	SPIFFS.format();
	server.sendContent(HEADER);
}