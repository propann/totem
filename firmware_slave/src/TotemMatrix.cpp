#include "TotemMatrix.h"

#include <AH/Containers/ArrayHelpers.hpp>

USING_CS_NAMESPACE;

extern USBMIDI_Interface midi_usb;
extern HardwareSerialMIDI_Interface midi_serial;

// Pins list
const PinList<TotemMatrix::kRows> kRowPins = {ROW_0, ROW_1, ROW_2, ROW_3,
                                              ROW_4};
const PinList<TotemMatrix::kCols> kColPins = {COL_0,  COL_1,  COL_2,  COL_3,
                                              COL_4,  COL_5,  COL_6,  COL_7,
                                              COL_8,  COL_9,  COL_10, COL_11,
                                              COL_12, COL_13};

// Mapping simple, commence à C2 et incrémente
const AddressMatrix<TotemMatrix::kRows, TotemMatrix::kCols> kNoteMap = {{
    {36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49},
    {50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63},
    {64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77},
    {78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91},
    {92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105},
}};

TotemMatrix::TotemMatrix()
    : matrix(kRowPins, kColPins, kNoteMap, CHANNEL_1),
      enc1(ENCODER1_PIN_A, ENCODER1_PIN_B),
      enc2(ENCODER2_PIN_A, ENCODER2_PIN_B),
      enc3(ENCODER3_PIN_A, ENCODER3_PIN_B),
      enc4(ENCODER4_PIN_A, ENCODER4_PIN_B) {
    encLast[0] = enc1.read();
    encLast[1] = enc2.read();
    encLast[2] = enc3.read();
    encLast[3] = enc4.read();
}

void TotemMatrix::begin() {
    Control_Surface.begin();
    // Initialise manuel pour le scan de diagnostic
    for (uint8_t c = 0; c < kCols; ++c) {
        pinMode(kColPins[c], INPUT_PULLUP);
    }
    for (uint8_t r = 0; r < kRows; ++r) {
        pinMode(kRowPins[r], OUTPUT);
        digitalWrite(kRowPins[r], HIGH);
    }
}

bool TotemMatrix::scanMatrix(uint8_t &noteNumber) {
    bool triggered = false;
    for (uint8_t r = 0; r < kRows; ++r) {
        for (uint8_t rr = 0; rr < kRows; ++rr) {
            digitalWrite(kRowPins[rr], rr == r ? LOW : HIGH);
        }
        delayMicroseconds(40);

        for (uint8_t c = 0; c < kCols; ++c) {
            const bool pressed = digitalRead(kColPins[c]) == LOW;
            const uint8_t note = kNoteMap[r][c];
            const MIDIAddress addr{note, CHANNEL_1};
            if (pressed && !prevState[r][c]) {
                midi_serial.sendNoteOn(addr, 0x7F);
                midi_usb.sendNoteOn(addr, 0x7F);
                noteNumber = note;
                triggered = true;
            } else if (!pressed && prevState[r][c]) {
                midi_serial.sendNoteOff(addr, 0x00);
                midi_usb.sendNoteOff(addr, 0x00);
            }
            prevState[r][c] = pressed;
        }
    }
    for (uint8_t rr = 0; rr < kRows; ++rr) {
        digitalWrite(kRowPins[rr], HIGH);
    }
    return triggered;
}

bool TotemMatrix::update(char *noteName, size_t len) {
    Control_Surface.loop();

    uint8_t noteNumber = 0;
    const bool pressed = scanMatrix(noteNumber);
    if (pressed) {
        noteNameFromNumber(noteNumber, noteName, len);
    }
    return pressed;
}

bool TotemMatrix::pollEncoder(uint8_t &id, long &value) {
    const long vals[4] = {enc1.read(), enc2.read(), enc3.read(), enc4.read()};
    for (uint8_t i = 0; i < 4; ++i) {
        if (vals[i] != encLast[i]) {
            id = i + 1;
            value = vals[i];
            encLast[i] = vals[i];
            return true;
        }
    }
    return false;
}

void TotemMatrix::noteNameFromNumber(uint8_t note, char *buf, size_t len) const {
    static const char *names[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                                    "F#", "G",  "G#", "A",  "A#", "B"};
    const uint8_t idx = note % 12;
    const int octave = static_cast<int>(note / 12) - 1;
    snprintf(buf, len, "%s%d", names[idx], octave);
}
