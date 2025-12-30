#pragma once

// ⚠️ AVERTISSEMENT CÂBLAGE TOTEM ⚠️
// Matrice modifiée physiquement :
// - Col 6 -> Pin 37
// - Col 7 -> Pin 33
// - Col 9 -> Pin 38
// Ne JAMAIS utiliser les pins 0, 1 ou 14 pour la matrice.

#include <Control_Surface.h>
USING_CS_NAMESPACE;

class TotemMatrix {
  public:
    static constexpr uint8_t kRows = 5;
    static constexpr uint8_t kCols = 10;

    TotemMatrix();
    void begin();
    void update();

    const char *getNoteName(uint8_t note);

    const CS::AddressMatrix<kRows, kCols> &addresses() const { return addressMatrix; }

  private:
    static const CS::PinList<kRows> rowPins;
    static const CS::PinList<kCols> colPins;
    static const CS::AddressMatrix<kRows, kCols> addressMatrix;

    CS::NoteButtonMatrix<4, 4> drums;
    CS::NoteButtonMatrix<1, 10> melodyBottom;
    CS::NoteButtonMatrix<1, 10> melodyTop;
};
