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

    bool inicializar();
    
    void atualizar();

    void definirInclinacaoAlvo(float targetRoll, float targetPitch, float tolerancia);

    bool verificarInclinacaoCorreta();

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