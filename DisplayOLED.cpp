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
    long startTime = millis();
    int frame = 0;

    while (millis() - startTime < duracao_ms) {
        _display.clearDisplay();
        _display.setTextSize(2);
        _display.setTextColor(SSD1306_WHITE);

        // 1. Monta o texto do quadro atual
        String texto = "Carregando";
        for (int i = 0; i < (frame % 4); i++) {
            texto += ".";
        }

        // 2. Calcula as dimensões e a posição para centralizar o texto
        int16_t x1, y1;
        uint16_t w, h;
        _display.getTextBounds(texto, 0, 0, &x1, &y1, &w, &h); // Pega a largura (w) e altura (h) do texto
        
        // Calcula a posição do cursor para que o texto fique no centro
        int cursorX = (LARGURA_TELA - w) / 2;
        int cursorY = (ALTURA_TELA - h) / 2;

        // 3. Define o cursor na posição calculada e imprime o texto
        _display.setCursor(cursorX, cursorY);
        _display.print(texto);
        
        _display.display(); // Mostra o quadro na tela
        
        frame++;
        delay(300); // Um delay um pouco maior para a animação ficar mais agradável
    }
}

void DisplayOLED::exibirDadosIMU(float roll, float pitch) {
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 0);
    _display.println("--- Teste do IMU ---");
    _display.setTextSize(2);
    _display.setCursor(0, 20);
    _display.print("R: ");
    _display.print(roll, 1); // Apenas 1 casa decimal
    _display.setCursor(0, 40);
    _display.print("P: ");
    _display.print(pitch, 1);
    _display.display();
}