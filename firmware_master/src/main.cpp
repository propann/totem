#include <Arduino.h>
#include "AudioGraph.h"

void setup() {
  Serial.begin(2000000);
  AudioGraph::init();
}

void loop() {
  AudioGraph::update();
}
