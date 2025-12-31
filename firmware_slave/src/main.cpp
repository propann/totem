#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// Définition des deux écrans (Mode Hardware I2C, Pas de Reset)
// Note : On utilise le même constructeur pour les deux
U8G2_SSD1306_128X64_NONAME_F_HW_I2C leftEye(U8G2_R0, U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C rightEye(U8G2_R0, U8X8_PIN_NONE);

void setup() {
  // 1. Démarrage Série pour le debug
  Serial.begin(115200);
  delay(1000);
  Serial.println("--- TEST DUAL SCREEN ---");

  // 2. Initialisation Écran GAUCHE (0x3C)
  Serial.print("Init Ecran Gauche (0x3C)... ");
  leftEye.setI2CAddress(0x3C * 2); // U8g2 veut l'adresse * 2 (soit 0x78)
  leftEye.begin();
  leftEye.clearBuffer();
  leftEye.setFont(u8g2_font_ncenB14_tr);
  leftEye.drawStr(10, 40, "GAUCHE");
  leftEye.sendBuffer();
  Serial.println("OK !");

  delay(500); // Petite pause de sécurité

  // 3. Initialisation Écran DROIT (0x3D)
  Serial.print("Init Ecran Droit (0x3D)... ");
  rightEye.setI2CAddress(0x3D * 2); // U8g2 veut l'adresse * 2 (soit 0x7A)
  rightEye.begin();
  rightEye.clearBuffer();
  rightEye.setFont(u8g2_font_ncenB14_tr);
  rightEye.drawStr(10, 40, "DROITE");
  rightEye.sendBuffer();
  Serial.println("OK !");
}

void loop() {
  // Faire clignoter pour prouver que ce n'est pas planté
  leftEye.setPowerSave(0);
  rightEye.setPowerSave(0);
  delay(500);
  // leftEye.setPowerSave(1); // Décommenter pour tester le clignotement
  // rightEye.setPowerSave(1);
  // delay(500);
}
