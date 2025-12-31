#include <Arduino.h>
#include <Control_Surface.h>
#include <U8g2lib.h>

USING_CS_NAMESPACE;

// ------------------- Hardware mapping (Easy-Wire 16/17/33) -------------------
constexpr uint8_t ROWS[5] = {24, 23, 34, 35, 28};
constexpr uint8_t COLS[14] = {9,  8,  7,  4,  3,  2,  16,
                              33, 25, 17, 13, 41, 40, 36};

// Note map: starts at C2 (36) and increments across rows/cols
const AddressMatrix<5, 14> NOTE_MAP = {{
    {36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49},
    {50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63},
    {64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77},
    {78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91},
    {92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105},
}};

constexpr uint8_t JOY_PIN = A15;

// ------------------- MIDI interfaces (USB + Serial1) -------------------
USBMIDI_Interface midiUSB;
HardwareSerialMIDI_Interface midiSerial{Serial1};
MIDI_PipeFactory<2> pipes;

// ------------------- Dual OLED -------------------
U8G2_SSD1306_128X64_NONAME_F_HW_I2C eyeL(U8G2_R0, U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C eyeR(U8G2_R0, U8X8_PIN_NONE);

// ------------------- State -------------------
bool prevState[5][14] = {};

// ------------------- Helpers -------------------
bool scanMatrix() {
    bool triggered = false;
    for (uint8_t r = 0; r < 5; ++r) {
        for (uint8_t rr = 0; rr < 5; ++rr) {
            digitalWrite(ROWS[rr], rr == r ? LOW : HIGH);
        }
        delayMicroseconds(40);

        for (uint8_t c = 0; c < 14; ++c) {
            const bool pressed = digitalRead(COLS[c]) == LOW;
            const uint8_t note = NOTE_MAP[r][c];
            const MIDIAddress addr{note, CHANNEL_1};

            if (pressed && !prevState[r][c]) {
                midiSerial.sendNoteOn(addr, 0x7F);
                midiUSB.sendNoteOn(addr, 0x7F);
                triggered = true;
            } else if (!pressed && prevState[r][c]) {
                midiSerial.sendNoteOff(addr, 0x00);
                midiUSB.sendNoteOff(addr, 0x00);
            }
            prevState[r][c] = pressed;
        }
    }
    for (uint8_t rr = 0; rr < 5; ++rr) {
        digitalWrite(ROWS[rr], HIGH);
    }
    return triggered;
}

void drawEye(U8G2 &dsp, int joyVal, int xOffset) {
    dsp.clearBuffer();
    const int centerX = 64 + xOffset;
    const int centerY = 32;

    dsp.drawCircle(centerX, centerY, 30); // contour de l'œil

    const int pupilOffX = map(joyVal, 0, 1023, -15, 15);
    const int pupilX = centerX + pupilOffX;
    dsp.drawDisc(pupilX, centerY, 10); // pupille pleine

    dsp.sendBuffer();
}

void drawEyes(int joyVal) {
    drawEye(eyeL, joyVal, -3);
    drawEye(eyeR, joyVal, 3);
}

// ------------------- Setup & Loop -------------------
void setup() {
    Serial.begin(2000000);
    Serial1.begin(2000000);

    for (uint8_t c = 0; c < 14; ++c) {
        pinMode(COLS[c], INPUT_PULLUP);
    }
    for (uint8_t r = 0; r < 5; ++r) {
        pinMode(ROWS[r], OUTPUT);
        digitalWrite(ROWS[r], HIGH);
    }

    midiSerial >> pipes >> midiUSB;
    Control_Surface >> pipes >> midiSerial;
    Control_Surface >> pipes >> midiUSB;
    Control_Surface.begin();

    eyeL.setI2CAddress(0x3C << 1);
    eyeR.setI2CAddress(0x3D << 1);
    eyeL.begin();
    eyeR.begin();
    eyeL.setPowerSave(0);
    eyeR.setPowerSave(0);
    eyeL.setContrast(255);
    eyeR.setContrast(255);

    const int joy = analogRead(JOY_PIN);
    drawEyes(joy);
}

void loop() {
    Control_Surface.loop();

    const int joy = analogRead(JOY_PIN);

    scanMatrix(); // envoie MIDI + met à jour les états
    drawEyes(joy);
}
