// AccelerometerMPU6050.h

#ifndef ACCELEROMETER_MPU6050_H
#define ACCELEROMETER_MPU6050_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AHRS.h> // Usaremos a fusão de sensores para melhor resultado
#include <Wire.h>

class AccelerometerMPU6050 {
public:
    // Construtor
    AccelerometerMPU6050();

    // Inicializa o sensor. Retorna 'true' se for bem-sucedido.
    bool inicializar();
    
    // Novo método que deve ser chamado repetidamente no loop() principal
    void atualizar();

    // Define a inclinação alvo em ângulos (Roll e Pitch)
    void definirInclinacaoAlvo(float targetRoll, float targetPitch, float tolerancia);

    // Verifica se a inclinação (orientação) atual está correta
    bool verificarInclinacaoCorreta();

    // Imprime os ângulos de orientação para debug
    void imprimirLeituras();
    float getRoll();
    float getPitch();

private:
    Adafruit_MPU6050 _mpu;
    Adafruit_Madgwick _filter;

    // Variáveis para guardar os ângulos calculados
    float _roll, _pitch;
    
    // Variáveis para os ângulos alvo
    float _targetRoll, _targetPitch;
    float _tolerance;
};

#endif