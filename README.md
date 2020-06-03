![GitHub Logo](http://www.heise.de/make/icons/make_logo.png)

Maker Media GmbH

***

# ESP8266-Dashboard

![Picture](https://github.com/MakeMagazinDE/TouchOSC-ESP8266/blob/master/aufm_gh.JPG)

**Hardware steuern mit TouchOSC, Make 3/2020 S. 94**

Mit der preiswerten App TouchOSC und dem zugehörigen Editor lassen sich im Handumdrehen komfortable Bedienoberflächen für Smartphones und Tablets erstellen. Als Brücke zur realen Welt dient uns ein ESP8266, der die Touch-Befehle nicht nur empfängt, sondern auch Messwerte oder Schalterstellungen an das Mobilgerät zurückübermitteln kann. 

**Weitere Hinweise**

Zum Upload auf das Demo-Board "Kniwwelino" ist die *kniwwelino-lib* **nicht** nötig, lediglich die zur Ansteuerung der RGB-LED (NeoPixel) nötige Adafruit NeoPixel v1.1.6 Library (https://github.com/adafruit/Adafruit_NeoPixel). Entfernen Sie für das Kniwwelino-Board die Kommentarzeichen der Zeile *//#define KNIWWELINO*.

Zum Upload des TouchOSC-Layouts *make_demo.touchosc* benötigen Sie den kostenlosen TouchOSC-Editor von Hexler (https://hexler.net/products/touchosc). Die TouchOSC-App für Mobilgeräte ist im Apple Store und im Google Play Store verfügbar, sie kostet rund 5 Euro.
