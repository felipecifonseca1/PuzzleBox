// Button.h

#ifndef BUTTON_H
#define BUTTON_H

#include "Arduino.h"

class Button {
public:
    Button(uint8_t pin);
    void inicializar();
    bool foiPressionado();

private:
    uint8_t _pin;
    unsigned long _ultimoTempoDebounce;
    unsigned long _delayDebounce;
    int _ultimoEstadoBotao;
    int _ultimoEstadoLeitura;
};

#endif