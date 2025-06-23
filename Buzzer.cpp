#include "Buzzer.h"

// Implementação do construtor
Buzzer::Buzzer(uint8_t pin) {
    _pin = pin;
}

void Buzzer::inicializar() {
    pinMode(_pin, OUTPUT);
}

void Buzzer::tocarNota(int frequencia, int duracao) {
    // tone(pino, frequencia, duracao_em_ms)
    tone(_pin, frequencia, duracao);
}

void Buzzer::tocarBeepSucesso() {
    tone(_pin, 900, 150); // Tom agudo por 150ms
}

void Buzzer::tocarSomErro() {
    tone(_pin, 150, 800); // Tom grave por 800ms
}

void Buzzer::tocarSomVitoria() {
    // Toca uma sequência de notas curtas
    tone(_pin, NOTE_C4, 100);
    vTaskDelay(pdMS_TO_TICKS(120)); // Pausa entre as notas
    tone(_pin, NOTE_E4, 100);
    vTaskDelay(pdMS_TO_TICKS(120));
    tone(_pin, NOTE_G4, 100);
    vTaskDelay(pdMS_TO_TICKS(120));
    tone(_pin, NOTE_C4, 200); // Nota final mais longa
}

void Buzzer::tocarBeepTecla() {
    tone(_pin, 1200, 50); 
}