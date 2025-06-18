#include "AccelerometerMPU6050.h"
#include "Arduino.h"

// Construtor
AccelerometerMPU6050::AccelerometerMPU6050() : _filter() {
}

// Método de inicialização
bool AccelerometerMPU6050::inicializar() {
    if (!_mpu.begin()) {
        Serial.println("Falha ao encontrar o sensor MPU6050.");
        return false;
    }
    Serial.println("Sensor MPU6050 encontrado.");

    _mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    _mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    _mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    
    // Inicia o filtro de fusão
    _filter.begin(100);

    return true;
}

// Método para atualizar o filtro com novos dados dos sensores
void AccelerometerMPU6050::atualizar() {
    sensors_event_t a, g, temp;
    _mpu.getEvent(&a, &g, &temp);

    _filter.update(g.gyro.x, g.gyro.y, g.gyro.z, 
               a.acceleration.x, a.acceleration.y, a.acceleration.z, 
               0, 0, 0); // Como nao tem mag valores sao 0

    _roll = _filter.getRoll();
    _pitch = _filter.getPitch();
}

// Define a inclinação alvo em graus
void AccelerometerMPU6050::definirInclinacaoAlvo(float targetRoll, float targetPitch, float tolerancia) {
    _targetRoll = targetRoll;
    _targetPitch = targetPitch;
    _tolerance = tolerancia;
}

// Verifica se a inclinação atual corresponde ao alvo
bool AccelerometerMPU6050::verificarInclinacaoCorreta() {
    bool rollOk = abs(_roll - _targetRoll) < _tolerance;
    bool pitchOk = abs(_pitch - _targetPitch) < _tolerance;
    return rollOk && pitchOk;
}

// Imprime os ângulos de Roll e Pitch
void AccelerometerMPU6050::imprimirLeituras() {
    Serial.print("Orientacao (Graus) - ");
    Serial.print("Roll: ");
    Serial.print(_roll, 2);
    Serial.print(", Pitch: ");
    Serial.print(_pitch, 2);
    Serial.println();
}