#ifndef LIGHT_SENSOR_LDR_H
#define LIGHT_SENSOR_LDR_H

#include "Arduino.h"

class LightSensorLDR {
public:
    // Construtor: Requer o pino analógico ao qual o LDR está conectado.
    LightSensorLDR(uint8_t pin);

    // Método de inicialização do sensor.
    void inicializar();

    // Lê o valor do sensor e o converte para lux.
    float lerNivelDeLuz();

    // Define a faixa de lux alvo para o puzzle.
    void definirFaixaDeLuzAlvo(float luxMinimo, float luxMaximo);

    // Verifica se a leitura atual está dentro da faixa alvo.
    bool verificarLuzCorreta();

private:
    uint8_t _pin; // Pino analógico do sensor LDR.
    float _luxMinimo; // Valor mínimo da faixa de lux.
    float _luxMaximo; // Valor máximo da faixa de lux.

    // --- Constantes para o cálculo de Lux ---
    // Valores ajustados para ESP32 (12 bits), operando com 5V e resistor de 10k Ohm.

    // A resolução do ADC do ESP32 é de 12 bits (0-4095).
    const int ADC_RESOLUTION = 4095;
    const float VOLTAGE_REFERENCE = 5.0;

    // Resistor fixo em série com o LDR (em Ohms). Ajustado para 10k Ohm.
    const float FIXED_RESISTOR = 10000.0; 
    
    // Constantes do LDR (baseadas na imagem).
    const float GAMMA = 0.7;
    const float RL10 = 50;
};

#endif