#ifndef LIGHT_SENSOR_LDR_H
#define LIGHT_SENSOR_LDR_H

#include "Arduino.h"

class LightSensorLDR {
public:
    enum class LdrCalculation {
       
        LDR_TO_GND,
        LDR_TO_VCC,
        MANUFACTURER_EXAMPLE
    };

    // O construtor aceita a escolha do método de cálculo.
    LightSensorLDR(uint8_t pin, LdrCalculation calcMethod);

    void inicializar();
    float lerNivelDeLuz();
    void definirFaixaDeLuzAlvo(float luxMinimo, float luxMaximo);
    bool verificarLuzCorreta();

private:
    uint8_t _pin;
    float _luxMinimo;
    float _luxMaximo;
    LdrCalculation _calcMethod; // Guarda o método de cálculo escolhido

    // Constantes para os modos padrão
    const float FIXED_RESISTOR = 10000.0;
    
    // Constantes do LDR
    const float GAMMA = 0.7; 
    const float RL10 = 50; 
    const int NUM_LEITURAS = 10;
};

#endif