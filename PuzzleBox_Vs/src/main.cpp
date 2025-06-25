// Includes
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "DisplayOLED.h"
#include "Buzzer.h"
#include "Button.h"
#include "AccelerometerMPU6050.h"
#include "KeypadMatrix.h"
#include "LightSensorLDR.h"
#include "GeniusGame.h"
#include "NonBlockingTimer.h"
#include <WiFi.h>
#include <WebServer.h>


/***********************************************************************
 Definições do Sistema
 ***********************************************************************/

// --- Estados da Máquina ---
enum Estado {
    INICIO, PREPARANDO_INCLINACAO, VERIFICANDO_INCLINACAO, FEEDBACK_INCLINACAO,
    PREPARANDO_LDR, VERIFICANDO_LDR, FEEDBACK_LDR,
    PREPARANDO_GENIUS, JOGO_GENIUS, FEEDBACK_GENIUS,
    PREPARANDO_CODIGO, ENTRADA_CODIGO_SECRETO, FEEDBACK_ERRO_CODIGO,
    FINALIZADO, NUM_ESTADOS
};

// --- Eventos ---
enum Evento {
    NENHUM_EVENTO = 0, BOTAO_INICIAR_PRESSIONADO, BOTAO_RESET_PRESSIONADO,
    TIMER_PAUSA_EXPIRADO, INCLINACAO_CORRETA, LDR_COBERTO, SEQUENCIA_GENIUS_CORRETA,
    CODIGO_SECRETO_CORRETO, CODIGO_SECRETO_INCORRETO, NUM_EVENTOS
};

// --- Ações ---
enum Acao {
    NENHUMA_ACAO = 0, A01_TELA_BEMVINDO, A02_PREPARAR_INCLINACAO,
    A03_PREPARAR_LDR, A04_PREPARAR_GENIUS, A05_PREPARAR_CODIGO,
    A06_FEEDBACK_SUCESSO, A07_FEEDBACK_ERRO_CODIGO, A08_FINALIZAR_PUZZLE,
    A09_RESETAR_JOGO, A10_INICIAR_JOGO_GENIUS 
};

/***********************************************************************
 Componentes e Pinos
 ***********************************************************************/
const int PINO_BOTAO_INICIAR = 23;
const int PINO_BOTAO_RESET = 16;
const int PINO_BUZZER = 2;
const int PINO_LDR = 34;
byte PINOS_LINHAS[LINHAS] = {13, 12, 14, 27};
byte PINOS_COLUNAS[COLUNAS] = {26, 25, 33, 17};
const int LED_PINS[] = {15, 4, 5};
const int BUTTON_PINS[] = {19, 18, 32};

DisplayOLED meuDisplay;
Buzzer meuBuzzer(PINO_BUZZER);
Button botaoIniciar(PINO_BOTAO_INICIAR);
Button botaoReset(PINO_BOTAO_RESET);
AccelerometerMPU6050 meuIMU;
KeypadMatrix meuTeclado(PINOS_LINHAS, PINOS_COLUNAS);
LightSensorLDR meuLDR(PINO_LDR, LightSensorLDR::LdrCalculation::MANUFACTURER_EXAMPLE);
GeniusGame meuJogoGenius(LED_PINS, BUTTON_PINS, meuDisplay, meuBuzzer);
NonBlockingTimer timerPausa;

String entradaTeclado = "";
String senhaRevelada = "------";
const char* CODIGO_SECRETO = "205733";

// --- Credenciais do Wi-Fi  ---
// const char* ssid = "iPhone de Luisa";
// const char* password = "123456789";

// Wi-Fi do Wokwi
const char* ssid = "Wokwi-GUEST"; 
const char* password = "";


WebServer server(80);

/***********************************************************************
 Estáticos e FreeRTOS
 ***********************************************************************/
