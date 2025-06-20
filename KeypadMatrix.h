// KeypadMatrix.h

#ifndef KEYPAD_MATRIX_H
#define KEYPAD_MATRIX_H

#include <Keypad.h>

const byte LINHAS = 4;
const byte COLUNAS = 4;

class KeypadMatrix {
public:
    // O construtor recebe os arrays de pinos para as linhas e colunas
    KeypadMatrix(byte* pinosLinhas, byte* pinosColunas);

    // Retorna a tecla pressionada no momento, ou NO_KEY se nenhuma for pressionada
    char obterTeclaPressionada();

private:
    // Mapa de caracteres do nosso teclado 4x4
    char _mapaDeTeclas[LINHAS][COLUNAS];

    byte _pinosLinhas[LINHAS];
    byte _pinosColunas[COLUNAS];
    
    // Objeto da biblioteca Keypad que fará todo o trabalho pesado
    Keypad _keypad;
};

#endif