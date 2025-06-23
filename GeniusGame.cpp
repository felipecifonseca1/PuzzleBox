#include "GeniusGame.h"

// Constantes para controlar o tempo do pisca-pisca (em milissegundos)
const int LED_ON_DURATION = 600; 
const int LED_OFF_DURATION = 200; 

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
    randomSeed(analogRead(35)); 
}

void GeniusGame::_gerarSequencia() {
    for (int i = 0; i < GENIUS_SEQUENCE_LENGTH; i++) _sequencia[i] = i;
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
    _display.exibirMensagem("Genius:", "Observe...", 2);
    delay(500); 
}

int GeniusGame::_lerBotoes() {
    for (int i = 0; i < GENIUS_SEQUENCE_LENGTH; i++) {
        if (digitalRead(_buttonPins[i]) == LOW) {
            delay(500); 
            while(digitalRead(_buttonPins[i]) == LOW); 
            return i;
        }
    }
    return -1;
}

bool GeniusGame::isJogoFinalizado() {
    // Agora o jogo só é considerado "finalizado" em caso de vitória.
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
                    _buzzer.tocarNota(notes[ledIndex], LED_ON_DURATION);
                    
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
                digitalWrite(_ledPins[_sequencia[_passoAtualShow]], LOW);
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
                _buzzer.tocarNota(notes[botaoPressionado], 150);
                digitalWrite(_ledPins[botaoPressionado], LOW);
                
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
            _buzzer.tocarSomVitoria();
            _estadoAtual = OCIOSO; 
            break;

        case DERROTA: {
            _display.exibirMensagem("Genius:", "ERROU!", 2);
            _buzzer.tocarSomErro();
            // Pausa para o jogador ver a mensagem de erro.
            delay(1500); 

            iniciarNovoJogo(); 
            break;
        }

        case OCIOSO:
            // Não faz nada até que um novo jogo seja iniciado de fora (após uma vitória).
            break;
    }
}