volatile int estadoAtual = INICIO;
int acao_matrizTransicaoEstados[NUM_ESTADOS][NUM_EVENTOS];
int proximo_estado_matrizTransicaoEstados[NUM_ESTADOS][NUM_EVENTOS];
QueueHandle_t xFilaDeEventos;

void taskMaqEstados(void *pvParameters);
void taskObterEvento(void *pvParameters);
void taskAtualizacaoContinua(void *pvParameters);

/************************************************************************
 Funções da Máquina de Estados
 ************************************************************************/

int executarAcao(int acao) {
    if (acao == NENHUMA_ACAO) return NENHUM_EVENTO;
    switch (acao) {
        case A01_TELA_BEMVINDO:
            senhaRevelada = "------";
            meuDisplay.exibirMensagem("Puzzle Box", "Pressione Iniciar", 2);
            break;
        case A02_PREPARAR_INCLINACAO:
            meuBuzzer.tocarBeepSucesso();
            meuDisplay.exibirTelaDeDesafio("Nivel 1: Inclinacao", "Acerte o Angulo", senhaRevelada);
            timerPausa.start(3000); 
            meuIMU.definirInclinacaoAlvo(0.0, 90.0, 5);
            break;
        case A03_PREPARAR_LDR:
            senhaRevelada[0] = CODIGO_SECRETO[0];
            senhaRevelada[1] = CODIGO_SECRETO[1];
            meuDisplay.exibirTelaDeDesafio("Nivel 2: Luminosidade", "Cubra o Sensor", senhaRevelada);
            timerPausa.start(3000); 
            meuLDR.definirFaixaDeLuzAlvo(60.0, 100.0);
            break;
        case A04_PREPARAR_GENIUS:
            senhaRevelada[2] = CODIGO_SECRETO[2];
            senhaRevelada[3] = CODIGO_SECRETO[3];
            meuDisplay.exibirTelaDeDesafio("Nivel 3: Genius", "Observe...", senhaRevelada);
            timerPausa.start(3000); 
            break;
        case A05_PREPARAR_CODIGO:
            senhaRevelada[4] = CODIGO_SECRETO[4];
            senhaRevelada[5] = CODIGO_SECRETO[5];
            entradaTeclado = "";
            meuDisplay.exibirTelaDeDesafio("Nivel Final: Senha", "Digite o Codigo", senhaRevelada);
            break;
        case A06_FEEDBACK_SUCESSO:
            meuBuzzer.tocarBeepSucesso();
            meuDisplay.exibirMensagem("CORRETO!", "", 3);
            timerPausa.start(1500); 
            break;
        case A07_FEEDBACK_ERRO_CODIGO:
            meuBuzzer.tocarSomErro();
            meuDisplay.exibirMensagem("ERRO!", "Tente de novo", 2);
            timerPausa.start(1500); 
            break;
        case A08_FINALIZAR_PUZZLE:
            meuBuzzer.tocarSomVitoria();
            meuDisplay.exibirMensagem("PUZZLE", "RESOLVIDO!", 2);
            break;
        case A09_RESETAR_JOGO:
            meuBuzzer.tocarBeepTecla();
            executarAcao(A01_TELA_BEMVINDO); 
            break;
        case A10_INICIAR_JOGO_GENIUS:
            meuJogoGenius.iniciarNovoJogo();
            break;
    }
    return NENHUM_EVENTO;
}

