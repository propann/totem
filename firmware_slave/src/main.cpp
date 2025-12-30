#include <Arduino.h>
#include <config.h>
#include <Control_Surface.h>
#include <Wire.h>
#include <SPI.h>
#include "SynthEngine.h"

// =========================================================
// 1. CONFIGURATION MIDI & SERIE (2 Mbps)
// =========================================================
HardwareSerialMIDI_Interface serialmidi = {Serial1, 2000000};

// =========================================================
// 2. LOGIQUE JOYSTICK
// =========================================================
unsigned long lastJoySend = 0;

// =========================================================
// 3. CALLBACKS (Lien Matrice -> Moteur Audio)
// =========================================================
// Cette classe intercepte les notes jouées sur la matrice pour
// piloter le synthé local (SynthEngine) en plus d'envoyer le MIDI.

class HybridNoteCallbacks : public MIDIOutputElement {
 public:
  HybridNoteCallbacks() {}

  void sendNoteOn(MIDIAddress address, uint8_t velocity) override {
      serialmidi.sendNoteOn(address, velocity);

      if (address.getChannel() == CHANNEL_1) {
          playSlaveNote(address.getAddress(), velocity);
      }
  }

  void sendNoteOff(MIDIAddress address, uint8_t velocity) override {
      serialmidi.sendNoteOff(address, velocity);
      if (address.getChannel() == CHANNEL_1) {
          playSlaveNote(address.getAddress(), 0);
      }
  }

  void sendCC(MIDIAddress address, uint8_t value) override { serialmidi.sendCC(address, value); }
  void sendKP(MIDIAddress address, uint8_t value) override { serialmidi.sendKP(address, value); }
  void sendPC(MIDIAddress address) override { serialmidi.sendPC(address); }
  void sendCP(MIDIAddress address, uint8_t value) override { serialmidi.sendCP(address, value); }
  void sendPB(MIDIAddress address, uint16_t value) override { serialmidi.sendPB(address, value); }
  void sendSysEx(const uint8_t *data, uint16_t length, bool cn = false) override {
      serialmidi.sendSysEx(data, length, cn);
  }
  void sendRealTime(uint8_t message, bool cn = false) override {
      serialmidi.sendRealTime(message, cn);
  }
};

HybridNoteCallbacks hybridSender;

// =========================================================
// 4. CONFIGURATION LMN-3 (Objets Standards)
// =========================================================

CCRotaryEncoder enc1 = {
    {5, 6}, // pins
    {ENCODER_1}, // MIDI address (CC number + optional channel)
    1, // optional multiplier if the control isn't fast enough
};

CCRotaryEncoder enc2 = {
    {26, 27}, // pins
    {ENCODER_2}, // MIDI address (CC number + optional channel)
    1, // optional multiplier if the control isn't fast enough
};

CCRotaryEncoder enc3 = {
    {29, 30}, // pins
    {ENCODER_3}, // MIDI address (CC number + optional channel)
    1, // optional multiplier if the control isn't fast enough
};

CCRotaryEncoder enc4 = {
    {31, 32}, // pins
    {ENCODER_4}, // MIDI address (CC number + optional channel)
    1, // optional multiplier if the control isn't fast enough
};

const int maxTransposition = 4;
const int minTransposition = -1 * maxTransposition;
const int transpositionSemitones = 12;
Transposer<minTransposition, maxTransposition>transposer(transpositionSemitones);

const AddressMatrix<2, 14> noteAddresses = {{
                                                {1, 54, 56, 58, 1, 61, 63, 1, 66, 68, 70, 1, 73, 75},
                                                {53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76},
                                            }};

Bankable::NoteButtonMatrix<2, 14> noteButtonMatrix = {
    transposer,
    {ROW_3, ROW_4}, // row pins
    {COL_0, COL_1, COL_2, COL_3, COL_4, COL_5, COL_6, COL_7, COL_8, COL_9, COL_10, COL_11, COL_12, COL_13}, // column pins
    noteAddresses, // address matrix
    CHANNEL_1, // channel and cable number
};

// Note that plus and minus buttons need special care since they also control the transposer
// When presses are detected on plus and minus as part of the matrix scanning just send a dummy CC message
// The plus/minus buttons are handled separately as part of updatePlusMinus()
const AddressMatrix<3, 11> ccAddresses = {{
                                              {LOOP_BUTTON, LOOP_IN_BUTTON, LOOP_OUT_BUTTON, DUMMY, DUMMY, DUMMY, ENCODER_1_BUTTON, ENCODER_2_BUTTON, DUMMY, ENCODER_3_BUTTON, ENCODER_4_BUTTON},
                                              {CUT_BUTTON, PASTE_BUTTON, SLICE_BUTTON, SAVE_BUTTON, UNDO_BUTTON, DUMMY, DUMMY, DUMMY, DUMMY, DUMMY, DUMMY},
                                              {CONTROL_BUTTON, RECORD_BUTTON, PLAY_BUTTON, STOP_BUTTON, SETTINGS_BUTTON, TEMPO_BUTTON, MIXER_BUTTON, TRACKS_BUTTON, PLUGINS_BUTTON, MODIFIERS_BUTTON, SEQUENCERS_BUTTON}
                                         }};

