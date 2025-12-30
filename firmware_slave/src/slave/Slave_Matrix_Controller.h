#pragma once

// ⚠️ AVERTISSEMENT CÂBLAGE TOTEM ⚠️
// Matrice modifiée physiquement :
// - Col 6 -> Pin 37
// - Col 7 -> Pin 33
// - Col 9 -> Pin 38
// Ne JAMAIS utiliser les pins 0, 1 ou 14 pour la matrice.

#include <Control_Surface.h>

class TotemMatrix {
  public:
    static constexpr uint8_t kRows = 5;
    static constexpr uint8_t kCols = 10;

    static const cs::PinList<kRows> rowPins;
    static const cs::PinList<kCols> colPins;

    // Tableau global d'adressage (notes système incluses pour commandes).
    static const cs::AddressMatrix<kRows, kCols> addressMatrix;

    TotemMatrix();
    void begin();
    void update();

  private:
    // 4x4 en haut à gauche : drums (Channel 10)
    cs::NoteButtonMatrix<4, 4> drumsMatrix;
    // Ligne du bas : mélodie (Channel 1)
    cs::NoteButtonMatrix<1, 10> melodyMatrix;
    // Zone commandes (haut droite) : notes système (Channel 1)
    cs::NoteButtonMatrix<4, 6> commandMatrix;
};
