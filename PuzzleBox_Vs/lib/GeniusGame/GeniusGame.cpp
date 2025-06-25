#include "GeniusGame.h"

// Constantes para controlar o tempo do pisca-pisca 
const int LED_ON_DURATION = 500;  // Duração que o LED fica aceso
const int LED_OFF_DURATION = 250; // Pausa entre os LEDs

GeniusGame::GeniusGame(const int ledPins[GENIUS_SEQUENCE_LENGTH], const int buttonPins[GENIUS_SEQUENCE_LENGTH], DisplayOLED& display, Buzzer& buzzer)
    : _display(display), _buzzer(buzzer) {
    for (int i = 0; i < GENIUS_SEQUENCE_LENGTH; i++) {
        _ledPins[i] = ledPins[i];
        _buttonPins[i] = buttonPins[i];
    }
}

void GeniusGame::inicializar() {
    for (int i = 0; i < GENIUS_SEQUENCE_LENGTH; i++) {
        pinMode(_ledPins[i], OUTPUT);
        digitalWrite(_ledPins[i], LOW);
        pinMode(_buttonPins[i], INPUT_PULLUP);
    }
}

void GeniusGame::_gerarSequencia() {
    randomSeed(esp_random()); 

    for (int i = 0; i < GENIUS_SEQUENCE_LENGTH; i++) {
        _sequencia[i] = i;
    }
    for (int i = GENIUS_SEQUENCE_LENGTH - 1; i > 0; i--) {
        int j = random(i + 1);
        int temp = _sequencia[i];
        _sequencia[i] = _sequencia[j];
        _sequencia[j] = temp;
    }
}

void GeniusGame::iniciarNovoJogo() {
    _gerarSequencia();
    _passoAtualJogador = 0;
    _passoAtualShow = 0;
    _estadoAtual = MOSTRANDO_LED; 
    _ultimoTempo = millis();
}

int GeniusGame::_lerBotoes() {
    for (int i = 0; i < GENIUS_SEQUENCE_LENGTH; i++) {
        if (digitalRead(_buttonPins[i]) == LOW) {
            vTaskDelay(pdMS_TO_TICKS(50)); 
            while(digitalRead(_buttonPins[i]) == LOW); 
            return i;
        }
    }
    return -1;
}

bool GeniusGame::isJogoFinalizado() {
    return _estadoAtual == VITORIA;
}

void GeniusGame::loop() {
    switch (_estadoAtual) {
        case MOSTRANDO_LED: {
            if (_passoAtualShow < GENIUS_SEQUENCE_LENGTH) {
                if (millis() - _ultimoTempo > LED_OFF_DURATION) {
                    int ledIndex = _sequencia[_passoAtualShow];
                    int notes[] = {262, 330, 392};

                    digitalWrite(_ledPins[ledIndex], HIGH);
                    _buzzer.tocarNota(notes[ledIndex],100);               
                    _ultimoTempo = millis();
                    _estadoAtual = MOSTRANDO_PAUSA;
                }
            } else {
                _estadoAtual = AGUARDANDO_JOGADOR;
                _display.exibirMensagem("Genius:", "Sua vez!", 2);
            }
            break;
        }

        case MOSTRANDO_PAUSA: {
            if (millis() - _ultimoTempo > LED_ON_DURATION) {
                int ledIndex = _sequencia[_passoAtualShow];
                digitalWrite(_ledPins[ledIndex], LOW);
                noTone(_buzzer.getPin()); 
        
                _passoAtualShow++;
                _ultimoTempo = millis();
                _estadoAtual = MOSTRANDO_LED;
            }
            break;
        }

        case AGUARDANDO_JOGADOR: {
            int botaoPressionado = _lerBotoes();
            if (botaoPressionado != -1) {
                int notes[] = {262, 330, 392};
                digitalWrite(_ledPins[botaoPressionado], HIGH);
                _buzzer.tocarNota(notes[botaoPressionado],100);
                vTaskDelay(pdMS_TO_TICKS(150)); 
                digitalWrite(_ledPins[botaoPressionado], LOW);
                noTone(_buzzer.getPin());
                
                if (botaoPressionado == _sequencia[_passoAtualJogador]) {
                    _passoAtualJogador++;
                    if (_passoAtualJogador >= GENIUS_SEQUENCE_LENGTH) {
                        _estadoAtual = VITORIA;
                    }
                } else {
                    _estadoAtual = DERROTA;
                }
            }
            break;
        }

        case VITORIA:
            _display.exibirMensagem("Genius:", "VITORIA!", 2);
            _estadoAtual = OCIOSO; 
            break;

        case DERROTA: {
            _display.exibirMensagem("Genius:", "ERROU!", 2);
            _buzzer.tocarSomErro();
            vTaskDelay(pdMS_TO_TICKS(1500)); 
            iniciarNovoJogo(); 
            break;
        }

        case OCIOSO:
            break;
    }
}