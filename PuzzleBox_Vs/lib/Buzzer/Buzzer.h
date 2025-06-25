#ifndef BUZZER_H
#define BUZZER_H

#include "Arduino.h"

// Definições de frequências para notas musicais (em Hz)
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_C5  523
#define NOTE_GS4 415 
#define NOTE_AS4 466 
#define NOTE_CS5 554 
#define NOTE_DS5 622 

class Buzzer {
public:
    Buzzer(uint8_t pin);

    void inicializar();

    void tocarBeepSucesso();

    void tocarSomErro();

    void tocarSomVitoria();

    void tocarBeepTecla();
    
    void tocarNota(int frequencia, int duracao);

    int getPin();

private:
    uint8_t _pin; // Pino do buzzer
};

#endif