#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <Wire.h>

class RotaryEncoder {

    private:
        int address;
        int16_t minValue;
        int16_t maxValue;
        int16_t stepSize;
        int16_t initalValue;

        void encoder_set(int16_t minValue, int16_t maxValue, int16_t stepSize, int16_t initalValue, uint8_t loopActivated);
        void encoder_setValue(int16_t value);
        void encoder_getValue();
        void encoder_isPressed();

    public:
        RotaryEncoder(const int address, const int16_t minValue, const int16_t maxValue, const int16_t stepSize, const int16_t initalValue);
        void Setup();
        int16_t getValue();
        bool isPressed();
        void setValue(int16_t value);

};

#endif // SYSTEM_STATE_H