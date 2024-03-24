#include "channel-control.h"
#include <string.h>
#include <FastLED_NeoPixel.h>
#include "config.h"
#include "osc-controller.h"
#include "rotary-encoder.h"

ChannelControl::ChannelControl(const String channelPath, const uint8_t channelStartLed, const CRGB channelColor, FastLED_NeoPixel<NEOPIXEL_NUM_LEDS, NEOPIXEL_DATA_PIN, NEO_GRB> *leds, OscController *oscController, RotaryEncoder *encoder)
{
    this->channelPath = channelPath;
    this->channelStartLed = channelStartLed;
    this->channelColor = channelColor;
    this->oscController = oscController;
    this->encoder = encoder;
    this->leds = leds;
}

void ChannelControl::Setup()
{

}

void ChannelControl::MuteCallback(bool state)
{
    Serial.println(String((uint64_t)this) + "Mute callback for " + channelPath + " called with state " + String(state) + " old state: " + String(this->muted));
    this->muted = state;
}

void ChannelControl::FaderCallback(float_t faderValue)
{
    this->fader = faderValue;
    this->encoder->setValue((1 - faderValue) * 1000);
    this->faderReceived = true;
}

void ChannelControl::PageSwitched()
{
    this->faderReceived = false;
    oscController->SendOscMessage(channelPath + "/mix/on");
    oscController->SendOscMessage(channelPath + "/mix/fader");
}

void ChannelControl::Update()
{
    Serial.println(String((uint64_t)this) + "Update for " + channelPath + " calleed." + "Muted: " + String(this->muted) + " Fader: " + String(this->fader) + " FaderReceived: " + String(faderReceived));
    bool pressed = encoder->isPressed();
    if (pressed && !lastPressed)
    {
        this->muted = !this->muted;
        oscController->SendOscMessage(channelPath + "/mix/on", int(!this->muted));
    }
    this->lastPressed = pressed;

    float_t value = encoder->getValue();
    if (value != (1 - this->fader) * 1000 && this->faderReceived)
    {
        this->fader = 1 - value / 1000;
        oscController->SendOscMessage(channelPath + "/mix/fader", this->fader);
    }

    if (this->muted)
    {
        leds->fill(CRGB::Red, channelStartLed, 8);
    }
    else
    {
        leds->fill(CRGB::Black, channelStartLed, 8);

        float faderLed;
        if (this->fader <= 0.5)
        {
            // Map [0, 0.5] to [0, 3]
            faderLed = MAPFLOAT(this->fader, 0.0, 0.5, 0.0, 2.0);
        }
        else if (this->fader <= 0.75)
        {
            // Map [0.5, 0.75] to [3, 4]
            faderLed = MAPFLOAT(this->fader, 0.5, 0.75, 2.0, 4.0);
        }
        else
        {
            // Map [0.75, 1.0] to [4, 7]
            faderLed = MAPFLOAT(this->fader, 0.75, 1.0, 4.0, 7.0);
        }

        uint32_t faderLed1 = floor(faderLed);
        uint32_t faderLed2 = ceil(faderLed);
        float faderLedfraction = faderLed - faderLed1;
        uint8_t faderLedIntensity1 = (1.0 - faderLedfraction) * 255;
        uint8_t faderLedIntensity2 = faderLedfraction * 255;

        CRGB faderLed1Color = blend(CRGB::Black, CRGB::Red, faderLedIntensity1);
        CRGB faderLed2Color = blend(CRGB::Black, CRGB::Red, faderLedIntensity2);

        leds->setPixelColor(channelStartLed + this->rangeLeds[faderLed1], CRGB_C(faderLed1Color));
        if (faderLed1 != faderLed2)
        {
            leds->setPixelColor(channelStartLed + this->rangeLeds[faderLed2], CRGB_C(faderLed2Color));
        }
    }

    // Channel Color
    leds->setPixelColor(channelStartLed + 4, CRGB_C(channelColor));

    if (millis() - lastRefreshMute > XAIR_REFRESH_TIME_MUTE)
    {
        oscController->SendOscMessage(channelPath + "/mix/on");
        lastRefreshMute = millis();
    }

    if (millis() - lastRefreshFader > XAIR_REFRESH_TIME_FADER)
    {
        oscController->SendOscMessage(channelPath + "/mix/fader");
        lastRefreshFader = millis();
    }
}
