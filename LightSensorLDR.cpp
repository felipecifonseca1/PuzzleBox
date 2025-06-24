#include "LightSensorLDR.h"
#include "Arduino.h"

LightSensorLDR::LightSensorLDR(uint8_t pin, LdrCalculation calcMethod) {
    _pin = pin;
    _calcMethod = calcMethod;
    _luxMinimo = 0;
    _luxMaximo = 0;
}

void LightSensorLDR::inicializar() {
    pinMode(_pin, INPUT);
}

float LightSensorLDR::lerNivelDeLuz() {
    long totalAnalogValue = 0;
    for (int i = 0; i < NUM_LEITURAS; i++) {
        totalAnalogValue += analogRead(_pin);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    // Média de leituras de 12 bits 
    int analogValue12bit = totalAnalogValue / NUM_LEITURAS;

    float resistance = 0;
    float lux = 0;

    switch (_calcMethod) {
        case LdrCalculation::LDR_TO_GND: {
            float voltage = analogValue12bit / 4095.0 * 5.0;
            if (voltage >= 5.0) voltage = 4.99;
            resistance = (voltage * FIXED_RESISTOR) / (5.0 - voltage);
            break;
        }
            
        case LdrCalculation::LDR_TO_VCC: {
            float voltage = analogValue12bit / 4095.0 * 5.0;
            if (voltage <= 0) voltage = 0.01;
            resistance = FIXED_RESISTOR * (5.0 - voltage) / voltage;
            break;
        }

        case LdrCalculation::MANUFACTURER_EXAMPLE: {
          
            int analogValue10bit = analogValue12bit / 4;
            float voltage = analogValue10bit / 1024.0 * 5.0; //   
            // Proteção para evitar divisão por zero ou valores negativos
            if (voltage >= 5.0) voltage = 4.99;

            resistance = 2000.0 * voltage / (1.0 - voltage / 5.0); //
            break;
        }
    }

    if (resistance <= 0) resistance = 0.1;

    // A conversão final para Lux é a mesma para todos os métodos
    lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA)); //
    
    return lux;
}

void LightSensorLDR::definirFaixaDeLuzAlvo(float luxMinimo, float luxMaximo) {
    _luxMinimo = luxMinimo;
    _luxMaximo = luxMaximo;
}

bool LightSensorLDR::verificarLuzCorreta() {
    float luxAtual = lerNivelDeLuz();
    return (luxAtual >= _luxMinimo && luxAtual <= _luxMaximo);
}