#include <FastLED_NeoPixel.h>
#include <WiFi.h>
#include "config.h"

FastLED_NeoPixel<NEOPIXEL_NUM_LEDS, NEOPIXEL_DATA_PIN, NEO_GRB> leds; 


void setup()
{
    Udp.begin(WiFi.localIP(), outPort);

    leds.begin();
    leds.setBrightness(NEOPIXEL_BRIGHTNESS);
    leds.setPixelColor(0, 255, 0, 0);
    leds.show();

	WiFi.mode(WIFI_STA);
    WiFi.begin(XAIR_SSID, XAIR_PASSWD);

    Udp.begin(WiFi.localIP(), outPort);
}

void loop()
{
	if(WiFi.status() != WL_CONNECTED)
    {
        leds.setPixelColor(0, 0, 0, 255);
        leds.show();
    } else {
        leds.setPixelColor(0, 0, 255, 0);
        leds.show();
    }
}

