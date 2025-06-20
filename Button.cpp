// Button.cpp

#include "Button.h"

Button::Button(uint8_t pin) {
    _pin = pin;
    _ultimoTempoDebounce = 0;
    _delayDebounce = 50;
    _ultimoEstadoBotao = HIGH;
    _ultimoEstadoLeitura = HIGH;
}

void Button::inicializar() {
    pinMode(_pin, INPUT_PULLUP);
    _ultimoEstadoBotao = digitalRead(_pin);
    _ultimoEstadoLeitura = _ultimoEstadoBotao;
}

bool Button::foiPressionado() {
    bool estadoPressionado = false;
    int leituraAtual = digitalRead(_pin);

    if (leituraAtual != _ultimoEstadoLeitura) {
        _ultimoTempoDebounce = millis();
    }

    if ((millis() - _ultimoTempoDebounce) > _delayDebounce) {
        if (leituraAtual != _ultimoEstadoBotao) {
            _ultimoEstadoBotao = leituraAtual;
            if (_ultimoEstadoBotao == LOW) {
                estadoPressionado = true;
            }
        }
    }

    _ultimoEstadoLeitura = leituraAtual;
    return estadoPressionado;
}