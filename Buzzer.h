// Buzzer.h

#ifndef BUZZER_H
#define BUZZER_H

#include "Arduino.h"

// Definições de frequências para notas musicais (em Hz)
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_G4  392
#define NOTE_A4  440

class Buzzer {
public:
    // Construtor: recebe o pino ao qual o buzzer está conectado
    Buzzer(uint8_t pin);

    // Método para inicializar o buzzer (deve ser chamado no setup)
    void inicializar();

    // Toca um beep curto para feedback positivo (ex: nível concluído)
    void tocarBeepSucesso();

    // Toca um som grave e longo para feedback de erro (ex: código incorreto)
    void tocarSomErro();

    // Toca uma pequena melodia de vitória para quando o puzzle for resolvido
    void tocarSomVitoria();

    void tocarBeepTecla();
    
    void tocarNota(int frequencia, int duracao);

private:
    uint8_t _pin; // Pino do buzzer
};

#endif