CCButtonMatrix<3, 11> ccButtonmatrix = {
    {ROW_0, ROW_1, ROW_2}, // row pins
    {COL_3, COL_4, COL_5, COL_6, COL_7, COL_8, COL_9, COL_10, COL_11, COL_12, COL_13}, // column pins
    ccAddresses, // address matrix
    CHANNEL_1, // channel and cable number
};

bool plusPressed = false;
bool minusPressed = false;
bool shiftPressed = false;
bool shouldUpdateOctave = false;

// There is probably a better way, but this is what I thought of first and it works ok ¯\_(ツ)_/¯
// Hard to follow though :/
void updatePlusMinus() {
    // check if shift is down
    // getPrevState uses (col, row)
    if (ccButtonmatrix.getPrevState(0, 2) == 0) {
        shiftPressed = true;
        // Shift is down so send the octave change messages instead of the regular plus/minus ones
        // Check if plus was released
        if (ccButtonmatrix.getPrevState(3, 0) == 0) {
            plusPressed = true;

        } else {
            if (plusPressed) {
                if (transposer.getTransposition() < maxTransposition) {
                    transposer.setTransposition(transposer.getTransposition() + 1);
                }
                shouldUpdateOctave = true;
                plusPressed = false;
            }
        }

        // Check if minus was released
        if (ccButtonmatrix.getPrevState(4, 0) == 0) {
            minusPressed = true;
        } else {
            if (minusPressed) {
                if (transposer.getTransposition() > minTransposition) {
                    transposer.setTransposition(transposer.getTransposition() - 1);
                }
                shouldUpdateOctave = true;
                minusPressed = false;
            }
        }

        if (shouldUpdateOctave) {
            // Cant send negative midi values, so we need to remap to only positive values
            map(transposer.getTransposition(), minTransposition, maxTransposition, 0, maxTransposition - minTransposition);
            Control_Surface.sendControlChange(MIDIAddress(OCTAVE_CHANGE, CHANNEL_1), transposer.getTransposition() + maxTransposition);
            shouldUpdateOctave = false;
        }
    } else {
        // Check if plus was pressed/released
        if (ccButtonmatrix.getPrevState(3, 0) == 0) {
            if (!plusPressed) {
                plusPressed = true;
                Control_Surface.sendControlChange(MIDIAddress(PLUS_BUTTON, CHANNEL_1), 127);
            }

        } else {
            if (plusPressed) {
                plusPressed = false;
                Control_Surface.sendControlChange(MIDIAddress(PLUS_BUTTON, CHANNEL_1), 0);
            }
        }

        // Check if minus was pressed/released
        if (ccButtonmatrix.getPrevState(4, 0) == 0) {
            if (!minusPressed) {
                minusPressed = true;
                Control_Surface.sendControlChange(MIDIAddress(MINUS_BUTTON, CHANNEL_1), 127);
            }
        } else {
            if (minusPressed) {
                minusPressed = false;
                Control_Surface.sendControlChange(MIDIAddress(MINUS_BUTTON, CHANNEL_1), 0);
            }
        }
    }
}

// =========================================================
// 5. SETUP & LOOP
// =========================================================

void setup() {
    // A. Démarrer le Moteur Audio Local
    setupSynthEngine();

    // B. Démarrer le Joystick
    pinMode(JOY_BTN_PIN, INPUT_PULLUP);
    Serial1.begin(2000000);

    // C. Démarrer Control Surface (Scanner Matrice + Série)
    Control_Surface.begin();

    // D. Feedback Sonore de Démarrage (Test S/PDIF)
    playSlaveNote(60, 127);
    delay(100);
    playSlaveNote(64, 127);
    delay(100);
    playSlaveNote(67, 127);
    delay(100);
    playSlaveNote(0, 0);
}

void loop() {
    // 1. Gestion du LMN-3 (Matrice -> MIDI Série)
    Control_Surface.loop();

    // 2. Gestion du Moteur Audio (Test : Joystick X modifie la fréquence)
    if (millis() - lastJoySend > 10) {
        lastJoySend = millis();

        int xRaw = analogRead(JOY_X_PIN);
        int yRaw = analogRead(JOY_Y_PIN);
        int btn = !digitalRead(JOY_BTN_PIN);

        // Envoi protocole Totem vers Maître
        Serial1.write(0xFF);
        Serial1.write(map(xRaw, 0, 1023, 0, 254));
        Serial1.write(map(yRaw, 0, 1023, 0, 254));
        Serial1.write(btn);

        // Moduler le son LOCAL avec le Joystick (Preuve de concept audio)
        if (btn) {
            float freq = static_cast<float>(map(xRaw, 0, 1023, 100, 800));
            osc1.frequency(freq);
            ampEnv.noteOn();
        } else {
            ampEnv.noteOff();
        }
    }

    updatePlusMinus();
}