void iniciaMaquinaEstados() {
    for (int i = 0; i < NUM_ESTADOS; i++) {
        for (int j = 0; j < NUM_EVENTOS; j++) {
            proximo_estado_matrizTransicaoEstados[i][j] = i;
            acao_matrizTransicaoEstados[i][j] = NENHUMA_ACAO;
        }
    }
    
    proximo_estado_matrizTransicaoEstados[INICIO][BOTAO_INICIAR_PRESSIONADO] = PREPARANDO_INCLINACAO;
    acao_matrizTransicaoEstados[INICIO][BOTAO_INICIAR_PRESSIONADO] = A02_PREPARAR_INCLINACAO;

    proximo_estado_matrizTransicaoEstados[PREPARANDO_INCLINACAO][TIMER_PAUSA_EXPIRADO] = VERIFICANDO_INCLINACAO;
    
    proximo_estado_matrizTransicaoEstados[VERIFICANDO_INCLINACAO][INCLINACAO_CORRETA] = FEEDBACK_INCLINACAO;
    acao_matrizTransicaoEstados[VERIFICANDO_INCLINACAO][INCLINACAO_CORRETA] = A06_FEEDBACK_SUCESSO;

    proximo_estado_matrizTransicaoEstados[FEEDBACK_INCLINACAO][TIMER_PAUSA_EXPIRADO] = PREPARANDO_LDR;
    acao_matrizTransicaoEstados[FEEDBACK_INCLINACAO][TIMER_PAUSA_EXPIRADO] = A03_PREPARAR_LDR;

    proximo_estado_matrizTransicaoEstados[PREPARANDO_LDR][TIMER_PAUSA_EXPIRADO] = VERIFICANDO_LDR;
    
    proximo_estado_matrizTransicaoEstados[VERIFICANDO_LDR][LDR_COBERTO] = FEEDBACK_LDR;
    acao_matrizTransicaoEstados[VERIFICANDO_LDR][LDR_COBERTO] = A06_FEEDBACK_SUCESSO;

    proximo_estado_matrizTransicaoEstados[FEEDBACK_LDR][TIMER_PAUSA_EXPIRADO] = PREPARANDO_GENIUS;
    acao_matrizTransicaoEstados[FEEDBACK_LDR][TIMER_PAUSA_EXPIRADO] = A04_PREPARAR_GENIUS;

    proximo_estado_matrizTransicaoEstados[PREPARANDO_GENIUS][TIMER_PAUSA_EXPIRADO] = JOGO_GENIUS;
    acao_matrizTransicaoEstados[PREPARANDO_GENIUS][TIMER_PAUSA_EXPIRADO] = A10_INICIAR_JOGO_GENIUS; 
    
    proximo_estado_matrizTransicaoEstados[JOGO_GENIUS][SEQUENCIA_GENIUS_CORRETA] = FEEDBACK_GENIUS;
    acao_matrizTransicaoEstados[JOGO_GENIUS][SEQUENCIA_GENIUS_CORRETA] = A06_FEEDBACK_SUCESSO;

    proximo_estado_matrizTransicaoEstados[FEEDBACK_GENIUS][TIMER_PAUSA_EXPIRADO] = PREPARANDO_CODIGO;
    acao_matrizTransicaoEstados[FEEDBACK_GENIUS][TIMER_PAUSA_EXPIRADO] = A05_PREPARAR_CODIGO;
    
    proximo_estado_matrizTransicaoEstados[PREPARANDO_CODIGO][NENHUM_EVENTO] = ENTRADA_CODIGO_SECRETO; 
    
    proximo_estado_matrizTransicaoEstados[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_CORRETO] = FINALIZADO;
    acao_matrizTransicaoEstados[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_CORRETO] = A08_FINALIZAR_PUZZLE;

    proximo_estado_matrizTransicaoEstados[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_INCORRETO] = FEEDBACK_ERRO_CODIGO;
    acao_matrizTransicaoEstados[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_INCORRETO] = A07_FEEDBACK_ERRO_CODIGO;
    
    proximo_estado_matrizTransicaoEstados[FEEDBACK_ERRO_CODIGO][TIMER_PAUSA_EXPIRADO] = PREPARANDO_CODIGO;
    acao_matrizTransicaoEstados[FEEDBACK_ERRO_CODIGO][TIMER_PAUSA_EXPIRADO] = A05_PREPARAR_CODIGO;
    
    for(int i=0; i < NUM_ESTADOS; i++) {
        proximo_estado_matrizTransicaoEstados[i][BOTAO_RESET_PRESSIONADO] = INICIO;
        acao_matrizTransicaoEstados[i][BOTAO_RESET_PRESSIONADO] = A09_RESETAR_JOGO;
    }
}

