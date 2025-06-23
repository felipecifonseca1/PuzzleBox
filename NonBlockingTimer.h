#ifndef NON_BLOCKING_TIMER_H
#define NON_BLOCKING_TIMER_H

#include <Arduino.h>

class NonBlockingTimer {
public:
    // Construtor
    NonBlockingTimer();

    void start(unsigned long duration_ms);
    void stop();
    bool hasExpired();
    bool isRunning();

private:
    unsigned long _startTime;
    unsigned long _duration;
    bool _isRunning;
};

#endif