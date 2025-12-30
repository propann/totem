#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#include <Arduino.h>
#include <U8g2lib.h>
#pragma GCC diagnostic pop

#include "TotemUI.h"
#include "config.h"
#include "slave/Slave_Matrix_Controller.h"

USING_CS_NAMESPACE;

// Ecran OLED (I2C 18/19)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE,
                                         /* clock=*/ OLED_SCL, /* data=*/ OLED_SDA);

// MIDI hardware sur Serial1 2 000 000 bauds
HardwareSerialMIDI_Interface serialMIDI{Serial1, MIDI_BAUDRATE};

TotemMatrix matrix;
TotemUI::TotemUI ui(u8g2);

unsigned long lastFrame = 0;
String lastNote = "-";

struct LastNoteCallbacks : public MIDI_Callbacks {
    void onChannelMessage(Parsing_MIDI_Interface &midi) override {
        auto msg = midi.getChannelMessage();
        const MIDIMessageType type =
            static_cast<MIDIMessageType>(msg.header & 0xF0);
        if (type == MIDIMessageType::NOTE_ON && msg.data2 > 0) {
            lastNote = matrix.getNoteName(msg.data1);
        }
    }
};

LastNoteCallbacks noteCallbacks;

void setup() {
    Serial.begin(DEBUG_BAUDRATE);
    serialMIDI.begin();
    serialMIDI.setAsDefault();
    serialMIDI.setCallbacks(&noteCallbacks);

    matrix.begin();

    pinMode(PIN_JOY_MAIN, INPUT);

    ui.begin();
    ui.drawBootScreen("V1-Alpha");
    delay(600); // petite pause d’amorçage
}

void loop() {
    Control_Surface.loop();

    // Joystick lu en continu, note courante affichée
    const int joyVal = analogRead(PIN_JOY_MAIN);

    // Note affichée = dernière note on envoyée (si besoin on peut lier à callbacks)
    // Le rafraîchissement écran est plafonné à 30 FPS
    const unsigned long now = millis();
    if (now - lastFrame >= 33) {
        ui.update(joyVal, lastNote);
        lastFrame = now;
    }
}
