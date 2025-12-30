#ifndef SYNTH_ENGINE_H
#define SYNTH_ENGINE_H

#include <Audio.h>
#include <cmath>

// --- GRAPH AUDIO DU SLAVE ---
// Conception : 2 Oscillateurs -> Mixeur -> Enveloppe -> S/PDIF Out (Pin 14)
// C'est une architecture soustractive simple et efficace.

AudioSynthWaveform osc1; // Oscillateur A (Saw/Square)
AudioSynthWaveform osc2; // Oscillateur B (Sub/Sine)
AudioMixer4 oscMixer; // Mélange des 2 osc
AudioEffectEnvelope ampEnv; // Enveloppe d'amplitude (ADSR)
AudioOutputSPDIF3 spdifOut; // SORTIE NUMÉRIQUE (Pin 14)

// Connexions (Patch Cords)
AudioConnection patch1(osc1, 0, oscMixer, 0);
AudioConnection patch2(osc2, 0, oscMixer, 1);
AudioConnection patch3(oscMixer, 0, ampEnv, 0);
AudioConnection patch4(ampEnv, 0, spdifOut, 0); // Canal Gauche
AudioConnection patch5(ampEnv, 0, spdifOut, 1); // Canal Droite (Mono dupliqué)

// Fonction d'initialisation du moteur
void setupSynthEngine() {
    AudioMemory(20); // Allocation mémoire audio

    // Config Osc 1 (Son Principal)
    osc1.begin(0.4, 440, WAVEFORM_SAWTOOTH);

    // Config Osc 2 (Sub-Bass ou Renfort)
    osc2.begin(0.3, 220, WAVEFORM_SINE);

    // Config Enveloppe (Snappy, percussive)
    ampEnv.attack(10);
    ampEnv.decay(50);
    ampEnv.sustain(0.8);
    ampEnv.release(200);

    // Config Mixer
    oscMixer.gain(0, 0.8); // Gain Osc 1
    oscMixer.gain(1, 0.6); // Gain Osc 2
}

// Fonction pour jouer une note (Trigger)
void playSlaveNote(int note, int velocity) {
    if (velocity > 0) {
        float freq = 440.0f * powf(2.0f, (note - 69) / 12.0f);
        osc1.frequency(freq);
        osc2.frequency(freq * 0.5f); // Octave en dessous
        ampEnv.noteOn();
    } else {
        ampEnv.noteOff();
    }
}

#endif
