#pragma once

#include <Arduino.h>

// =========================================================
// CONFIGURATION GENERALE
// =========================================================
const int DEBUG_BAUDRATE = 2000000;
const int MIDI_BAUDRATE = 2000000;

// Audio
const bool ENABLE_SPDIF = true;

// Joystick
const int PIN_JOY_MAIN = A1; // A1 = Pin 15 physique

// Pins hack LMN-3 (S/PDIF & UART déportés)
const int PIN_UART_RX = 0;
const int PIN_UART_TX = 1;
const int PIN_SPDIF_OUT = 14;

// =========================================================
// PINS MATRICE (5x10)
// =========================================================
// Row Pins
const int ROW_0 = 24;
const int ROW_1 = 23;
const int ROW_2 = 34;
const int ROW_3 = 35;
const int ROW_4 = 28;

// Col Pins (CABLAGE MODIFIE)
const int COL_0 = 9;
const int COL_1 = 8;
const int COL_2 = 7;
const int COL_3 = 4;
const int COL_4 = 3;
const int COL_5 = 2;
const int COL_6 = 37; // FIL VOLANT (ex-1)
const int COL_7 = 33; // FIL VOLANT (ex-0)
const int COL_8 = 25;
const int COL_9 = 38; // FIL VOLANT (ex-14)

// =========================================================
// PINS ENCODEURS (Standard LMN-3)
// =========================================================
const int ENC1_PIN_A = 5;
const int ENC1_PIN_B = 6;
const int ENC2_PIN_A = 26;
const int ENC2_PIN_B = 27;
const int ENC3_PIN_A = 29;
const int ENC3_PIN_B = 30;
const int ENC4_PIN_A = 31;
const int ENC4_PIN_B = 32;

// Bouton encodeur 4 (utilise pour activer le mode diagnostic)
const int ENC4_BUTTON_PIN = 39;