int obterAcao(int estado, int evento) { return acao_matrizTransicaoEstados[estado][evento]; }
int obterProximoEstado(int estado, int evento) { return proximo_estado_matrizTransicaoEstados[estado][evento]; }

String gerarPaginaHTML() {
  String html = "<!DOCTYPE html><html><head><title>Puzzle Box Status</title>";
  html += "<meta http-equiv='refresh' content='5'>"; // Atualiza a página a cada 5 segundos
  html += "<style>body { font-family: sans-serif; background-color: #282c34; color: #abb2bf; text-align: center; }";
  html += "h1 { color: #61afef; } .hint { background-color: #3e4451; padding: 15px; border-radius: 8px; margin-top: 20px; }</style>";
  html += "</head><body>";
  html += "<h1>Status da Puzzle Box</h1>";
  html += "<h2>Senha Parcial: " + senhaRevelada + "</h2>";

  html += "<div class='hint'>";
  // Gera uma dica baseada no estado atual
  switch (estadoAtual) {
    case INICIO:
      html += "Pressione o botao branco na caixa para comecar e o botao preto para reiniciar o jogo.";
      break;
    case VERIFICANDO_INCLINACAO:
      html += "Dica: A caixa parece ter uma posicao de equilibrio... Tente deixa-la na vertical.";
      break;
    case VERIFICANDO_LDR:
      html += "Dica: A escuridao as vezes revela segredos. Encontre o sensor de luz e cubra-o.";
      break;
    case JOGO_GENIUS:
      html += "Dica: Repita a sequencia de luzes e sons. Memoria e atencao sao a chave!";
      break;
    case ENTRADA_CODIGO_SECRETO:
      html += "Dica: Use a senha revelada para abrir a caixa!";
      break;
    case FINALIZADO:
      html += "Parabens! Voce resolveu o puzzle!";
      break;
    default:
      html += "Aguardando o proximo passo...";
      break;
  }
  html += "</div></body></html>";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", gerarPaginaHTML());
}

/***********************************************************************
 Tasks do FreeRTOS
 ***********************************************************************/

void taskMaqEstados(void *pvParameters) {
    int eventoRecebido;
    int codigoAcao;
    for (;;) {
        if (xQueueReceive(xFilaDeEventos, &eventoRecebido, portMAX_DELAY) == pdPASS) {
            codigoAcao = obterAcao(estadoAtual, eventoRecebido);
            estadoAtual = obterProximoEstado(estadoAtual, eventoRecebido);
            executarAcao(codigoAcao);

            if (estadoAtual == PREPARANDO_CODIGO) {
                int eventoAutomatico = NENHUM_EVENTO;
                xQueueSendToBack(xFilaDeEventos, &eventoAutomatico, 0);
            }
        }
    }
}

int obterEvento() {
    if (botaoReset.foiPressionado()) return BOTAO_RESET_PRESSIONADO;
    switch (estadoAtual) {
        case INICIO:
            if (botaoIniciar.foiPressionado()) return BOTAO_INICIAR_PRESSIONADO;
            break;
        case VERIFICANDO_INCLINACAO:
            if (meuIMU.verificarInclinacaoCorreta()) return INCLINACAO_CORRETA;
            break;
        case VERIFICANDO_LDR:
            if (meuLDR.verificarLuzCorreta()) return LDR_COBERTO;
            break;
        case JOGO_GENIUS:
            if (meuJogoGenius.isJogoFinalizado()) return SEQUENCIA_GENIUS_CORRETA;
            break;
        case ENTRADA_CODIGO_SECRETO: {
            char tecla = meuTeclado.obterTeclaPressionada();
            if (tecla != NO_KEY) {
                meuBuzzer.tocarBeepTecla();
                entradaTeclado += tecla;
                meuDisplay.exibirMensagem("Codigo:", entradaTeclado, 3);
                if (entradaTeclado.length() >= 6) {
                    return (entradaTeclado == CODIGO_SECRETO) ? CODIGO_SECRETO_CORRETO : CODIGO_SECRETO_INCORRETO;
                }
            }
            break;
        }
        case PREPARANDO_INCLINACAO:
        case PREPARANDO_LDR:
        case PREPARANDO_GENIUS:
        case FEEDBACK_INCLINACAO:
        case FEEDBACK_LDR:
        case FEEDBACK_GENIUS:
        case FEEDBACK_ERRO_CODIGO:
            if (timerPausa.hasExpired()) return TIMER_PAUSA_EXPIRADO;
            break;
    }
    return NENHUM_EVENTO;
}

