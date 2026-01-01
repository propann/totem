# Documentation matérielle Totem Slave (Hack Easy‑Wire)

⚠️ Spécifique au PCB modifié (ne pas flasher sur un LMN‑3 stock).

## Câblage principal
- MCU : Teensy 4.1
- OLED : 2× SSD1306 I2C, SDA=18, SCL=19, adresses 0x3C (gauche) et 0x3D (droite).
- Joystick : Axe X sur A15.
- Encoders :
  - E1 : 5 / 6
  - E2 : 29 / 30
  - E3 (menu) : 31 / 32
  - E4 : 26 / 27
- Boutons d’encoders (codes MIDI) : 20, 21, 22, 23.

## Matrice “Easy‑Wire” (5×14)
Lignes : 24, 23, 34, 35, 28  
Colonnes : 9, 8, 7, 4, 3, 2, **16**, **17**, 25, **33**, 13, 41, 40, 36  
Hacks : Pin1→16, Pin0→17, Pin14→33. Pinout reflété dans `config.h`.

### Mapping logique actuel (`midiMap` du firmware)
- Row0 (CC) : LOOP / LOOP IN / LOOP OUT (cols 3‑5), encoders btn 1/2 (cols 9‑10), encoders btn 3/4 (cols 12‑13).
- Row1 (CC) : CUT / PASTE / SLICE / SAVE / UNDO (cols 3‑7).
- Row2 (CC) : CTRL / REC / PLAY / STOP / SETTINGS / TEMPO / MIXER / TRACKS / PLUGINS / MODIFIERS / SEQUENCERS (cols 3‑13).
- Row3 (Notes) : 52,54,56,58,59,61,63,64,66,68,70,71,73,75.
- Row4 (Notes) : 53,55,57,59,60,62,64,65,67,69,71,72,74,76.
- Les cases non utilisées envoient `DUMMY` (31) mais sont nommées en note pour le debug.

## Interaction firmware (rappel)
- Menu : navigation par encodeur 3, validation par clic court sur Row0/Col13 (btn enc3), sortie des modes Test/Info par appui long sur la même touche.
- Test mode : affiche Row/Col/Pin/CC+nom et log série, joystick sur écran gauche, encoders sur écran droit.
- Liaison Maître : Serial1 à 2 Mbaud, handshake READY/ACK non bloquant au boot.

## Fichiers clés
- `src/slave/config.h` : définitions pins et CC.
- `src/main.cpp` : logique menu/test/affichage + handshake maître.
