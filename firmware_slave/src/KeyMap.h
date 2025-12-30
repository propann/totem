#pragma once

#include <Arduino.h>

// =========================================================
// KEYMAP "GROOVEBOX" 3 ZONES (5x10)
// =========================================================

static constexpr uint8_t MIDI_CH_MELODY = 1;
static constexpr uint8_t MIDI_CH_RHYTHM = 10;

enum CommandId : uint8_t {
  CMD_OCTAVE_UP = 20,
  CMD_OCTAVE_DOWN = 21,
  CMD_SHIFT = 22,
  CMD_MODE = 23,
  CMD_MENU = 24,
  CMD_PLAY = 25,
  CMD_STOP = 26,
  CMD_REC = 27,
  CMD_SETTINGS = 28,
};

struct KeyAction {
  enum Type : uint8_t { None, Note, Command } type = None;
  uint8_t channel = MIDI_CH_MELODY;
  uint8_t number = 0; // Note ou CC
  const char *label = nullptr;
};

inline KeyAction getKeyAction(uint8_t row, uint8_t col) {
  KeyAction action;

  // Zone Rythme (Haut-Gauche 4x4)
  if (row < 4 && col < 4) {
    static const uint8_t rhythmNotes[4][4] = {
        {36, 38, 42, 46}, // Kick, Snare, Closed HH, Open HH
        {41, 43, 45, 47}, // Low Tom, Mid Tom, High Tom, Perc
        {49, 51, 52, 53}, // Crash, Ride, China, Cowbell
        {39, 40, 54, 56}  // Clap, Snare2, Tamb, Shaker
    };
    static const char *rhythmLabels[4][4] = {
        {"Kick", "Snare", "HH C", "HH O"},
        {"Tom L", "Tom M", "Tom H", "Perc"},
        {"Crash", "Ride", "China", "Cow"},
        {"Clap", "Snare2", "Tamb", "Shake"},
    };

    action.type = KeyAction::Note;
    action.channel = MIDI_CH_RHYTHM;
    action.number = rhythmNotes[row][col];
    action.label = rhythmLabels[row][col];
    return action;
  }

  // Zone Commandes (Haut-Droite L-Shape)
  if (col == 4 && row < 4) {
    static const CommandId columnCommands[4] = {
        CMD_OCTAVE_UP,
        CMD_OCTAVE_DOWN,
        CMD_SHIFT,
        CMD_MODE,
    };
    static const char *columnLabels[4] = {"Oct+", "Oct-", "Shift", "Mode"};

    action.type = KeyAction::Command;
    action.channel = MIDI_CH_MELODY;
    action.number = columnCommands[row];
    action.label = columnLabels[row];
    return action;
  }

  if (row == 0 && col >= 5 && col <= 9) {
    static const CommandId topRowCommands[5] = {
        CMD_MENU,
        CMD_PLAY,
        CMD_STOP,
        CMD_REC,
        CMD_SETTINGS,
    };
    static const char *topRowLabels[5] = {"Menu", "Play", "Stop", "Rec", "Setup"};

    action.type = KeyAction::Command;
    action.channel = MIDI_CH_MELODY;
    action.number = topRowCommands[col - 5];
    action.label = topRowLabels[col - 5];
    return action;
  }

  // Zone Mélodie (2 rangées du bas)
  if (row >= 3 && row <= 4) {
    const uint8_t baseNote = 48; // C3
    const uint8_t index = (row - 3) * 10 + col;
    action.type = KeyAction::Note;
    action.channel = MIDI_CH_MELODY;
    action.number = baseNote + index;
    action.label = "Melodie";
    return action;
  }

  return action;
}
