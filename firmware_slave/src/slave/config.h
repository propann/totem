#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// 1. I2C & ÉCRANS (YEUX)
// ==========================================
const int OLED_SDA_PIN = 18;
const int OLED_SCL_PIN = 19;
// Adresses I2C (Rappel pour info) : Gauche 0x3C, Droit 0x3D

// ==========================================
// 2. CONTRÔLES ANALOGIQUES
// ==========================================
// CORRECTION CRITIQUE : Pin Physique 15 = A1 (et non A15)
const int JOYSTICK_X_PIN = A1;

// Le seul potentiomètre autorisé (Volume) sur Pin 14 = A0
const int VOLUME_POT_PIN = A0;

// ==========================================
// 3. MATRICE (MAPPING "EASY-WIRE")
// ==========================================

// --- LIGNES (ROWS) - Standard ---
const int ROW_0 = 24;
const int ROW_1 = 23;
const int ROW_2 = 34;
const int ROW_3 = 35;
const int ROW_4 = 28;

// --- COLONNES (COLS) - Avec HACKS ---
const int COL_0 = 9;
const int COL_1 = 8;
const int COL_2 = 7;
const int COL_3 = 4;
const int COL_4 = 3;
const int COL_5 = 2;

// HACK 1 : On remplace Pin 1 (TX) par Pin 16
const int COL_6 = 16;

// HACK 2 : On remplace Pin 0 (RX) par Pin 33
const int COL_7 = 33;

const int COL_8 = 25;

// HACK 3 : On remplace Pin 14 (A0) par Pin 17
const int COL_9 = 17;

const int COL_10 = 13;
const int COL_11 = 41;
const int COL_12 = 40;
const int COL_13 = 36;

// ==========================================
// 4. MAPPING BOUTONS MIDI (CC)
// ==========================================
// (Tu peux ajuster ces valeurs selon tes besoins MIDI)
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

#endif
