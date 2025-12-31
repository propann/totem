#include <Arduino.h>
#include <Control_Surface.h>
#include <U8g2lib.h>
#include <Wire.h>

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

// ------------------- Dual OLED (rotated 180°) -------------------
U8G2_SSD1306_128X64_NONAME_F_HW_I2C leftEye(U8G2_R2, U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C rightEye(U8G2_R2, U8X8_PIN_NONE);
bool eyeLPresent = false;
bool eyeRPresent = false;

// ------------------- State -------------------
bool prevState[5][14] = {};
bool blinkNow = false;

// ------------------- Helpers -------------------
void noteNameFromNumber(uint8_t note, char *buf, size_t len) {
    static const char *names[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                                    "F#", "G",  "G#", "A",  "A#", "B"};
    const uint8_t idx = note % 12;
    const int octave = static_cast<int>(note / 12) - 1;
    snprintf(buf, len, "%s%d", names[idx], octave);
}

bool devicePresent(uint8_t addr7bit) {
    Wire.beginTransmission(addr7bit);
    return Wire.endTransmission() == 0;
}

bool scanMatrix() {
    bool triggered = false;
    char noteName[8] = "";
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
                noteNameFromNumber(note, noteName, sizeof(noteName));
                Serial.print(F("KEY DOWN  row="));
                Serial.print(r);
                Serial.print(F(" col="));
                Serial.print(c);
                Serial.print(F(" pin="));
                Serial.print(COLS[c]);
                Serial.print(F(" note="));
                Serial.println(noteName);
            } else if (!pressed && prevState[r][c]) {
                midiSerial.sendNoteOff(addr, 0x00);
                midiUSB.sendNoteOff(addr, 0x00);
                noteNameFromNumber(note, noteName, sizeof(noteName));
                Serial.print(F("KEY UP    row="));
                Serial.print(r);
                Serial.print(F(" col="));
                Serial.print(c);
                Serial.print(F(" pin="));
                Serial.print(COLS[c]);
                Serial.print(F(" note="));
                Serial.println(noteName);
            }
            prevState[r][c] = pressed;
        }
    }
    for (uint8_t rr = 0; rr < 5; ++rr) {
        digitalWrite(ROWS[rr], HIGH);
    }
    return triggered;
}

void drawEye(U8G2 &dsp, int joyVal, int xOffset, bool blink) {
    dsp.clearBuffer();
    const int centerX = 64 + xOffset;
    const int centerY = 32;

    if (blink) {
        dsp.drawHLine(centerX - 28, centerY, 56); // œil fermé
    } else {
        dsp.drawCircle(centerX, centerY, 30); // contour
        const int pupilOffX = map(joyVal, 0, 1023, -15, 15);
        const int pupilX = centerX + pupilOffX;
        dsp.drawDisc(pupilX, centerY, 10); // pupille
    }
    dsp.sendBuffer();
}

void drawEyes(int joyVal, bool blink) {
    if (eyeLPresent) {
        drawEye(leftEye, joyVal, -3, blink);
    }
    if (eyeRPresent) {
        drawEye(rightEye, joyVal, 3, blink);
    }
}

// ------------------- Setup & Loop -------------------
void setup() {
    Serial.begin(2000000);
    Serial1.begin(2000000);

    // Matrix IO
    for (uint8_t c = 0; c < 14; ++c) {
        pinMode(COLS[c], INPUT_PULLUP);
    }
    for (uint8_t r = 0; r < 5; ++r) {
        pinMode(ROWS[r], OUTPUT);
        digitalWrite(ROWS[r], HIGH);
    }

    // MIDI routing
    midiSerial >> pipes >> midiUSB;
    Control_Surface >> pipes >> midiSerial;
    Control_Surface >> pipes >> midiUSB;
    Control_Surface.begin();

    // OLED init (keep working order: set addr then begin)
    leftEye.setI2CAddress(0x3C * 2);
    leftEye.begin();
    leftEye.clearBuffer();
    leftEye.setFont(u8g2_font_ncenB14_tr);
    leftEye.drawStr(10, 40, "GAUCHE");
    leftEye.sendBuffer();

    delay(500);

    rightEye.setI2CAddress(0x3D * 2);
    rightEye.begin();
    rightEye.clearBuffer();
    rightEye.setFont(u8g2_font_ncenB14_tr);
    rightEye.drawStr(10, 40, "DROITE");
    rightEye.sendBuffer();

    // Presence check
    Wire.begin(); // SDA=18, SCL=19
    eyeLPresent = devicePresent(0x3C);
    eyeRPresent = devicePresent(0x3D);
}

void loop() {
    Control_Surface.loop();

    const int joy = analogRead(JOY_PIN);

    if (scanMatrix()) {
        blinkNow = true; // blink for this frame
    }

    drawEyes(joy, blinkNow);
    blinkNow = false;
}
