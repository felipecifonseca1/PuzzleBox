#include "Buzzer.h"

Buzzer::Buzzer(uint8_t pin) {
    _pin = pin;
}

void Buzzer::inicializar() {
    pinMode(_pin, OUTPUT);
}

void Buzzer::tocarBeepSucesso() {
    // tone(pino, frequencia, duracao_em_ms)
    tone(_pin, 900, 150); // Tom agudo por 150ms
}

void Buzzer::tocarSomErro() {
    tone(_pin, 150, 800); // Tom grave por 800ms
}

// Implementação da melodia de vitória
void Buzzer::tocarSomVitoria() {
    tone(_pin, NOTE_E4, 100);
    delay(120);
    tone(_pin, NOTE_G4, 100);
    delay(120);
    tone(_pin, NOTE_C4, 200); 
}