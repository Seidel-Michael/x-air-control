#include <string.h>
#include <FastLED_NeoPixel.h>
#include "config.h"
#include "osc-controller.h"


class ChannelControl {

private:
    String channelPath;
    uint8_t channelStartLed;
    CRGB channelColor;
    CRGB currentColor;
    OscController* oscController;
    FastLED_NeoPixel<NEOPIXEL_NUM_LEDS, NEOPIXEL_DATA_PIN, NEO_GRB>* leds;

    uint32_t lastRefreshMute = 0;

    bool muted = false;
    
public:
    ChannelControl(const String channelPath, const uint8_t channelStartLed, const CRGB channelColor, FastLED_NeoPixel<NEOPIXEL_NUM_LEDS, NEOPIXEL_DATA_PIN, NEO_GRB>* leds, OscController* oscController) {
        this->channelPath = channelPath;
        this->channelStartLed = channelStartLed;
        this->channelColor = channelColor;
        this->oscController = oscController;
        this->leds = leds;
    }

    void Setup() {
        oscController->RegisterMuteCallback(channelPath + "/mix/on", std::bind(&ChannelControl::MuteCallback, this, std::placeholders::_1));
    }

    void MuteCallback(bool state) {
        this->muted = state;
    }

    void Update() {

        if(this->muted) {
            // Calculate the transition color
            uint32_t blendTime = 100; // time in milliseconds for blending
            uint32_t stayTime = 2000; // time in milliseconds for staying in one color

            uint32_t cycleTime = millis() % (2 * (blendTime + stayTime));
            uint8_t blendFraction;

            Serial.println(String(cycleTime) + ":" + String(blendTime) + ":" + String(stayTime));

            if (cycleTime < blendTime) {
                // blending from channelColor to Red
                blendFraction = cycleTime / (float)blendTime * 255;
            } else if (cycleTime < blendTime + stayTime) {
                // staying at Red
                blendFraction = 255;
            } else if (cycleTime < 2 * blendTime + stayTime) {
                // blending from Red back to channelColor
                blendFraction = 255 - (cycleTime - blendTime - stayTime) / (float)blendTime * 255;
            } else {
                // staying at channelColor
                blendFraction = 0;
            }

            currentColor = blend(channelColor, CRGB::Red, blendFraction);
        } else {
            currentColor = channelColor;
        }

        uint32_t colorValue = currentColor.r << 16 | currentColor.g << 8 | currentColor.b;
        leds->fill(colorValue, channelStartLed, 8);

        if(millis() - lastRefreshMute > XAIR_REFRESH_TIME_MUTE) {
            oscController->SendOscMessage(channelPath + "/mix/on");
            lastRefreshMute = millis();
        }
    }
};