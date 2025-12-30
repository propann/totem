// ⚠️ AVERTISSEMENT CÂBLAGE TOTEM ⚠️
// Matrice modifiée physiquement :
// - Col 6 -> Pin 37
// - Col 7 -> Pin 33
// - Col 9 -> Pin 38
// Ne JAMAIS utiliser les pins 0, 1 ou 14 pour la matrice.

#include "Slave_Matrix_Controller.h"

using namespace CS;

namespace {
constexpr uint32_t kFastMidiBaud = 2000000;

// Définition CORRIGÉE pour le Mod Totem (Fly-wires)
// Ne JAMAIS remettre 0, 1 ou 14 ici !
const PinList<TotemMatrix::kCols> kColPins = {
    9, 8, 7, 4, 3, 2, // Cols 0-5
    37,              // Col 6 (Remplace TX/1)
    33,              // Col 7 (Remplace RX/0)
    25,              // Col 8
    38,              // Col 9 (Remplace S/PDIF/14)
};

const PinList<TotemMatrix::kRows> kRowPins = {24, 23, 34, 35, 28};

// Sous-ensembles pour les différentes zones.
const PinList<4> kRowPinsTop = {24, 23, 34, 35};
const PinList<1> kRowPinsBottom = {28};

const PinList<4> kColPinsDrums = {9, 8, 7, 4};
const PinList<10> kColPinsMelody = {9, 8, 7, 4, 3, 2, 37, 33, 25, 38};
const PinList<6> kColPinsCommands = {3, 2, 37, 33, 25, 38};

const AddressMatrix<4, 4> kDrumNotes = {
    {36, 37, 38, 39},
    {40, 41, 42, 43},
    {44, 45, 46, 47},
    {48, 49, 50, 51},
};

const AddressMatrix<1, 10> kMelodyNotes = {
    {48, 49, 50, 51, 52, 53, 54, 55, 56, 57},
};

// Notes système pour commandes (Transport / Utilitaires).
// Exemple demandé : ROW_1, COL_4 -> 118 (Play).
const AddressMatrix<4, 6> kCommandNotes = {
    {112, 113, 114, 115, 116, 117},
    {118, 119, 120, 121, 122, 123},
    {106, 107, 108, 109, 110, 111},
    {100, 101, 102, 103, 104, 105},
};

HardwareSerialMIDI_Interface midi{Serial1, kFastMidiBaud};
} // namespace

const PinList<TotemMatrix::kRows> TotemMatrix::rowPins = kRowPins;
const PinList<TotemMatrix::kCols> TotemMatrix::colPins = kColPins;

// Tableau global d'adressage (notes) pour documentation et debug.
const AddressMatrix<TotemMatrix::kRows, TotemMatrix::kCols>
    TotemMatrix::addressMatrix = {
        // Cols: 0    1    2    3    | 4    5    6    7    8    9
        {36, 37, 38, 39, 112, 113, 114, 115, 116, 117}, // Row 0 (haut)
        {40, 41, 42, 43, 118, 119, 120, 121, 122, 123}, // Row 1
        {44, 45, 46, 47, 106, 107, 108, 109, 110, 111}, // Row 2
        {48, 49, 50, 51, 100, 101, 102, 103, 104, 105}, // Row 3
        {48, 49, 50, 51, 52,  53,  54,  55,  56,  57}, // Row 4 (bas)
    };

TotemMatrix::TotemMatrix()
    : drumsMatrix(kRowPinsTop, kColPinsDrums, kDrumNotes, {Channel_10, Cable_1}),
      melodyMatrix(kRowPinsBottom, kColPinsMelody, kMelodyNotes,
                   {Channel_1, Cable_1}),
      commandMatrix(kRowPinsTop, kColPinsCommands, kCommandNotes,
                    {Channel_1, Cable_1}) {}

void TotemMatrix::begin() {
    // MIDI série ultra-rapide sur Serial1.
    midi.begin();
    Control_Surface.begin();
}

void TotemMatrix::update() { Control_Surface.loop(); }