void taskObterEvento(void *pvParameters) {
    for (;;) {
        int eventoDetectado = obterEvento();
        if (eventoDetectado != NENHUM_EVENTO) {
            xQueueSendToBack(xFilaDeEventos, &eventoDetectado, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void taskAtualizacaoContinua(void *pvParameters) {
    unsigned long ultimoUpdateDisplay = 0;
    const int INTERVALO_DISPLAY = 40;

    for(;;) {
        meuIMU.atualizar();
        if (estadoAtual == VERIFICANDO_INCLINACAO) {
            if (millis() - ultimoUpdateDisplay > INTERVALO_DISPLAY) {
                meuDisplay.exibirDadosIMU(meuIMU.getRoll(), meuIMU.getPitch());
                ultimoUpdateDisplay = millis();
            }
        }
        if (estadoAtual == VERIFICANDO_LDR) {
            if (millis() - ultimoUpdateDisplay > INTERVALO_DISPLAY) {
                float lux = meuLDR.lerNivelDeLuz();
                meuDisplay.exibirMensagem("Luz Atual:", String(lux, 0) + " Lux", 2);
                ultimoUpdateDisplay = millis();
            }
        }
        if (estadoAtual == JOGO_GENIUS) {
            meuJogoGenius.loop();
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}


void taskWiFiWebServer(void *pvParameters) {
    for (;;) {
        server.handleClient(); // Escuta por requisições HTTP
        vTaskDelay(pdMS_TO_TICKS(5)); // Pequena pausa para não sobrecarregar
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("--- Puzzle Box ---");

    meuDisplay.inicializar();
    meuIMU.inicializar();
    meuBuzzer.inicializar();
    botaoIniciar.inicializar();
    botaoReset.inicializar();
    meuLDR.inicializar();
    meuJogoGenius.inicializar();

    // --- Inicia a conexão Wi-Fi ---
    Serial.print("Conectando a ");
    Serial.println(ssid);
    meuDisplay.exibirMensagem("Conectando", "Wi-Fi...", 2);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi conectado!");
    Serial.print("Endereco IP: ");
    Serial.println(WiFi.localIP());

    // --- Configura o Servidor Web ---
    server.on("/", handleRoot); // Rota principal
    server.begin();
    Serial.println("Servidor HTTP iniciado.");

    iniciaMaquinaEstados();
    executarAcao(A01_TELA_BEMVINDO);

    xFilaDeEventos = xQueueCreate(10, sizeof(int));

    if (xFilaDeEventos != NULL) {
        xTaskCreate(taskMaqEstados, "MaqEstados", 4096, NULL, 2, NULL);
        xTaskCreate(taskObterEvento, "ObterEvento", 4096, NULL, 1, NULL);
        xTaskCreate(taskAtualizacaoContinua, "Atualiza", 2048, NULL, 1, NULL);
        xTaskCreate(taskWiFiWebServer, "WebServer", 4096, NULL, 1, NULL);
    } else {
        Serial.println("Erro ao criar a fila!");
    }
}

void loop() {
  vTaskDelay(100);
}