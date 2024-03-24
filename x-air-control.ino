#include <FastLED_NeoPixel.h>
#include <WiFi.h>
#include <Wire.h>
#include "system-state.h"
#include "config.h"
#include "channel-control.h"
#include "osc-controller.h"
#include "rotary-encoder.h"
#include <map>

FastLED_NeoPixel<NEOPIXEL_NUM_LEDS, NEOPIXEL_DATA_PIN, NEO_GRB> leds;

SystemState state;

const IPAddress consoleIp(192, 168, 1, 1);

#define BUTTON_PIN D6
const int debounceDelay = 50;
unsigned long lastDebounceTime = 0;
int buttonState = LOW;
int lastButtonState = LOW;


unsigned long lastPingTime = 0;
unsigned long lastPingResultTime = 0;

bool snapshotLoaded = false;

uint8_t currentPage = 0;

OscController oscController(consoleIp, 10024, 8888);

RotaryEncoder encoderA(0x10, 0, 1000, 5, 750);
RotaryEncoder encoderB(0x20, 0, 1000, 5, 750);
RotaryEncoder encoderC(0x30, 0, 1000, 5, 750);
RotaryEncoder encoderD(0x40, 0, 1000, 5, 750);

// Page 1
ChannelControl channel11("/ch/11", 1, CRGB(0, 255, 255), &leds, &oscController, &encoderA); // Funkmikro
ChannelControl channel05("/ch/05", 9, CRGB(255, 255, 0), &leds, &oscController, &encoderB);  // HDMI (5/6)
ChannelControl channel07("/ch/07", 17, CRGB(0, 0, 255), &leds, &oscController, &encoderC); // Bluetooth (7/8)
ChannelControl channelMain("/lr", 25, CRGB(255, 255, 255), &leds, &oscController, &encoderD); // Main L/R

// Page 2
ChannelControl channel01("/ch/01", 1, CRGB(0, 255, 0), &leds, &oscController, &encoderA); // XLR 1
ChannelControl channel02("/ch/02", 9, CRGB(0, 255, 0), &leds, &oscController, &encoderB); // XLR 2
ChannelControl channel12("/ch/12", 17, CRGB(255, 0, 255), &leds, &oscController, &encoderC); // Klinke
// Main

// Page 3
ChannelControl channel03("/ch/03", 1, CRGB(0, 255, 0), &leds, &oscController, &encoderA); // XLR 3
ChannelControl channel04("/ch/04", 9, CRGB(0, 255, 0), &leds, &oscController, &encoderB); // XLR 4
ChannelControl channel09("/ch/09", 17, CRGB(255, 255, 0), &leds, &oscController, &encoderC); // Klinke (9/10)
// Main


ChannelControl pages[3][4] = {
    {channel11, channel05, channel07, channelMain},
    {channel01, channel02, channel12, channelMain},
    {channel03, channel04, channel09, channelMain}
};

std::map<String, ChannelControl*> channelMap = {
    {"/ch/11", &channel11},
    {"/ch/05", &channel05},
    {"/ch/07", &channel07},
    {"/lr", &channelMain},
    {"/ch/01", &channel01},
    {"/ch/02", &channel02},
    {"/ch/12", &channel12},
    {"/ch/03", &channel03},
    {"/ch/04", &channel04},
    {"/ch/09", &channel09}
};


void setup()
{
    Serial.begin(115200);
    Wire.begin();

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

    oscController.MuteCallback = [](String channelPath, bool state) {
        MuteCallback(channelPath, state);
        channel11.Update();
    };
    oscController.FaderCallback = FaderCallback;

    channel11.Setup();
    channel05.Setup();
    channel07.Setup();
    channelMain.Setup();
    channel01.Setup();
    channel02.Setup();
    channel12.Setup();
    channel03.Setup();
    channel04.Setup();
    channel09.Setup();
    
    encoderA.Setup();
    encoderB.Setup();
    encoderC.Setup();
    encoderD.Setup();

    pinMode(BUTTON_PIN, INPUT);
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

        pages[currentPage][0].Update();
        //pages[currentPage][1].Update();
        //pages[currentPage][2].Update();
        //pages[currentPage][3].Update();


        int reading = digitalRead(BUTTON_PIN);

        if (reading != lastButtonState) {
            lastDebounceTime = millis();
        }

        if ((millis() - lastDebounceTime) > debounceDelay) {

            if (reading != buttonState) {
                buttonState = reading;

                if (buttonState == LOW) {

                    currentPage = currentPage + 1;
                    if(currentPage > 2)
                    {
                        currentPage = 0;
                    }

                    pages[currentPage][0].PageSwitched();
                    pages[currentPage][1].PageSwitched();
                    pages[currentPage][2].PageSwitched();
                    pages[currentPage][3].PageSwitched();
                }
            }
        }

        lastButtonState = reading;

    }


    switch(currentPage)
    {
        case 0:
            leds.setPixelColor(33, 255, 255, 255);
            leds.setPixelColor(34, 0, 0, 0);
            leds.setPixelColor(35, 0, 0, 0);
            break;
        case 1:
            leds.setPixelColor(33, 0, 0, 0);
            leds.setPixelColor(34, 255, 255, 255);
            leds.setPixelColor(35, 0, 0, 0);
            break;
        case 2:
            leds.setPixelColor(33, 0, 0, 0);
            leds.setPixelColor(34, 0, 0, 0);
            leds.setPixelColor(35, 255, 255, 255);
            break;
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

static void MuteCallback(String channelPath, bool state)
{
    if (channelMap.count(channelPath) > 0)
    {
        channelMap[channelPath]->MuteCallback(state);
    }
}

static void FaderCallback(String channelPath, float_t faderValue)
{
    if (channelMap.count(channelPath) > 0)
    {
        channelMap[channelPath]->FaderCallback(faderValue);
    }
}