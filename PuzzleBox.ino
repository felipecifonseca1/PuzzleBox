// 1. Includes
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

/***********************************************************************
 Definições do Sistema
 ***********************************************************************/

// --- Estados da Máquina ---
enum Estado {
    INICIO, 
    VERIFICANDO_INCLINACAO,
    VERIFICANDO_LDR,
    JOGO_GENIUS,
    ENTRADA_CODIGO_SECRETO,
    AGUARDANDO_FEEDBACK, 
    FINALIZADO,
    NUM_ESTADOS
};

// --- Eventos que podem ocorrer ---
enum Evento {
    NENHUM_EVENTO = 0, BOTAO_INICIAR_PRESSIONADO, BOTAO_RESET_PRESSIONADO,
    INCLINACAO_CORRETA, LDR_COBERTO, SEQUENCIA_GENIUS_CORRETA,
    CODIGO_SECRETO_CORRETO, CODIGO_SECRETO_INCORRETO, TIMER_FEEDBACK_EXPIRADO,
    NUM_EVENTOS
};

// --- Ações que a máquina de estados pode executar ---
enum Acao {
    NENHUMA_ACAO = 0, A01_TELA_BEMVINDO, A02_PREPARAR_INCLINACAO,
    A03_PREPARAR_LDR, A04_PREPARAR_GENIUS, A05_PREPARAR_CODIGO,
    A06_FEEDBACK_SUCESSO_ETAPA, A07_FEEDBACK_ERRO_CODIGO, A08_FINALIZAR_PUZZLE,
    A09_RESETAR_JOGO
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
NonBlockingTimer feedbackTimer;

String entradaTeclado = "";
const char* CODIGO_SECRETO = "123A";

/***********************************************************************
 Estáticos da Máquina de Estados
 ***********************************************************************/
volatile int estadoAtual = INICIO;
int codigoEvento = NENHUM_EVENTO;
int eventoInterno = NENHUM_EVENTO;
int codigoAcao;
int acao_matrizTransicaoEstados[NUM_ESTADOS][NUM_EVENTOS];
int proximo_estado_matrizTransicaoEstados[NUM_ESTADOS][NUM_EVENTOS];

/***********************************************************************
 FreeRTOS - Declarações
 ***********************************************************************/
void taskMaqEstados(void *pvParameters);
void taskObterEvento(void *pvParameters);
void taskAtualizacaoContinua(void *pvParameters);
QueueHandle_t xFilaDeEventos;

/************************************************************************
 Funções da Máquina de Estados
 ************************************************************************/

// --- executarAcao ---
int executarAcao(int acao) {
    if (acao == NENHUMA_ACAO) return NENHUM_EVENTO;
    switch (acao) {
        case A01_TELA_BEMVINDO:
            meuDisplay.exibirMensagem("Puzzle Box", "Pressione Iniciar", 2);
            break;
        case A02_PREPARAR_INCLINACAO:
            meuBuzzer.tocarBeepSucesso();
            meuDisplay.exibirMensagem("Nivel 1", "Incline a Caixa", 2);
            meuIMU.definirInclinacaoAlvo(0.0, 30.0, 10.0); 
            break;
        case A03_PREPARAR_LDR:
            meuDisplay.exibirMensagem("Nivel 2", "Cubra o Sensor", 2);
            meuLDR.definirFaixaDeLuzAlvo(0, 100.0);
            break;
        case A04_PREPARAR_GENIUS:
            meuJogoGenius.iniciarNovoJogo();
            break;
        case A05_PREPARAR_CODIGO:
            entradaTeclado = "";
            meuDisplay.exibirMensagem("Senha Final:", "", 2);
            break;
        case A06_FEEDBACK_SUCESSO_ETAPA:
            meuBuzzer.tocarBeepSucesso();
            meuDisplay.exibirMensagem("CORRETO!", "", 3);
            feedbackTimer.start(1500);
            break;
        case A07_FEEDBACK_ERRO_CODIGO:
            meuBuzzer.tocarSomErro();
            meuDisplay.exibirMensagem("ERRO!", "Tente de novo", 2);
            feedbackTimer.start(1500); 
            break;
        case A08_FINALIZAR_PUZZLE:
            meuBuzzer.tocarSomVitoria();
            meuDisplay.exibirMensagem("PUZZLE", "RESOLVIDO!", 3);
            break;
        case A09_RESETAR_JOGO:
            meuBuzzer.tocarBeepTecla();
            executarAcao(A01_TELA_BEMVINDO); 
            break;
    }
    return NENHUM_EVENTO;
}

// --- iniciaMaquinaEstados (CORRIGIDO) ---
void iniciaMaquinaEstados() {
    for (int i = 0; i < NUM_ESTADOS; i++) {
        for (int j = 0; j < NUM_EVENTOS; j++) {
            proximo_estado_matrizTransicaoEstados[i][j] = i;
            acao_matrizTransicaoEstados[i][j] = NENHUMA_ACAO;
        }
    }
    
    // Fluxo Principal
    proximo_estado_matrizTransicaoEstados[INICIO][BOTAO_INICIAR_PRESSIONADO] = VERIFICANDO_INCLINACAO;
    acao_matrizTransicaoEstados[INICIO][BOTAO_INICIAR_PRESSIONADO] = A02_PREPARAR_INCLINACAO;

    proximo_estado_matrizTransicaoEstados[VERIFICANDO_INCLINACAO][INCLINACAO_CORRETA] = AGUARDANDO_FEEDBACK;
    acao_matrizTransicaoEstados[VERIFICANDO_INCLINACAO][INCLINACAO_CORRETA] = A06_FEEDBACK_SUCESSO_ETAPA;

    proximo_estado_matrizTransicaoEstados[VERIFICANDO_LDR][LDR_COBERTO] = AGUARDANDO_FEEDBACK;
    acao_matrizTransicaoEstados[VERIFICANDO_LDR][LDR_COBERTO] = A06_FEEDBACK_SUCESSO_ETAPA;

    proximo_estado_matrizTransicaoEstados[JOGO_GENIUS][SEQUENCIA_GENIUS_CORRETA] = AGUARDANDO_FEEDBACK;
    acao_matrizTransicaoEstados[JOGO_GENIUS][SEQUENCIA_GENIUS_CORRETA] = A06_FEEDBACK_SUCESSO_ETAPA;

    proximo_estado_matrizTransicaoEstados[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_INCORRETO] = AGUARDANDO_FEEDBACK;
    acao_matrizTransicaoEstados[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_INCORRETO] = A07_FEEDBACK_ERRO_CODIGO;
    
    // Lógica da transição especial do timer
    proximo_estado_matrizTransicaoEstados[AGUARDANDO_FEEDBACK][TIMER_FEEDBACK_EXPIRADO] = -1;

    // Lógica da senha correta 
    proximo_estado_matrizTransicaoEstados[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_CORRETO] = FINALIZADO;
    acao_matrizTransicaoEstados[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_CORRETO] = A08_FINALIZAR_PUZZLE;

    // Lógica do Reset
    for(int i=0; i < NUM_ESTADOS; i++) {
        proximo_estado_matrizTransicaoEstados[i][BOTAO_RESET_PRESSIONADO] = INICIO;
        acao_matrizTransicaoEstados[i][BOTAO_RESET_PRESSIONADO] = A09_RESETAR_JOGO;
    }
}

int obterAcao(int estado, int evento) { return acao_matrizTransicaoEstados[estado][evento]; }
int obterProximoEstado(int estado, int evento) { return proximo_estado_matrizTransicaoEstados[estado][evento]; }

/***********************************************************************
 Tasks do FreeRTOS
 ***********************************************************************/

// --- taskMaqEstados  ---
void taskMaqEstados(void *pvParameters) {
    int eventoRecebido;
    int estadoAnterior = INICIO;

    for (;;) {
        if (xQueueReceive(xFilaDeEventos, &eventoRecebido, portMAX_DELAY) == pdPASS) {
            
            if (estadoAtual != AGUARDANDO_FEEDBACK) {
                 estadoAnterior = estadoAtual;
            }
            
            int proximoEstado = obterProximoEstado(estadoAtual, eventoRecebido);
            int acao = obterAcao(estadoAtual, eventoRecebido);

            // Lógica especial para a transição do timer
            if (proximoEstado == -1 && eventoRecebido == TIMER_FEEDBACK_EXPIRADO) {
                if (estadoAnterior == VERIFICANDO_INCLINACAO) {
                    proximoEstado = VERIFICANDO_LDR;
                    acao = A03_PREPARAR_LDR;
                } else if (estadoAnterior == VERIFICANDO_LDR) {
                    proximoEstado = JOGO_GENIUS;
                    acao = A04_PREPARAR_GENIUS;
                } else if (estadoAnterior == JOGO_GENIUS) {
                    proximoEstado = ENTRADA_CODIGO_SECRETO;
                    acao = A05_PREPARAR_CODIGO;
                } else if (estadoAnterior == ENTRADA_CODIGO_SECRETO) {
                    proximoEstado = ENTRADA_CODIGO_SECRETO;
                    acao = A05_PREPARAR_CODIGO;
                }
            }
            
            estadoAtual = proximoEstado;
            executarAcao(acao);
        }
    }
}

// --- obterEvento ---
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
                if (entradaTeclado.length() >= 4) {
                    return (entradaTeclado == CODIGO_SECRETO) ? CODIGO_SECRETO_CORRETO : CODIGO_SECRETO_INCORRETO;
                }
            }
            break;
        }
        case AGUARDANDO_FEEDBACK:
            if (feedbackTimer.hasExpired()) return TIMER_FEEDBACK_EXPIRADO;
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

