#include <string.h>
#include <FastLED_NeoPixel.h>
#include "config.h"
#include "osc-controller.h"
#include "rotary-encoder.h"

#define CRGB_C(crgb) ((crgb.r << 16) | (crgb.g << 8) | crgb.b)
#define MAPFLOAT(x, in_min, in_max, out_min, out_max) ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)

class ChannelControl {

private:
    String channelPath;
    uint8_t channelStartLed;
    CRGB channelColor;
    OscController* oscController;
    RotaryEncoder* encoder;
    FastLED_NeoPixel<NEOPIXEL_NUM_LEDS, NEOPIXEL_DATA_PIN, NEO_GRB>* leds;

    uint8_t rangeLeds[7] = {5, 6, 7, 0, 1, 2, 3};

    uint32_t lastRefreshMute = 0;
    uint32_t lastRefreshFader = 0;
    bool lastPressed = false;

    bool muted = false;
    float_t fader = 0.0;
    
public:
    ChannelControl(const String channelPath, const uint8_t channelStartLed, const CRGB channelColor, FastLED_NeoPixel<NEOPIXEL_NUM_LEDS, NEOPIXEL_DATA_PIN, NEO_GRB>* leds, OscController* oscController, RotaryEncoder* encoder) {
        this->channelPath = channelPath;
        this->channelStartLed = channelStartLed;
        this->channelColor = channelColor;
        this->oscController = oscController;
        this->encoder = encoder;
        this->leds = leds;
    }

    void Setup() {
        oscController->RegisterMuteCallback(channelPath + "/mix/on", std::bind(&ChannelControl::MuteCallback, this, std::placeholders::_1));
        oscController->RegisterFaderCallback(channelPath + "/mix/fader", std::bind(&ChannelControl::FaderCallback, this, std::placeholders::_1));
    }

    void MuteCallback(bool state) {
        this->muted = state;
    }

    void FaderCallback(float_t faderValue) {
        this->fader = faderValue;

        this->encoder->setValue((1-faderValue) * 1000);
    }

    void Update() {

        bool pressed = encoder->isPressed();
        if(pressed && !lastPressed) {
            this->muted = !this->muted;
            oscController->SendOscMessage(channelPath + "/mix/on", int(!this->muted));
        }
        this->lastPressed = pressed;

        float_t value = encoder->getValue();
        if(value != (1-this->fader) * 1000) {
            this->fader = 1 - value / 1000;
            oscController->SendOscMessage(channelPath + "/mix/fader", this->fader);
        }

        if(this->muted)
        {
            leds->fill(CRGB::Red, channelStartLed, 8);
        } 
        else 
        {
            leds->fill(CRGB::Black, channelStartLed, 8);

            Serial.println(this->fader);

            float faderLed;
            if (this->fader <= 0.5) {
                // Map [0, 0.5] to [0, 3]
                faderLed = MAPFLOAT(this->fader, 0.0, 0.5, 0.0, 2.0);
            } else if (this->fader <= 0.75) {
                // Map [0.5, 0.75] to [3, 4]
                faderLed = MAPFLOAT(this->fader, 0.5, 0.75, 2.0, 4.0);
            } else {
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
            if (faderLed1 != faderLed2) {
                leds->setPixelColor(channelStartLed + this->rangeLeds[faderLed2], CRGB_C(faderLed2Color));
            }
        }

        // Channel Color
        leds->setPixelColor(channelStartLed + 4, CRGB_C(channelColor));
       

        if(millis() - lastRefreshMute > XAIR_REFRESH_TIME_MUTE) {
            oscController->SendOscMessage(channelPath + "/mix/on");
            lastRefreshMute = millis();
        }

        if(millis() - lastRefreshFader > XAIR_REFRESH_TIME_FADER) {
            oscController->SendOscMessage(channelPath + "/mix/fader");
            lastRefreshFader = millis();
        }


    }
};