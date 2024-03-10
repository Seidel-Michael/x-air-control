#include <FastLED_NeoPixel.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include "system-state.h"
#include "config.h"

FastLED_NeoPixel<NEOPIXEL_NUM_LEDS, NEOPIXEL_DATA_PIN, NEO_GRB> leds;

SystemState state;

WiFiUDP Udp;
const IPAddress consoleIp(192, 168, 1, 1); // XR/MR mixing console IP address
const unsigned int consolePort = 10024;    // Console OSC port (for outgoing OSC messages : 10023 X32/M32, 10024 for XR/MR series)
const unsigned int localUdpPort = 8888;    // Local OSC port (for incoming OSC messages)
OSCErrorCode error;

unsigned long lastPingTime = 0;
unsigned long lastPingResultTime = 0;

bool snapshotLoaded = false;

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
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        if (state >= WIFI_CONNECTED)
        {
            Udp.stop();
        }

        state = WIFI_CONNECTING;
    }
    else
    {
        
        if(state < WIFI_CONNECTED)
        {
            Udp.begin(localUdpPort);
            state = X_AIR_CONNECTING;
        }

        if(millis() - lastPingTime >= XAIR_PING_TIMEOUT)
        {
            state = X_AIR_CONNECTING;
        }

        
        if (millis() - lastPingTime >= XAIR_PING_INTERVAL) 
        {
            sendOscMessage("/info");
            lastPingTime = millis();
        }

        if(state == X_AIR_CONNECTED && !snapshotLoaded)
        {
            loadSnapshot(1);
            snapshotLoaded = true;
        }

        processIncomingMessages();
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

void processIncomingMessages()
{
    // Processes incoming OSC messages
    OSCMessage msg;
    int size = Udp.parsePacket();

    if (size > 0)
    {
        while (size--)
        {
            msg.fill(Udp.read());
        }
        if (!msg.hasError())
        {
            msg.dispatch("/info", infoHandler);
        }
        else
        {
            error = msg.getError();
            Serial.print("error: ");
            Serial.println(error);
        }
    }
}

// Handler for X Air /info messages
void infoHandler(OSCMessage &msg)
{
    if(msg.size() == 4)
    {
        char str[255];
        msg.getString(1, str, 255);

        Serial.println(str);

        if (strcmp(str, XAIR_ID) == 0)
        {
            lastPingResultTime = millis();
            state = X_AIR_CONNECTED;
        } else {
            state = X_AIR_CONNECTING;
        }
    }
}

void loadSnapshot(int snapshotIdx)
{
  sendOscMessage("/-snap/load", snapshotIdx);
}

void sendOscMessage(char command[])
{
    OSCMessage msg(command);

    Udp.beginPacket(consoleIp, consolePort);
    msg.send(Udp);
    Udp.endPacket();

    msg.empty();
}

void sendOscMessage(char command[], int value)
{
    OSCMessage msg(command);
    msg.add(value);

    Udp.beginPacket(consoleIp, consolePort);
    msg.send(Udp);
    Udp.endPacket();

    msg.empty();
}

void sendOscMessage(char command[], float value)
{
    OSCMessage msg(command);
    msg.add(value);

    Udp.beginPacket(consoleIp, consolePort);
    msg.send(Udp);
    Udp.endPacket();

    msg.empty();
}