// --- taskAtualizacaoContinua ---
void taskAtualizacaoContinua(void *pvParameters) {
    unsigned long ultimoUpdateDisplay = 0;
    const int INTERVALO_DISPLAY = 50; // Atualiza o display a cada 50ms 

    for(;;) {
        if (estadoAtual == VERIFICANDO_INCLINACAO) {
            meuIMU.atualizar();
            if(millis() - ultimoUpdateDisplay > INTERVALO_DISPLAY) {
                meuDisplay.exibirDadosIMU(meuIMU.getRoll(), meuIMU.getPitch());
                ultimoUpdateDisplay = millis();
            }
        }
        if (estadoAtual == VERIFICANDO_LDR) {
            if(millis() - ultimoUpdateDisplay > INTERVALO_DISPLAY) {
                float lux = meuLDR.lerNivelDeLuz();
                meuDisplay.exibirMensagem("Luz Atual:", String(lux, 0) + " Lux", 2);
                ultimoUpdateDisplay = millis();
            }
        }
        if (estadoAtual == JOGO_GENIUS) {
            meuJogoGenius.loop();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


/************************************************************************
 Main (setup e loop)
 *************************************************************************/
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

    iniciaMaquinaEstados();
    executarAcao(A01_TELA_BEMVINDO);

    xFilaDeEventos = xQueueCreate(10, sizeof(int));

    if (xFilaDeEventos != NULL) {
        xTaskCreate(taskMaqEstados, "MaqEstados", 4096, NULL, 2, NULL);
        xTaskCreate(taskObterEvento, "ObterEvento", 4096, NULL, 1, NULL);
        xTaskCreate(taskAtualizacaoContinua, "Atualiza", 4096, NULL, 1, NULL);
    } else {
        Serial.println("Erro ao criar a fila!");
    }
}

void loop() {
  vTaskDelay(1000);
}