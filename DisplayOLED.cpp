// DisplayOLED.cpp

#include "DisplayOLED.h"

DisplayOLED::DisplayOLED() : _display(LARGURA_TELA, ALTURA_TELA, &Wire, -1) {
}

// Implementação do método de inicialização
bool DisplayOLED::inicializar() {
    // O SSD1306_SWITCHCAPVCC diz para gerar a tensão do display a partir dos 3.3V
    if (!_display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("Falha ao inicializar o Display OLED"));
        return false;
    }
    
    _display.clearDisplay();
    _display.display(); 
    return true;
}

void DisplayOLED::limpar() {
    _display.clearDisplay();
    _display.display();
}

void DisplayOLED::exibirMensagem(const String& linha1, const String& linha2, uint8_t tamanhoTexto) {
    _display.clearDisplay();
    _display.setTextSize(tamanhoTexto);
    _display.setTextColor(SSD1306_WHITE);

    // Centraliza a primeira linha
    _display.setCursor(0, 16); // Posição aproximada para o centro vertical
    _display.println(linha1); 

    if (linha2 != "") {
        _display.println(linha2);
    }

    _display.display(); 
}


void DisplayOLED::animacaoCarregando(int duracao_ms) {
    _display.clearDisplay();
    long startTime = millis();
    int frame = 0;
    while(millis() - startTime < duracao_ms) {
        _display.clearDisplay();
        _display.setTextSize(2);
        _display.setCursor(0, 24);
        
        String texto = "Carregando";
        for (int i=0; i < (frame % 4); i++) {
          texto += ".";
        }
        _display.print(texto);
        _display.display();
        frame++;
        delay(250);
    }
}