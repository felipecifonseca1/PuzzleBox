// KeypadMatrix.cpp

#include "KeypadMatrix.h"

// Implementação do construtor
KeypadMatrix::KeypadMatrix(byte* pinosLinhas, byte* pinosColunas)
  // Usamos uma lista de inicialização para construir o objeto _keypad.
  // Isso é mais eficiente e é a maneira correta de inicializar objetos de membro.
  : _mapaDeTeclas{
      {'1', '2', '3', 'A'},
      {'4', '5', '6', 'B'},
      {'7', '8', '9', 'C'},
      {'*', '0', '#', 'D'}
    },
    _keypad(makeKeymap(_mapaDeTeclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS)
{
    // O corpo do construtor pode ficar vazio, pois a inicialização
    // principal foi feita na lista de inicialização acima.
}

// Implementação do método que lê a tecla
char KeypadMatrix::obterTeclaPressionada() {
    // A biblioteca Keypad já cuida do debounce e da varredura.
    // Nós simplesmente pedimos a ela a tecla atual.
    return _keypad.getKey();
}