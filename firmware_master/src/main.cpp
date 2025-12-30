/*
 * PROJET TOTEM - FIRMWARE MAITRE (ENGINE)
 * Based on the LMN-3 Project by fundamental.frequency.
 */

#include <Arduino.h>
#include "AudioGraph.h"

void setup() {
  Serial.begin(2000000);
  AudioGraph::init();
}

void loop() {
  AudioGraph::update();
}
