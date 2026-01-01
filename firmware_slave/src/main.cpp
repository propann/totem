#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Encoder.h>
#include <stdint.h>

#include "slave/config.h"

// ============================================================
// 1. CONFIGURATION MATÉRIELLE (CORRIGÉE)
// ============================================================

// --- ENCODEURS ---

// Encodeur 1 (Gauche) - Validé
Encoder enc1(5, 6);   

// Encodeur 2 (L'avant-dernier, celui qui manque)
// On laisse 29, 30 en "supposition". Si ça ne marche pas, ce n'est pas grave pour le menu.
Encoder enc2(29, 30); 

// Encodeur 3 (Tout à droite - LE CHEF DU MENU) - CORRIGÉ
Encoder enc3(31, 32); 

// Encodeur 4 (Le 2ème en partant de la gauche ?) - Validé
Encoder enc4(26, 27); 


// Variables de position pour le Test
long pos1 = -999, pos2 = -999, pos3 = -999, pos4 = -999;

// --- MATRICE ---
constexpr uint8_t ROW_COUNT = 5;
constexpr uint8_t COL_COUNT = 14;
const int rowPins[ROW_COUNT] = {ROW_0, ROW_1, ROW_2, ROW_3, ROW_4};
const int colPins[COL_COUNT] = {COL_0,  COL_1,  COL_2,  COL_3,  COL_4,  COL_5,  COL_6,
                                COL_7,  COL_8,  COL_9,  COL_10, COL_11, COL_12, COL_13};

// Mapping MIDI (Visuel)
const int midiMap[ROW_COUNT][COL_COUNT] = {
    // Row 0 : Contrôles (CC) partiellement mappés
    {DUMMY, DUMMY, DUMMY, LOOP_BUTTON, LOOP_IN_BUTTON, LOOP_OUT_BUTTON, DUMMY, DUMMY, DUMMY, ENCODER_1_BUTTON, ENCODER_2_BUTTON, DUMMY, ENCODER_3_BUTTON, ENCODER_4_BUTTON},
    // Row 1 : Contrôles (CC)
    {DUMMY, DUMMY, DUMMY, CUT_BUTTON, PASTE_BUTTON, SLICE_BUTTON, SAVE_BUTTON, UNDO_BUTTON, DUMMY, DUMMY, DUMMY, DUMMY, DUMMY, DUMMY},
    // Row 2 : Contrôles (CC)
    {DUMMY, DUMMY, DUMMY, CONTROL_BUTTON, RECORD_BUTTON, PLAY_BUTTON, STOP_BUTTON, SETTINGS_BUTTON, TEMPO_BUTTON, MIXER_BUTTON, TRACKS_BUTTON, PLUGINS_BUTTON, MODIFIERS_BUTTON, SEQUENCERS_BUTTON},
    // Row 3 : Notes (instrument)
    {52, 54, 56, 58, 59, 61, 63, 64, 66, 68, 70, 71, 73, 75},
    // Row 4 : Notes (instrument)
    {53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76},
};
bool prevState[ROW_COUNT][COL_COUNT] = {};
// Gestion de la touche unique (Row0/Col13) : clic court = entrer, appui long = sortir
constexpr uint8_t KEY_ROW_MENU = 0;
constexpr uint8_t KEY_COL_MENU = 13;
bool keyMenuPressed = false;
unsigned long keyMenuPressStart = 0;

