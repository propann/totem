#pragma once

#include <Arduino.h>
#include <Encoder.h>
#include <Control_Surface.h>
#include <MIDI_Outputs/CCRotaryEncoder.hpp>

#include "config.h"

USING_CS_NAMESPACE;

class TotemMatrix {
  public:
    static constexpr uint8_t kRows = 5;
    static constexpr uint8_t kCols = 14;

    TotemMatrix();

    void begin();
    // Retourne true si une touche est pressée (transition OFF->ON) et remplit
    // noteName avec le nom humain (ex: C3).
    bool update(char *noteName, size_t len);

  private:
    bool scanMatrix(uint8_t &noteNumber);
    void noteNameFromNumber(uint8_t note, char *buf, size_t len) const;

    // Matrice MIDI (utilisée par Control Surface pour envoyer du MIDI si besoin)
    NoteButtonMatrix<kRows, kCols> matrix;
    CCRotaryEncoder enc1;
    CCRotaryEncoder enc2;
    CCRotaryEncoder enc3;
    CCRotaryEncoder enc4;

    bool prevState[kRows][kCols] = {};
};
