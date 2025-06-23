// Buzzer.cpp

#include "Buzzer.h"

// Implementação do construtor
Buzzer::Buzzer(uint8_t pin) {
    _pin = pin;
}

// Implementação do método inicializar
void Buzzer::inicializar() {
    pinMode(_pin, OUTPUT);
}

void Buzzer::tocarNota(int frequencia, int duracao) {
    // tone(pino, frequencia, duracao_em_ms)
    tone(_pin, frequencia, duracao);
}

// Implementação do beep de sucesso
void Buzzer::tocarBeepSucesso() {
    // A função tone() gera uma onda quadrada na frequência especificada.
    // tone(pino, frequencia, duracao_em_ms)
    tone(_pin, 900, 150); // Tom agudo por 150ms
}

// Implementação do som de erro
void Buzzer::tocarSomErro() {
    tone(_pin, 150, 800); // Tom grave por 800ms
}

// Implementação da melodia de vitória
void Buzzer::tocarSomVitoria() {
    // Toca uma sequência de notas curtas
    tone(_pin, NOTE_C4, 100);
    delay(120); // Pausa entre as notas
    tone(_pin, NOTE_E4, 100);
    delay(120);
    tone(_pin, NOTE_G4, 100);
    delay(120);
    tone(_pin, NOTE_C4, 200); // Nota final mais longa
    // noTone(_pin) não é estritamente necessário aqui porque a próxima
    // chamada a tone() irá sobrescrever a anterior, mas o delay cria a pausa.
}

void Buzzer::tocarBeepTecla() {
    tone(_pin, 1200, 50); // Tom agudo e bem curto (50ms)
}