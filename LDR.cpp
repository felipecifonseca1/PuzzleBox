#include "LightSensorLDR.h"
#include "Arduino.h"

// Construtor
LightSensorLDR::LightSensorLDR(uint8_t pin) {
    _pin = pin;
    _luxMinimo = 0;
    _luxMaximo = 0;
}

// Inicializa o pino do sensor
void LightSensorLDR::inicializar() {
    pinMode(_pin, INPUT);
}

// Implementação da leitura e conversão para Lux
float LightSensorLDR::lerNivelDeLuz() {
    // Lê o valor analógico do pino do sensor.
    int analogValue = analogRead(_pin);

    // Converte o valor analógico para tensão.
    // Usa as constantes VOLTAGE_REFERENCE e ADC_RESOLUTION definidas no .h
    float voltage = analogValue / (float)ADC_RESOLUTION * VOLTAGE_REFERENCE;

    // Calcula a resistência do LDR usando a fórmula do divisor de tensão.
    // Nota: Se `voltage` se aproximar de `VOLTAGE_REFERENCE`, a resistência pode tender ao infinito.
    if (voltage >= VOLTAGE_REFERENCE) {
        voltage = VOLTAGE_REFERENCE - 0.01; // Evita divisão por zero
    }
    float resistance = (voltage * FIXED_RESISTOR) / (VOLTAGE_REFERENCE - voltage);

    // Converte a resistência para lux usando a fórmula de referência
    float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));

    return lux;
}

// Define a faixa de luz alvo
void LightSensorLDR::definirFaixaDeLuzAlvo(float luxMinimo, float luxMaximo) {
    _luxMinimo = luxMinimo;
    _luxMaximo = luxMaximo;
}

// Verifica se a luz está na faixa correta
bool LightSensorLDR::verificarLuzCorreta() {
    float luxAtual = lerNivelDeLuz();
    return (luxAtual >= _luxMinimo && luxAtual <= _luxMaximo);
}