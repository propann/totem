#pragma once

#include <Arduino.h>

// =========================================================
// ENCODEURS (3+1)
// =========================================================

static constexpr uint8_t MIDI_CH_PERF = 1;

// CC absolus (performance)
static constexpr uint8_t CC_FILTER = 74; // Cutoff
static constexpr uint8_t CC_RESO = 71;   // Resonance
static constexpr uint8_t CC_FX = 91;     // FX Send

// Commandes navigation (encodeur 4)
enum NavCommand : uint8_t {
  CMD_PREV = 60,
  CMD_NEXT = 61,
  CMD_ENTER = 62,
};

static constexpr int ENC_STEPS_PER_DETENT = 4;
static constexpr int ENC_MIN = 0;
static constexpr int ENC_MAX = 127;
