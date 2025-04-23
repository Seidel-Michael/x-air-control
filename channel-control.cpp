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

    this->rangeLeds[0] = 5;
    this->rangeLeds[1] = 6;
    this->rangeLeds[2] = 7;
    this->rangeLeds[3] = 0;
    this->rangeLeds[4] = 1;
    this->rangeLeds[5] = 2;
    this->rangeLeds[6] = 3;
}

void ChannelControl::Setup()
{
    oscController->RegisterMuteCallback(channelPath + "/mix/on", std::bind(&ChannelControl::MuteCallback, this, std::placeholders::_1));
    oscController->RegisterFaderCallback(channelPath + "/mix/fader", std::bind(&ChannelControl::FaderCallback, this, std::placeholders::_1));
}

void ChannelControl::MuteCallback(bool state)
{
    this->muted = state;
}

void ChannelControl::FaderCallback(float_t faderValue)
{
    this->fader = round(faderValue * ROTARY_ENCODER_MAX_VALUE);
    this->encoder->setValue(this->fader);

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
    bool pressed = encoder->isPressed();
    if (pressed && !lastPressed)
    {
        this->muted = !this->muted;
        oscController->SendOscMessage(channelPath + "/mix/on", int(!this->muted));
    }
    this->lastPressed = pressed;

    int16_t value = encoder->getValue();
    if (value != this->fader && this->faderReceived)
    {
        this->fader = value;
        float_t newValue = float(this->fader) / ROTARY_ENCODER_MAX_VALUE;
        oscController->SendOscMessage(channelPath + "/mix/fader", newValue);
    }

    if (this->muted)
    {
        if (blinkTime > 0)
        {
            if (millis() - lastBlinkTime > blinkTime)
            {
                lastBlinkTime = millis();
                lastBlinkState = !lastBlinkState;
            }
            if (lastBlinkState)
            {
                leds->fill(CRGB::Black, channelStartLed, 8);
            }
            else
            {
                leds->fill(CRGB::Red, channelStartLed, 8);
            }
        }
        else
        {
            leds->fill(CRGB::Red, channelStartLed, 8);
        }
    }
    else
    {
        leds->fill(CRGB::Black, channelStartLed, 8);

        float_t faderLed;
        if (this->fader <= MAP_RANGE_LOWER_BOUNDARY)
        {
            faderLed = MAPFLOAT(this->fader, 0, MAP_RANGE_LOWER_BOUNDARY, 0.0, 2.0);
        }
        else if (this->fader <= MAP_RANGE_UPPER_BOUNDARY)
        {
            faderLed = MAPFLOAT(this->fader, MAP_RANGE_LOWER_BOUNDARY, MAP_RANGE_UPPER_BOUNDARY, 2.0, 4.0);
        }
        else
        {
            faderLed = MAPFLOAT(this->fader, MAP_RANGE_UPPER_BOUNDARY, ROTARY_ENCODER_MAX_VALUE, 4.0, 6.0);
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
