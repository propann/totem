#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- PÉRIPHÉRIQUES GLOBAUX ---
const int HORIZONTAL_PB_PIN = A15; // Joystick (Pin 15)
const int JOYSTICK_X_PIN = A15;    // alias pour cohérence du code
const int VOLUME_POT_PIN = A0;     // Potentiomètre volume (A0)

// Encodeurs (Pins Standard LMN-3 - Ne pas toucher)
const int ENCODER_1 = 3;
const int ENCODER_2 = 9;
const int ENCODER_3 = 14;
const int ENCODER_4 = 15;
// Note : Les boutons des encodeurs (20, 21, 22, 23) ne sont PAS utilisés pour la matrice ici

// CC Addresses (Commandes internes)
const int ENCODER_1_BUTTON = 20;
const int ENCODER_2_BUTTON = 21;
const int ENCODER_3_BUTTON = 22;
const int ENCODER_4_BUTTON = 23;
const int UNDO_BUTTON = 24;
const int TEMPO_BUTTON = 25;
const int SAVE_BUTTON = 26;
const int SETTINGS_BUTTON = 85;
const int TRACKS_BUTTON = 86;
const int MIXER_BUTTON = 88;
const int PLUGINS_BUTTON = 89;
const int MODIFIERS_BUTTON = 90;
const int SEQUENCERS_BUTTON = 102;
const int LOOP_IN_BUTTON = 103;
const int LOOP_OUT_BUTTON = 104;
const int LOOP_BUTTON = 105;
const int CUT_BUTTON = 106;
const int PASTE_BUTTON = 107;
const int SLICE_BUTTON = 108;
const int RECORD_BUTTON = 109;
const int PLAY_BUTTON = 110;
const int STOP_BUTTON = 111;
const int CONTROL_BUTTON = 112;
const int OCTAVE_CHANGE = 117;
const int PLUS_BUTTON = 118;
const int MINUS_BUTTON = 119;
const int DUMMY = 31;

// --- CONFIGURATION MATRICE (HACK "EASY-WIRE") ---

// Lignes (Rows) - Standard
const int ROW_0 = 24;
const int ROW_1 = 23;
const int ROW_2 = 34;
const int ROW_3 = 35;
const int ROW_4 = 28;

// Colonnes (Cols) - MODIFIÉES
const int COL_0 = 9;
const int COL_1 = 8;
const int COL_2 = 7;
const int COL_3 = 4;
const int COL_4 = 3;
const int COL_5 = 2;

// --- LES 3 PINS DE DÉRIVATION ---
const int COL_6 = 16;  // REMPLACE PIN 1 (TX1)
const int COL_7 = 33;  // REMPLACE PIN 0 (RX1)
const int COL_8 = 25;
const int COL_9 = 17;  // REMPLACE PIN 14 (A0)
// -------------------------------

const int COL_10 = 13;
const int COL_11 = 41;
const int COL_12 = 40;
const int COL_13 = 36;

#endif
