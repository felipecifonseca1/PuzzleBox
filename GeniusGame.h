#ifndef GENIUS_GAME_H
#define GENIUS_GAME_H

#include "Arduino.h"
#include "Buzzer.h"
#include "DisplayOLED.h"

const int GENIUS_SEQUENCE_LENGTH = 3;

class GeniusGame {
public:
    GeniusGame(const int ledPins[GENIUS_SEQUENCE_LENGTH], const int buttonPins[GENIUS_SEQUENCE_LENGTH], DisplayOLED& display, Buzzer& buzzer);
    void inicializar();
    void iniciarNovoJogo();
    void loop();
    bool isJogoFinalizado();

private:
    // Máquina de estados para controlar o fluxo do jogo de forma precisa
    enum GameState {
        OCIOSO,
        MOSTRANDO_LED,      // Estado para LIGAR o LED da sequência
        MOSTRANDO_PAUSA,    // Estado para dar um tempo com o LED aceso
        AGUARDANDO_JOGADOR,
        VITORIA,
        DERROTA
    };

    GameState _estadoAtual;

    int _ledPins[GENIUS_SEQUENCE_LENGTH];
    int _buttonPins[GENIUS_SEQUENCE_LENGTH];
    DisplayOLED& _display;
    Buzzer& _buzzer;

    int _sequencia[GENIUS_SEQUENCE_LENGTH];
    int _passoAtualJogador;
    int _passoAtualShow;
    unsigned long _ultimoTempo;

    void _gerarSequencia();
    int _lerBotoes();
};

#endif