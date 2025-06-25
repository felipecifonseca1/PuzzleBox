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
    // Uma fanfarra de vitória mais longa e elaborada
    int melodia[] = {
        // Frase 1: Arpejo ascendente
        NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5,
        // Frase 2: Repete com mais ritmo
        NOTE_C4, NOTE_E4, NOTE_C4, NOTE_E4,
        // Frase 3: Construindo a tensão
        NOTE_G4, NOTE_AS4, NOTE_C5, NOTE_G4,
        // Frase Final: Resolução triunfante
        NOTE_DS5, NOTE_DS5, NOTE_DS5, NOTE_C5
    };

    // Duração de cada nota em milissegundos
    int duracoes[] = {
        200, 200, 200, 400, // Frase 1
        150, 150, 150, 150, // Frase 2
        200, 200, 200, 200, // Frase 3
        200, 200, 200, 400, // Frase 1
        150, 150, 150, 150, // Frase 2
        200, 200, 200, 200, // Frase 3
        150, 150, 150, 500,  // Frase Final
        200, 200, 200, 400, // Frase 1
        150, 150, 150, 150, // Frase 2
        200, 200, 200, 200, // Frase 3
        200, 200, 200, 400, // Frase 1
        150, 150, 150, 150, // Frase 2
        200, 200, 200, 200, // Frase 3
        150, 150, 150, 500  // Frase Final
    };

    // O tamanho do array agora é 16
    int tamanhoDaMusica = 16;

    // Itera sobre as notas da melodia
    for (int i = 0; i < tamanhoDaMusica; i++) {
        // Pega a duração da nota atual
        int duracaoNota = duracoes[i];
        
        // Toca a nota
        tone(_pin, melodia[i], duracaoNota);

        int pausaEntreNotas = duracaoNota * 1.30;
        vTaskDelay(pdMS_TO_TICKS(pausaEntreNotas));

        noTone(_pin);
    }
}

void Buzzer::tocarBeepTecla() {
    tone(_pin, 1200, 50); 
}