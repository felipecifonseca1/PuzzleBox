#include "KeypadMatrix.h"

// Implementação do construtor
KeypadMatrix::KeypadMatrix(byte* pinosLinhas, byte* pinosColunas)

  : _mapaDeTeclas{
      {'1', '2', '3', 'A'},
      {'4', '5', '6', 'B'},
      {'7', '8', '9', 'C'},
      {'*', '0', '#', 'D'}
    },
    _keypad(makeKeymap(_mapaDeTeclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS)
{

}

// Implementação do método que lê a tecla
char KeypadMatrix::obterTeclaPressionada() {

    return _keypad.getKey();
}