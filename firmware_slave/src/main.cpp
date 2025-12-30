#include <Arduino.h>
#include <U8g2lib.h>
#include <Control_Surface.h>
#include <AH/Hardware/ButtonMatrix.hpp>

#include "config.h"

USING_CS_NAMESPACE;

// ---------------------------------------------------------------------------
// Configuration matrice (ordre exact demandé pour le test)
// ---------------------------------------------------------------------------
constexpr uint8_t ROWS = 5;
constexpr uint8_t COLS = 10;
const uint8_t rowPins[ROWS] = {24, 23, 34, 35, 28};
const uint8_t colPins[COLS] = {9, 8, 7, 4, 3, 2, 37, 33, 25, 38};

// Ecran OLED (I2C 18/19)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE,
                                         /* clock=*/ OLED_SCL, /* data=*/ OLED_SDA);

// Sous-classe ButtonMatrix pour capter les événements
class DebugMatrix : public AH::ButtonMatrix<ROWS, COLS> {
  public:
    DebugMatrix(const uint8_t (&rows)[ROWS], const uint8_t (&cols)[COLS])
        : AH::ButtonMatrix<ROWS, COLS>(rows, cols) {}

  private:
    void onButtonChanged(uint8_t row, uint8_t col, bool state) override {
        if (state == LOW) { // appui détecté
            const uint8_t physPin = colPins[col];
            Serial.print(F("--- TOUCHE DETECTEE --- | Ligne: "));
            Serial.print(row);
            Serial.print(F(" | Colonne: "));
            Serial.print(col);
            Serial.print(F(" | Pin Physique: "));
            Serial.println(physPin);

            if (physPin == 33 || physPin == 37 || physPin == 38) {
                Serial.println(F("[OK] SIGNAL RECU SUR PIN DE REPARATION"));
            }

            // Mise à jour de l'écran
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_6x12_tr);
            u8g2.drawStr(0, 12, "CONSOLE DEBUG READY");
            char line[24];
            snprintf(line, sizeof(line), "Key: [%u][%u]", row, col);
            u8g2.drawStr(0, 28, line);
            u8g2.sendBuffer();
        }
    }
};

DebugMatrix matrix(rowPins, colPins);

void setup() {
    Serial.begin(115200);
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(0, 12, "CONSOLE DEBUG READY");
    u8g2.sendBuffer();

    matrix.begin();
}

void loop() {
    // Le ButtonMatrix d'AH (Control_Surface) gère le scan + debounce en interne
    matrix.update();
    // Pas besoin d'appeler Control_Surface.loop() ici, on ne génère pas de MIDI
}
