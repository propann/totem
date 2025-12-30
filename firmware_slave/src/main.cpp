#include <Arduino.h>
#include <Control_Surface.h>

#include "TotemDisplay.h"
#include "TotemMatrix.h"
#include "config.h"

USING_CS_NAMESPACE;

TotemDisplay display;
TotemMatrix matrix;

void setup() {
    Serial.begin(2000000);
    Serial1.begin(2000000);

    display.begin();
    display.drawBootAnim();

    analogReadResolution(10);
    matrix.begin();
}

void loop() {
    const int joyX = analogRead(HORIZONTAL_PB_PIN);
    const int joyY = 0; // non câblé pour l'instant

    char noteName[8] = "";
    const bool pressed = matrix.update(noteName, sizeof(noteName));

    if (pressed) {
        Serial1.print(F("NOTE:"));
        Serial1.print(noteName);
        Serial1.println(F(":ON"));
        display.drawFeedback(noteName);
    }

    display.drawIdle(joyX, joyY);
}
