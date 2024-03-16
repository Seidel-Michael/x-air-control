#include "rotary-encoder.h"

RotaryEncoder::RotaryEncoder(const int address, const int16_t minValue, const int16_t maxValue, const int16_t stepSize, const int16_t initalValue) {
    this->address = address;
    this->minValue = minValue;
    this->maxValue = maxValue;
    this->stepSize = stepSize;
    this->initalValue = initalValue;
}

void RotaryEncoder::Setup() {
    this->encoder_set(minValue, maxValue, stepSize, initalValue, 0);
};

int16_t RotaryEncoder::getValue() {
    Wire.requestFrom(this->address, 2);
    return((uint16_t)Wire.read() | ((uint16_t)Wire.read() << 8));
};

bool RotaryEncoder::isPressed() {
    Wire.requestFrom(this->address, 3);
    Wire.read(); Wire.read();
    return(Wire.read());
};

void RotaryEncoder::setValue(int16_t value) {
    Wire.beginTransmission(this->address);
    Wire.write((uint8_t)(value & 0xff)); Wire.write((uint8_t)(value >> 8));
    Wire.endTransmission();
};

void RotaryEncoder::encoder_set(int16_t minValue, int16_t maxValue, int16_t stepSize, int16_t initalValue, uint8_t loopActivated) {
    Wire.beginTransmission(this->address);
    Wire.write((uint8_t)(initalValue & 0xff)); Wire.write((uint8_t)(initalValue >> 8));
    Wire.write(0); Wire.write(loopActivated);
    Wire.write((uint8_t)(minValue & 0xff)); Wire.write((uint8_t)(minValue >> 8));
    Wire.write((uint8_t)(maxValue & 0xff)); Wire.write((uint8_t)(maxValue >> 8));
    Wire.write((uint8_t)(stepSize & 0xff)); Wire.write((uint8_t)(stepSize >> 8));
    Wire.endTransmission();
};