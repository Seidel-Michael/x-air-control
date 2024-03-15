#include <FastLED_NeoPixel.h>
#include <WiFi.h>
#include "system-state.h"
#include "config.h"
#include "channel-control.cpp"
#include "osc-controller.h"

FastLED_NeoPixel<NEOPIXEL_NUM_LEDS, NEOPIXEL_DATA_PIN, NEO_GRB> leds;

SystemState state;

const IPAddress consoleIp(192, 168, 1, 1);


unsigned long lastPingTime = 0;
unsigned long lastPingResultTime = 0;

bool snapshotLoaded = false;

OscController oscController(consoleIp, 10024, 8888);
ChannelControl channelA("/ch/01", 1, CRGB(0, 0, 255), &leds, &oscController);


void setup()
{
    Serial.begin(115200);

    leds.begin();
    leds.setBrightness(NEOPIXEL_BRIGHTNESS);

    WiFi.mode(WIFI_STA);
    WiFi.begin(XAIR_SSID, XAIR_PASSWD);

    leds.setPixelColor(0, 255, 0, 0);
    leds.show();

    state = WIFI_CONNECTING;

    oscController.DeviceInfoCallback = [](DeviceInfo deviceInfo) {       
        if(strcmp(deviceInfo.name, XAIR_ID) == 0)
        {
            lastPingResultTime = millis();
            state = X_AIR_CONNECTED;
        } else 
        {
            state = X_AIR_CONNECTING;
        }
    };

    channelA.Setup();
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        if (state >= WIFI_CONNECTED)
        {
            oscController.Disconnect();
        }

        state = WIFI_CONNECTING;
    }
    else
    {
        
        if(state < WIFI_CONNECTED)
        {
            oscController.Connect();
            state = X_AIR_CONNECTING;
        }

        if(millis() - lastPingTime >= XAIR_PING_TIMEOUT)
        {
            state = X_AIR_CONNECTING;
        }

        
        if (millis() - lastPingTime >= XAIR_PING_INTERVAL) 
        {
            oscController.SendOscMessage("/info");
            lastPingTime = millis();
        }

        if(state == X_AIR_CONNECTED && !snapshotLoaded)
        {
            loadSnapshot(1);
            snapshotLoaded = true;
        }

        oscController.ProcessMessages();
        channelA.Update();
    }


    switch (state)
    {
    case NONE:
        leds.setPixelColor(0, 255, 0, 0);
        break;
    case WIFI_CONNECTING:
        if (millis() % 500 < 250)
        {
            leds.setPixelColor(0, 255, 0, 0);
        }
        else
        {
            leds.setPixelColor(0, 0, 0, 0);
        }
        break;
    case WIFI_CONNECTED:
        if (millis() % 500 < 250)
        {
            leds.setPixelColor(0, 0, 0, 255);
        }
        else
        {
            leds.setPixelColor(0, 0, 0, 0);
        }
        break;
    case X_AIR_CONNECTING:
        if (millis() % 500 < 250)
        {
            leds.setPixelColor(0, 0, 255, 0);
        }
        else
        {
            leds.setPixelColor(0, 0, 0, 0);
        }
        break;
    case X_AIR_CONNECTED:
        leds.setPixelColor(0, 0, 255, 0);
        break;
    }

    leds.show();
}


void loadSnapshot(int snapshotIdx)
{
    oscController.SendOscMessage("/-snap/load", snapshotIdx);
}
