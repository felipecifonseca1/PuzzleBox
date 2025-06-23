#include "NonBlockingTimer.h"

NonBlockingTimer::NonBlockingTimer() {
    _isRunning = false;
    _startTime = 0;
    _duration = 0;
}

void NonBlockingTimer::start(unsigned long duration_ms) {
    _duration = duration_ms;
    _startTime = millis();
    _isRunning = true;
}

void NonBlockingTimer::stop() {
    _isRunning = false;
}

bool NonBlockingTimer::isRunning() {
    return _isRunning;
}

bool NonBlockingTimer::hasExpired() {
    // Se o timer não está rodando, ele não pode ter expirado.
    if (!_isRunning) {
        return false;
    }

    // Verifica se o tempo decorrido é maior que a duração definida
    if (millis() - _startTime >= _duration) {
        _isRunning = false; // Para o timer para que ele só expire uma vez
        return true;
    }

    return false;
}