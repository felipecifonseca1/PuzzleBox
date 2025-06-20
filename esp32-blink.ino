// MegaTesteIntegrado.ino
// Testa todos os componentes principais do projeto em sequência.

// 1. Includes de todas as nossas classes
#include "DisplayOLED.h"
#include "Buzzer.h"
#include "Button.h"
#include "AccelerometerMPU6050.h"
#include "KeypadMatrix.h"
#include "LightSensorLDR.h" 
#include "GeniusGame.h"


// 2. Definição dos Pinos
// --- Pinos I2C são definidos pela biblioteca Wire ---
const int PINO_BOTAO_INICIAR = 23;
const int PINO_BUZZER = 2;
byte PINOS_LINHAS[LINHAS] = {13, 12, 14, 27};
byte PINOS_COLUNAS[COLUNAS] = {26, 25, 33, 35};
const int PINO_LDR = 34;

const int LED_PINS[GENIUS_SEQUENCE_LENGTH] = {15, 4, 5}; // Ex: Vermelho, Verde, Azul
const int BUTTON_PINS[GENIUS_SEQUENCE_LENGTH] = {19, 18, 32}; // Ex: Botão Vermelho, Verde, Azul

// 3. Instanciação dos Objetos
DisplayOLED meuDisplay;
Buzzer meuBuzzer(PINO_BUZZER);
Button botaoIniciar(PINO_BOTAO_INICIAR);
AccelerometerMPU6050 meuIMU;
KeypadMatrix meuTeclado(PINOS_LINHAS, PINOS_COLUNAS);
LightSensorLDR meuLDR(PINO_LDR, LightSensorLDR::LdrCalculation::MANUFACTURER_EXAMPLE);
GeniusGame meuJogoGenius(LED_PINS, BUTTON_PINS, meuDisplay, meuBuzzer);

// 4. Variáveis de Controle do Teste
int estadoTeste = 0;
String entradaTeclado = "";
bool geniusIniciado = false; 

void setup() {
  Serial.begin(115200);
  Serial.println("--- Mega Teste Integrado ---");

  // 5. Inicializa todos os componentes
  bool displayOk = meuDisplay.inicializar();
  bool imuOk = meuIMU.inicializar();
  meuBuzzer.inicializar();
  botaoIniciar.inicializar();
  meuLDR.inicializar();
  meuJogoGenius.inicializar();
  // Keypad não tem método inicializar, é feito no construtor

  // Verifica se os componentes I2C foram encontrados
  if (!displayOk || !imuOk) {
    Serial.println("ERRO: Falha em um componente I2C. Verifique as conexoes.");
    while (1); // Para a execução
  }

  // Sequência inicial
  meuDisplay.exibirMensagem("TESTE", "SISTEMA", 3);
  meuBuzzer.tocarBeepSucesso();
  delay(1500);

  meuDisplay.exibirMensagem("Pressione", "o Botao", 2);
  estadoTeste = 1; // Próximo estado: aguardar botão
}

void loop() {
  // A lógica do teste é controlada por uma mini-máquina de estados
  switch (estadoTeste) {
    case 1: // Estado 1: Aguardando o botão Iniciar
      if (botaoIniciar.foiPressionado()) {
        Serial.println("Botao pressionado. Iniciando teste do IMU.");
        meuBuzzer.tocarBeepSucesso();
        // Define uma inclinação alvo para o teste (ex: caixa deitada de lado)
        meuIMU.definirInclinacaoAlvo(90.0, 0.0, 10.0); // Roll 90, Pitch 0, com tolerância de 10 graus
        estadoTeste = 4; // Vai para o próximo estado
      }
      break;

    case 2: // Estado 2: Testando o MPU6050
      meuIMU.atualizar(); // Atualiza o filtro AHRS
      meuIMU.imprimirLeituras(); // Imprime no Serial para debug
      meuDisplay.exibirDadosIMU(meuIMU.getRoll(), meuIMU.getPitch()); // Usa o novo método do display

      if (meuIMU.verificarInclinacaoCorreta()) {
        Serial.println("Inclinacao correta atingida! Iniciando teste do teclado.");
        meuBuzzer.tocarBeepSucesso();
        meuDisplay.exibirMensagem("Digite:", "'123A'", 2);
        entradaTeclado = ""; // Limpa a string de entrada
        estadoTeste = 3; // Vai para o próximo estado
      }
      break;

    case 3: {
        char tecla = meuTeclado.obterTeclaPressionada();
        if (tecla != NO_KEY) {
          meuBuzzer.tocarBeepTecla();
          entradaTeclado += tecla;
          meuDisplay.exibirMensagem("Entrada:", entradaTeclado, 2);

          if (entradaTeclado == "123A") {
            Serial.println("Sequencia correta! Teste concluido.");
            meuDisplay.exibirMensagem("TESTES OK!", "PARABENS!", 2);
            meuBuzzer.tocarSomVitoria();
            
            meuDisplay.exibirMensagem("Ajuste a Luz", "(20-100 Lux)", 2);
            estadoTeste = 4;
          } else if (entradaTeclado.length() >= 4) {
            Serial.println("Sequencia incorreta. Tente novamente.");
            meuDisplay.exibirMensagem("ERRO!", "Tente de novo", 2);
            meuBuzzer.tocarSomErro();
            delay(1000);
            meuDisplay.exibirMensagem("Digite:", "'123A'", 2);
            entradaTeclado = "";
          }
        }
        break;
      }  // <-- FECHA A CHAVE AQUI

    case 4: { // Teste do LDR
        meuLDR.definirFaixaDeLuzAlvo(20.0, 100.0); // Define uma faixa de iluminação
        float lux = meuLDR.lerNivelDeLuz();
        Serial.print("Nivel de Luz: ");
        Serial.println(lux);

        // Exibe a leitura atual no display
        meuDisplay.exibirMensagem("Luz Atual:", String(lux, 0) + " Lux", 2);
      
        if (meuLDR.verificarLuzCorreta()) {
            Serial.println("Nivel de luz correto! Iniciando Jogo Genius!"); // MODIFICADO
            meuDisplay.exibirMensagem("PROXIMO:", "JOGO GENIUS", 2);
            meuBuzzer.tocarBeepSucesso();
            delay(2000);
            estadoTeste = 5; // Vai para o estado do genius
      }
      delay(250); // Pequeno delay para evitar flickering e spam no serial
      break;
    }

    case 5: {
        if (!geniusIniciado) {
            meuJogoGenius.iniciarNovoJogo();
            geniusIniciado = true;
        }

        meuJogoGenius.loop();

        if (meuJogoGenius.isJogoFinalizado()) {
            Serial.println("Jogo Genius finalizado! Todos os testes concluidos!");
            // Mensagem final de sucesso geral
            delay(2000);
            meuDisplay.exibirMensagem("PUZZLE BOX", "OK!", 3);
            meuBuzzer.tocarBeepSucesso();
            estadoTeste = 6; // Vai para o estado final
        }
        break;
    }

    case 6: // Estado Final (antigo estado 5)
      // O programa para aqui, todos os testes concluídos com sucesso.

      break;
  }
}