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

    static const CS::PinList<kRows> rowPins;
    static const CS::PinList<kCols> colPins;

    // Tableau global d'adressage (notes système incluses pour commandes).
    static const CS::AddressMatrix<kRows, kCols> addressMatrix;

    TotemMatrix();
    void begin();
    void update();

  private:
    // 4x4 en haut à gauche : drums (Channel 10)
    CS::NoteButtonMatrix<4, 4> drumsMatrix;
    // Ligne du bas : mélodie (Channel 1)
    CS::NoteButtonMatrix<1, 10> melodyMatrix;
    // Zone commandes (haut droite) : notes système (Channel 1)
    CS::NoteButtonMatrix<4, 6> commandMatrix;
};
