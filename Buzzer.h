#ifndef BUZZER_H
#define BUZZER_H

#include "Arduino.h"

// Algumas notas musicais
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_G4  392
#define NOTE_A4  440

class Buzzer {
public:
    // Construtor: 
    Buzzer(uint8_t pin);
    void inicializar();
    void tocarBeepSucesso();
    void tocarSomErro();
    void tocarSomVitoria();

private:
    uint8_t _pin; // Pino do buzzer
};

#endif