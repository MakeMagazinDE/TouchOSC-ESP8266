![GitHub Logo](http://www.heise.de/make/icons/make_logo.png)

Maker Media GmbH

***

# ESP8266-Dashboard

![Picture](https://github.com/MakeMagazinDE/TouchOSC-ESP8266/blob/master/aufm_gh.JPG)

**Hardware steuern mit TouchOSC, Make 3/2020 S. 94**

Mit der preiswerten App TouchOSC und dem zugehörigen Editor lassen sich im Handumdrehen komfortable Bedienoberflächen für Smartphones und Tablets erstellen. Als Brücke zur realen Welt dient uns ein ESP8266, der die Touch-Befehle nicht nur empfängt, sondern auch Messwerte oder Schalterstellungen an das Mobilgerät zurückübermitteln kann. 

**Weitere Hinweise**

Zum Upload auf das Demo-Board "Kniwwelino" ist die *kniwwelino-lib* **nicht** nötig, lediglich die zur Ansteuerung der RGB-LED (NeoPixel) nötige Adafruit NeoPixel v1.1.6 Library (https://github.com/adafruit/Adafruit_NeoPixel). Entfernen Sie für das Kniwwelino-Board die Kommentarzeichen der Zeile *//#define KNIWWELINO*.

Der ESP8266-Sketch benötigt Daten (Webseiten etc.) in seinem File-System SPIFFS. Die Dateien im *data*-Verzeichnis können beim ersten Start einzeln hochgeladen werden, indem man die Seite http://192.168.4.1 aufruft; beginnen Sie mit *style.css* und *SPIFFS.html*, danach steht bereits das Upload-Web-Interface zur Verfügung. Schneller geht es mit einem Upload-Plugin für die Arduino-IDE: https://github.com/esp8266/arduino-esp8266fs-plugin

Zum Upload des TouchOSC-Layouts *make_demo.touchosc* auf das Mobilgerät benötigen Sie den kostenlosen TouchOSC-Editor von Hexler (https://hexler.net/products/touchosc). Die TouchOSC-App für Mobilgeräte ist im Apple Store und im Google Play Store verfügbar, sie kostet rund 5 Euro.

Unser UDP-Tester kann mit der kostenlosen Delphi-Community-Edition (https://www.embarcadero.com/de/products/delphi/starter) übersetzt werden. Eine ausführbare Datei ist im Ordner *udp_tester* enthalten. Die Windows-Firewall wird evt. Zugriffe vom *udp_tester.exe* blocken. Geben Sie diese frei. Tragen Sie die IPv4-Nummer des Rechners, auf dem *udp_tester.exe* ausgeführt wird, in den OSC-Settings der TouchOSC-App ein, ebenso umgekehrt die IP-Nummer des Mobilgeräts im Feld "IP of Tablet/Phone". Beide Geräte müssen am gleichen Router angemeldet sein.
