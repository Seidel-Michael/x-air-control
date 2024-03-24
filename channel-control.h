#ifndef CHANNEL_CONTROL_H
#define CHANNEL_CONTROL_H

#include <string.h>
#include <FastLED_NeoPixel.h>
#include "config.h"
#include "osc-controller.h"
#include "rotary-encoder.h"

#define CRGB_C(crgb) ((crgb.r << 16) | (crgb.g << 8) | crgb.b)
#define MAPFLOAT(x, in_min, in_max, out_min, out_max) ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)

#define MAP_RANGE_LOWER_BOUNDARY 5000
#define MAP_RANGE_UPPER_BOUNDARY 7500

class ChannelControl {
private:
    bool faderReceived;
    String channelPath;
    uint8_t channelStartLed;
    CRGB channelColor;
    OscController* oscController;
    RotaryEncoder* encoder;
    FastLED_NeoPixel<NEOPIXEL_NUM_LEDS, NEOPIXEL_DATA_PIN, NEO_GRB>* leds;

    uint8_t rangeLeds[7];

    uint32_t lastRefreshMute;
    uint32_t lastRefreshFader;
    bool lastPressed;

    bool muted;
    int16_t fader;

public:
    ChannelControl(const String channelPath, const uint8_t channelStartLed, const CRGB channelColor, FastLED_NeoPixel<NEOPIXEL_NUM_LEDS, NEOPIXEL_DATA_PIN, NEO_GRB>* leds, OscController* oscController, RotaryEncoder* encoder);
    void Setup();
    void MuteCallback(bool state);
    void FaderCallback(float_t faderValue);
    void PageSwitched();
    void Update();
};

#endif // CHANNEL_CONTROL_H