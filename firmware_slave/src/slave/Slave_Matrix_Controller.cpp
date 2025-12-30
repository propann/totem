// ⚠️ AVERTISSEMENT CÂBLAGE TOTEM ⚠️
// Matrice modifiée physiquement :
// - Col 6 -> Pin 37
// - Col 7 -> Pin 33
// - Col 9 -> Pin 38
// Ne JAMAIS utiliser les pins 0, 1 ou 14 pour la matrice.

#include "Slave_Matrix_Controller.h"

#include "../config.h"

// Définition CORRIGÉE pour le Mod Totem (Fly-wires)
// Ne JAMAIS remettre 0, 1 ou 14 ici !
const CS::PinList<TotemMatrix::kCols> TotemMatrix::colPins = {
    COL_0, COL_1, COL_2, COL_3, COL_4, COL_5,
    COL_6, // Pin 37 (ex-TX1 isolée)
    COL_7, // Pin 33 (ex-RX1 isolée)
    COL_8,
    COL_9, // Pin 38 (ex-SPDIF isolée)
};
const CS::PinList<TotemMatrix::kRows> TotemMatrix::rowPins = {ROW_0, ROW_1, ROW_2, ROW_3, ROW_4};

// Mapping notes : drums (4x4) en canal 10, reste en canal 1.
const CS::AddressMatrix<TotemMatrix::kRows, TotemMatrix::kCols>
    TotemMatrix::addressMatrix = {{
        {36, 37, 38, 39, 112, 113, 114, 115, 116, 117}, // Row 0
        {40, 41, 42, 43, 118, 119, 120, 121, 122, 123}, // Row 1
        {44, 45, 46, 47, 106, 107, 108, 109, 110, 111}, // Row 2
        {48, 49, 50, 51, 52, 53, 54, 55, 56, 57},       // Row 3
        {58, 59, 60, 61, 62, 63, 64, 65, 66, 67},       // Row 4
    }};

const CS::AddressMatrix<4, 4> kDrumNotes = {{
    {36, 37, 38, 39},
    {40, 41, 42, 43},
    {44, 45, 46, 47},
    {48, 49, 50, 51},
}};

const CS::AddressMatrix<1, 10> kMelodyBottom = {{
    {48, 49, 50, 51, 52, 53, 54, 55, 56, 57},
}};

const CS::AddressMatrix<1, 10> kMelodyTop = {{
    {58, 59, 60, 61, 62, 63, 64, 65, 66, 67},
}};

TotemMatrix::TotemMatrix()
    : drums({ROW_0, ROW_1, ROW_2, ROW_3}, {COL_0, COL_1, COL_2, COL_3},
            kDrumNotes,
            CHANNEL_10),
      melodyBottom({ROW_3}, colPins, kMelodyBottom, CHANNEL_1),
      melodyTop({ROW_4}, colPins, kMelodyTop, CHANNEL_1) {}

void TotemMatrix::begin() { Control_Surface.begin(); }

void TotemMatrix::update() { Control_Surface.loop(); }

const char *TotemMatrix::getNoteName(uint8_t note) {
    static char name[8];
    static const char *names[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                                    "F#", "G",  "G#", "A",  "A#", "B"};
    const uint8_t idx = note % 12;
    const int octave = static_cast<int>(note / 12) - 1;
    snprintf(name, sizeof(name), "%s%d", names[idx], octave);
    return name;
}
