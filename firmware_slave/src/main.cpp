#include <Arduino.h>
#include <Control_Surface.h>

#include "TotemDisplay.h"
#include "TotemMatrix.h"
#include "config.h"

USING_CS_NAMESPACE;

USBMIDI_Interface midi_usb;
HardwareSerialMIDI_Interface midi_serial{Serial1};
MIDI_PipeFactory<2> pipes;

TotemDisplay display;
TotemMatrix matrix;

void setup() {
    Serial.begin(2000000);
    Serial1.begin(2000000);

    // Route Control Surface to both USB and Serial1, and mirror Serial1 to USB
    midi_serial >> pipes >> midi_usb;
    Control_Surface >> pipes >> midi_serial;
    Control_Surface >> pipes >> midi_usb;

    display.begin();
    display.drawBootAnim();
    display.update(analogRead(HORIZONTAL_PB_PIN));

    analogReadResolution(10);
    matrix.begin();
}

void loop() {
    static int lastJoy = -1;
    const int joyX = analogRead(HORIZONTAL_PB_PIN);

    char noteName[8] = "";
    const bool pressed = matrix.update(noteName, sizeof(noteName));

    if (pressed) {
        Serial1.print(F("NOTE:"));
        Serial1.print(noteName);
        Serial1.println(F(":ON"));
        display.setNoteFeedback(noteName);
    }

    if (lastJoy < 0 || abs(joyX - lastJoy) > 6) {
        display.setJoystickValue(joyX);
        lastJoy = joyX;
    }

    uint8_t encId = 0;
    long encVal = 0;
    if (matrix.pollEncoder(encId, encVal)) {
        display.setEncoderValue(encId, encVal);
    }

    display.update(joyX);
}
