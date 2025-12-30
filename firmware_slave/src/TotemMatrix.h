#pragma once

#include <Arduino.h>
#include <Encoder.h>
#include <Control_Surface.h>
#include <Encoder.h>

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
    bool pollEncoder(uint8_t &id, long &value);

  private:
    bool scanMatrix(uint8_t &noteNumber);
    void noteNameFromNumber(uint8_t note, char *buf, size_t len) const;

    // Matrice MIDI (utilisée par Control Surface pour envoyer du MIDI si besoin)
    NoteButtonMatrix<kRows, kCols> matrix;
    Encoder enc1;
    Encoder enc2;
    Encoder enc3;
    Encoder enc4;

    bool prevState[kRows][kCols] = {};
    long encLast[4] = {};
};
