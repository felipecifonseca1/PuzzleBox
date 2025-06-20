// DisplayOLED.h

#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LARGURA_TELA 128 // Largura do OLED em pixels
#define ALTURA_TELA 64   // Altura do OLED em pixels

class DisplayOLED {
public:
    // Construtor
    DisplayOLED();

    // Inicializa o display. Retorna 'true' se for bem-sucedido.
    bool inicializar();

    // Limpa completamente a tela
    void limpar();

    // Exibe até duas linhas de texto centralizadas na tela
    void exibirMensagem(const String& linha1, const String& linha2 = "", uint8_t tamanhoTexto = 2);

    // Mostra uma animação simples de "carregando"
    void animacaoCarregando(int duracao_ms);

    void exibirDadosIMU(float roll, float pitch);

private:
    // Objeto da biblioteca Adafruit que representa nosso display físico
    Adafruit_SSD1306 _display;
};

#endif