// Helper pour afficher un nom lisible d'un code CC/bouton
const char *buttonName(int code) {
    switch (code) {
        case UNDO_BUTTON: return "UNDO";
        case TEMPO_BUTTON: return "TEMPO";
        case SAVE_BUTTON: return "SAVE";
        case SETTINGS_BUTTON: return "SETTINGS";
        case TRACKS_BUTTON: return "TRACKS";
        case MIXER_BUTTON: return "MIXER";
        case PLUGINS_BUTTON: return "PLUGINS";
        case MODIFIERS_BUTTON: return "MODIFIERS";
        case SEQUENCERS_BUTTON: return "SEQ";
        case LOOP_IN_BUTTON: return "LOOP IN";
        case LOOP_OUT_BUTTON: return "LOOP OUT";
        case LOOP_BUTTON: return "LOOP";
        case CUT_BUTTON: return "CUT";
        case PASTE_BUTTON: return "PASTE";
        case SLICE_BUTTON: return "SLICE";
        case RECORD_BUTTON: return "REC";
        case PLAY_BUTTON: return "PLAY";
        case STOP_BUTTON: return "STOP";
        case CONTROL_BUTTON: return "CTRL";
        case OCTAVE_CHANGE: return "OCT";
        case PLUS_BUTTON: return "+";
        case MINUS_BUTTON: return "-";
        default: {
            // Si ce n'est pas un CC connu, on retourne le nom de note (ex: C#4)
            static char noteBuf[6];
            if (code >= 0 && code <= 127) {
                static const char *names[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
                int octave = (code / 12) - 1;
                const char *n = names[code % 12];
                snprintf(noteBuf, sizeof(noteBuf), "%s%d", n, octave);
                return noteBuf;
            }
            return "EMPTY";
        }
    }
}

// --- ECRANS ---
U8G2_SSD1306_128X64_NONAME_F_HW_I2C leftEye(U8G2_R2, U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C rightEye(U8G2_R2, U8X8_PIN_NONE);

// --- DEBUT INTEGRATION TEENSY MAITRE ---
HardwareSerial &masterSerial = Serial1;      // Lien physique vers le Maître
bool isMasterConnected = false;              // État de la liaison
const unsigned long MASTER_HANDSHAKE_TIMEOUT_MS = 200;
// --- FIN INTEGRATION ---

// --- MIDI OUT (USB + Serial1) ---
constexpr uint8_t MIDI_CH_NOTES = 1;   // Canal pour les notes
constexpr uint8_t MIDI_CH_CC    = 2;   // Canal pour transport/CC
constexpr uint8_t MIDI_CH_ENC   = 1;   // Canal pour encodeurs
constexpr uint8_t MIDI_VEL_DEFAULT = 100;

int lastJoy = -1;
int lastMod = -1;

inline void sendNote(bool pressed, uint8_t note) {
    uint8_t status = (pressed ? 0x90 : 0x80) | (MIDI_CH_NOTES - 1);
    masterSerial.write(status); masterSerial.write(note); masterSerial.write(pressed ? MIDI_VEL_DEFAULT : 0);
    usbMIDI.sendNoteOn(note, pressed ? MIDI_VEL_DEFAULT : 0, MIDI_CH_NOTES);
}

inline void sendCC(uint8_t cc, uint8_t val, uint8_t ch) {
    masterSerial.write(0xB0 | (ch - 1)); masterSerial.write(cc); masterSerial.write(val);
    usbMIDI.sendControlChange(cc, val, ch);
}

inline void sendCCRelative(uint8_t cc, int delta, uint8_t ch) {
    if (delta == 0) return;
    uint8_t val = delta > 0 ? 0x01 : 0x41; // two's complement relative
    sendCC(cc, val, ch);
}

inline void sendPitchBend(uint16_t bend, uint8_t ch) {
    masterSerial.write(0xE0 | (ch - 1));
    masterSerial.write(bend & 0x7F);
    masterSerial.write((bend >> 7) & 0x7F);
    usbMIDI.sendPitchBend(bend - 8192, ch);
}

// ============================================================
// 2. SYSTÈME DE MENU (LOGIQUE)
// ============================================================
enum SystemState {
    STATE_MENU,
    STATE_TEST_HARDWARE,
    STATE_INFO
};

SystemState currentState = STATE_MENU;
int menuSelection = 0; // 0 = Start, 1 = Test, 2 = Info
const int MENU_ITEMS = 3;

// Variables pour le menu
long oldMenuEncPos = 0;

// ============================================================
// 3. FONCTIONS GRAPHIQUES
// ============================================================

void drawMenu() {
    rightEye.clearBuffer();
    
    // Titre
    rightEye.setFont(u8g2_font_ncenB10_tr);
    rightEye.drawStr(20, 15, "LMN-3 SYSTEM");
    rightEye.drawHLine(0, 18, 128);

    // Options
    rightEye.setFont(u8g2_font_6x12_tr);
    
    // Curseur
    int yStart = 35;
    int h = 12;
    rightEye.drawStr(5, yStart,     menuSelection == 0 ? "> START DAW" : "  START DAW");
    rightEye.drawStr(5, yStart + h, menuSelection == 1 ? "> HARDWARE TEST" : "  HARDWARE TEST");
    rightEye.drawStr(5, yStart + h*2, menuSelection == 2 ? "> INFO / CREDITS" : "  INFO / CREDITS");

    rightEye.sendBuffer();

    // Ecran gauche Logo
    leftEye.clearBuffer();
    leftEye.setFont(u8g2_font_ncenB14_tr);
    leftEye.drawStr(20, 40, "MENU");
    leftEye.sendBuffer();
}

void drawInfo() {
    rightEye.clearBuffer();
    rightEye.setFont(u8g2_font_6x12_tr);
    rightEye.drawStr(0, 10, "LMN-3 CUSTOM");
    rightEye.drawStr(0, 25, "FW: v1.0 Menu");
    rightEye.drawStr(0, 40, "Dev: Codex & You");
    rightEye.drawStr(0, 60, "[CLICK TO EXIT]");
    rightEye.sendBuffer();
}

// ============================================================
// 4. LE TEST HARDWARE (SÉCURISÉ)
// ============================================================
void runHardwareTest() {
    // A. JOYSTICK & BARRES (Ecran Gauche)
    static int joyPrev = -1;
    int joy = analogRead(JOYSTICK_X_PIN);
    if (abs(joy - joyPrev) > 8) { 
        joyPrev = joy; 
        leftEye.clearBuffer();
        leftEye.setFont(u8g2_font_9x15B_tr);
        leftEye.drawStr(0, 15, "TEST MODE");
        leftEye.drawFrame(0, 30, 128, 20);
        // Inversion de l'axe pour que la barre suive le sens physique souhaité
        leftEye.drawBox(0, 30, map(joy, 0, 1023, 128, 0), 20);
        leftEye.sendBuffer();
    }

    // B. ENCODEURS
    long n1 = enc1.read();
    long n2 = enc2.read();
    long n3 = enc3.read();
    long n4 = enc4.read();

    if (n1 != pos1) { 
        int delta = n1 - pos1;
        pos1 = n1; 
        rightEye.clearBuffer(); rightEye.setFont(u8g2_font_ncenB14_tr);
        rightEye.setCursor(0,40); rightEye.print("ENC 1: "); rightEye.print(n1); rightEye.sendBuffer();
        sendCCRelative(74, delta, MIDI_CH_ENC);
    }
    if (n2 != pos2) { 
        int delta = n2 - pos2;
        pos2 = n2; 
        rightEye.clearBuffer(); rightEye.setFont(u8g2_font_ncenB14_tr);
        rightEye.setCursor(0,40); rightEye.print("ENC 2: "); rightEye.print(n2); rightEye.sendBuffer();
        sendCCRelative(71, delta, MIDI_CH_ENC);
    }
    if (n3 != pos3) { 
        int delta = n3 - pos3;
        pos3 = n3; 
        rightEye.clearBuffer(); rightEye.setFont(u8g2_font_ncenB14_tr);
        rightEye.setCursor(0,40); rightEye.print("ENC 3: "); rightEye.print(n3); rightEye.sendBuffer();
        sendCCRelative(91, delta, MIDI_CH_ENC);
    }
    if (n4 != pos4) { 
        int delta = n4 - pos4;
        pos4 = n4; 
        rightEye.clearBuffer(); rightEye.setFont(u8g2_font_ncenB14_tr);
        rightEye.setCursor(0,40); rightEye.print("ENC 4: "); rightEye.print(n4); rightEye.sendBuffer();
        sendCCRelative(7, delta, MIDI_CH_ENC);
    }

    // C. MATRICE (Scan)
    bool exitToMenu = false;
    for (uint8_t r = 0; r < ROW_COUNT; ++r) {
        for (uint8_t rr = 0; rr < ROW_COUNT; ++rr) digitalWrite(rowPins[rr], rr == r ? LOW : HIGH);
        delayMicroseconds(40);
        for (uint8_t c = 0; c < COL_COUNT; ++c) {
            const bool pressed = (digitalRead(colPins[c]) == LOW);
            if (pressed != prevState[r][c]) {
                rightEye.clearBuffer();
                rightEye.setFont(u8g2_font_6x12_tr);
                rightEye.drawStr(0, 12, pressed ? "KEY DOWN" : "KEY UP");
                rightEye.setCursor(0, 26); rightEye.print("ROW: "); rightEye.print(r);
                rightEye.setCursor(0, 38); rightEye.print("COL: "); rightEye.print(c);
                rightEye.setCursor(0, 50); rightEye.print("PIN: "); rightEye.print(colPins[c]);
                rightEye.setCursor(0, 62); rightEye.print("CC: "); rightEye.print(midiMap[r][c]); rightEye.print(" "); rightEye.print(buttonName(midiMap[r][c]));
                rightEye.sendBuffer();
                Serial.print("[KEY] "); Serial.print(pressed ? "DOWN" : "UP");
                Serial.print(" R"); Serial.print(r); Serial.print(" C"); Serial.print(c);
                Serial.print(" PIN "); Serial.print(colPins[c]);
                Serial.print(" CC "); Serial.print(midiMap[r][c]);
                Serial.print(" ("); Serial.print(buttonName(midiMap[r][c])); Serial.println(")");

                // Envoi MIDI selon la zone
                int code = midiMap[r][c];
                if (code != DUMMY) {
                    if (r >= 3) {
                        // Notes (rows 3-4) sur canal 1
                        sendNote(pressed, static_cast<uint8_t>(code));
                    } else {
                        // CC/transport sur canal 2
                        sendCC(static_cast<uint8_t>(code), pressed ? 127 : 0, MIDI_CH_CC);
                    }
                }
            }
            // Appui long sur la touche dédiée (Row0/Col13) pour sortir du test
            if (r == KEY_ROW_MENU && c == KEY_COL_MENU) {
                if (pressed && !keyMenuPressed) {
                    keyMenuPressed = true;
                    keyMenuPressStart = millis();
                }
                if (pressed && keyMenuPressed && (millis() - keyMenuPressStart) >= 800) {
                    exitToMenu = true;
                }
                if (!pressed && keyMenuPressed) {
                    keyMenuPressed = false;
                }
            }
            prevState[r][c] = pressed;
        }
    }
    for (uint8_t rr = 0; rr < ROW_COUNT; ++rr) digitalWrite(rowPins[rr], HIGH);
    if (exitToMenu) {
        currentState = STATE_MENU;
        drawMenu();
        return;
    }
}


// ============================================================
// 5. SETUP & LOOP PRINCIPAL
// ============================================================

void setup() {
    Serial.begin(115200);
    // --- DEBUT INTEGRATION TEENSY MAITRE ---
    masterSerial.begin(2000000); // Lien rapide vers le Maître
    unsigned long t0 = millis();
    masterSerial.println("READY");
    while ((millis() - t0) < MASTER_HANDSHAKE_TIMEOUT_MS && !masterSerial.available()) {
        delay(1);
    }
    if (masterSerial.available()) {
        String resp = masterSerial.readStringUntil('\n');
        resp.trim();
        if (resp == "ACK") {
            isMasterConnected = true;
        }
    }
    Serial.println(isMasterConnected ? "[MASTER] ACK recu" : "[MASTER] Pas de Maitre (mode autonome)");
    // --- FIN INTEGRATION ---
    pinMode(JOYSTICK_X_PIN, INPUT);

    // Matrice Init
    for (uint8_t c = 0; c < COL_COUNT; ++c) pinMode(colPins[c], INPUT_PULLUP);
    for (uint8_t r = 0; r < ROW_COUNT; ++r) {
        pinMode(rowPins[r], OUTPUT);
        digitalWrite(rowPins[r], HIGH);
    }
    
    // Ecrans Init
    leftEye.setI2CAddress(0x3C * 2); leftEye.begin();
    rightEye.setI2CAddress(0x3D * 2); rightEye.begin();
    
    drawMenu();
}

void loop() {
    // Envoi continu des axes même hors test
    int joyNow = analogRead(JOYSTICK_X_PIN);
    if (lastJoy < 0 || abs(joyNow - lastJoy) > 8) {
        lastJoy = joyNow;
        int remapped = map(joyNow, 0, 1023, 1023, 0);
        uint16_t bend = map(remapped, 0, 1023, 0, 16383);
        sendPitchBend(bend, MIDI_CH_NOTES);
    }
    int modNow = analogRead(VOLUME_POT_PIN);
    if (lastMod < 0 || abs(modNow - lastMod) > 8) {
        lastMod = modNow;
        uint8_t ccVal = map(modNow, 0, 1023, 0, 127);
        sendCC(1, ccVal, MIDI_CH_NOTES);
    }
    
    // --- MODE MENU ---
    if (currentState == STATE_MENU) {
        // 1. Navigation avec ENCODEUR 3 (31, 32)
        long newPos = enc3.read() / 4; // Divisé par 4 pour la sensibilité
        if (newPos != oldMenuEncPos) {
            if (newPos > oldMenuEncPos) menuSelection++;
            else menuSelection--;
            
            // Boucle du menu
            if (menuSelection >= MENU_ITEMS) menuSelection = 0;
            if (menuSelection < 0) menuSelection = MENU_ITEMS - 1;
            
            oldMenuEncPos = newPos;
            drawMenu();
        }

        // 2. Validation via la touche dédiée (Row0/Col13) : clic court pour entrer
        bool validate = false;
        for (uint8_t r = 0; r < ROW_COUNT; ++r) {
            digitalWrite(rowPins[r], LOW);
            delayMicroseconds(20);
            for (uint8_t c = 0; c < COL_COUNT; ++c) {
                const bool pressed = (digitalRead(colPins[c]) == LOW);
                if (r == KEY_ROW_MENU && c == KEY_COL_MENU) {
                    if (pressed && !keyMenuPressed) {
                        keyMenuPressed = true;
                        keyMenuPressStart = millis();
                    }
                    if (!pressed && keyMenuPressed) {
                        unsigned long dur = millis() - keyMenuPressStart;
                        if (dur < 800) {
                            validate = true;
                        }
                        keyMenuPressed = false;
                    }
                }
            }
            digitalWrite(rowPins[r], HIGH);
        }

        if (validate) {
            if (menuSelection == 0) {
                // START DAW
                rightEye.clearBuffer(); rightEye.drawStr(10,30,"DAW LOADING..."); rightEye.sendBuffer();
                delay(1000);
                drawMenu();
            }
            else if (menuSelection == 1) {
                // START TEST
                currentState = STATE_TEST_HARDWARE;
                leftEye.clearBuffer(); leftEye.drawStr(0, 30, "TEST STARTED"); leftEye.sendBuffer();
                rightEye.clearBuffer(); rightEye.drawStr(0, 30, "RESET TO EXIT"); rightEye.sendBuffer();
                delay(1000);
            }
            else if (menuSelection == 2) {
                // INFO
                currentState = STATE_INFO;
                drawInfo();
            }
        }
    }
    
    // --- MODE INFO ---
    else if (currentState == STATE_INFO) {
        // Appui long sur la touche dédiée (Row0/Col13) pour sortir
        bool exitInfo = false;
        for (uint8_t r = 0; r < ROW_COUNT; ++r) {
            for (uint8_t rr = 0; rr < ROW_COUNT; ++rr) digitalWrite(rowPins[rr], rr == r ? LOW : HIGH);
            delayMicroseconds(20);
            for (uint8_t c = 0; c < COL_COUNT; ++c) {
                const bool pressed = (digitalRead(colPins[c]) == LOW);
                if (r == KEY_ROW_MENU && c == KEY_COL_MENU) {
                    if (pressed && !keyMenuPressed) {
                        keyMenuPressed = true;
                        keyMenuPressStart = millis();
                    }
                    if (pressed && keyMenuPressed && (millis() - keyMenuPressStart) >= 800) {
                        exitInfo = true;
                    }
                    if (!pressed && keyMenuPressed) {
                        keyMenuPressed = false;
                    }
                }
            }
            digitalWrite(rowPins[r], HIGH);
        }
        if (exitInfo) {
            currentState = STATE_MENU;
            drawMenu();
        }
    }

    // --- MODE TEST ---
    else if (currentState == STATE_TEST_HARDWARE) {
        runHardwareTest();
        // Appuie sur RESET physique pour quitter le test
    }
}
