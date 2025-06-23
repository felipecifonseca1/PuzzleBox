// Includes para as classes dos componentes 
#include "DisplayOLED.h"
#include "Buzzer.h"
#include "Button.h"
#include "AccelerometerMPU6050.h"
#include "KeypadMatrix.h"
#include "LightSensorLDR.h"
#include "GeniusGame.h"

/***********************************************************************
 Definições do Sistema 
 ***********************************************************************/

// --- Estados da Máquina ---
enum Estado {
    AGUARDANDO,                // 0: Sistema ligado, aguardando início
    VERIFICANDO_INCLINACAO,    // 1: Jogador ajusta a inclinação
    VERIFICANDO_LDR,           // 2: Jogador cobre o sensor de luz
    JOGO_GENIUS,               // 3: Jogador joga o Genius
    ENTRADA_CODIGO_SECRETO,    // 4: Jogador digita a senha
    FINALIZADO,                // 5: Puzzle resolvido
    NUM_ESTADOS                // Helper para o tamanho dos arrays
};

// --- Eventos que podem ocorrer ---
enum Evento {
    NENHUM_EVENTO = 0,
    BOTAO_INICIAR_PRESSIONADO,
    BOTAO_RESET_PRESSIONADO,
    INCLINACAO_CORRETA,
    LDR_COBERTO,
    SEQUENCIA_GENIUS_CORRETA,
    CODIGO_SECRETO_CORRETO,
    CODIGO_SECRETO_INCORRETO,
    NUM_EVENTOS
};

// --- Ações que a máquina de estados pode executar ---
enum Acao {
    NENHUMA_ACAO = 0,
    A01_TELA_BEMVINDO,
    A02_PREPARAR_INCLINACAO,
    A03_PREPARAR_LDR,
    A04_PREPARAR_GENIUS,
    A05_PREPARAR_CODIGO,
    A06_FEEDBACK_SUCESSO_ETAPA,
    A07_FEEDBACK_ERRO_CODIGO,
    A08_FINALIZAR_PUZZLE,
    A09_RESETAR_JOGO
};


// --- Definição dos Pinos ---
const int PINO_BOTAO_INICIAR = 23;
const int PINO_BOTAO_RESET = 16;
const int PINO_BUZZER = 2;
const int PINO_LDR = 34;
byte PINOS_LINHAS[LINHAS] = {13, 12, 14, 27};
byte PINOS_COLUNAS[COLUNAS] = {26, 25, 33, 17};
const int LED_PINS[] = {15, 4, 5};
const int BUTTON_PINS[] = {19, 18, 32};

// --- Instanciação dos Objetos ---
DisplayOLED meuDisplay;
Buzzer meuBuzzer(PINO_BUZZER);
Button botaoIniciar(PINO_BOTAO_INICIAR);
Button botaoReset(PINO_BOTAO_RESET);
AccelerometerMPU6050 meuIMU;
KeypadMatrix meuTeclado(PINOS_LINHAS, PINOS_COLUNAS);
LightSensorLDR meuLDR(PINO_LDR, LightSensorLDR::LdrCalculation::MANUFACTURER_EXAMPLE);
GeniusGame meuJogoGenius(LED_PINS, BUTTON_PINS, meuDisplay, meuBuzzer);

// Variáveis de controle
String entradaTeclado = "";
const char* CODIGO_SECRETO = "1234";

/***********************************************************************
 Estáticos da Máquina de Estados
 ***********************************************************************/
int estadoAtual = AGUARDANDO;
int codigoEvento = NENHUM_EVENTO;
int eventoInterno = NENHUM_EVENTO;
int codigoAcao;
int acao_matrizTransicaoEstados[NUM_ESTADOS][NUM_EVENTOS];
int proximo_estado_matrizTransicaoEstados[NUM_ESTADOS][NUM_EVENTOS];

/************************************************************************
 executarAcao
 *************************************************************************/
int executarAcao(int acao) {
    if (acao == NENHUMA_ACAO) return NENHUM_EVENTO;

    switch (acao) {
        case A01_TELA_BEMVINDO:
            meuDisplay.exibirMensagem("Puzzle Box", "Pressione Iniciar", 2);
            break;
        case A02_PREPARAR_INCLINACAO:
            meuBuzzer.tocarBeepSucesso();
            meuDisplay.exibirMensagem("Nivel 1", "Incline a Caixa", 2);
            meuIMU.definirInclinacaoAlvo(0.0, 20.0, 5.0);
            break;
        case A03_PREPARAR_LDR:
            executarAcao(A06_FEEDBACK_SUCESSO_ETAPA);
            meuDisplay.exibirMensagem("Nivel 2", "Cubra o Sensor", 2);
            meuLDR.definirFaixaDeLuzAlvo(0, 100.0);
            break;
        case A04_PREPARAR_GENIUS:
            executarAcao(A06_FEEDBACK_SUCESSO_ETAPA);
            meuJogoGenius.iniciarNovoJogo();
            break;
        case A05_PREPARAR_CODIGO:
            executarAcao(A06_FEEDBACK_SUCESSO_ETAPA);
            entradaTeclado = "";
            meuDisplay.exibirMensagem("Senha Final:", "", 2);
            break;
        case A06_FEEDBACK_SUCESSO_ETAPA:
            meuBuzzer.tocarBeepSucesso();
            meuDisplay.exibirMensagem("CORRETO!", "", 3);
            delay(1500);
            break;
        case A07_FEEDBACK_ERRO_CODIGO:
            meuBuzzer.tocarSomErro();
            meuDisplay.exibirMensagem("ERRO!", "Tente de novo", 2);
            delay(1500);
            entradaTeclado = "";
            meuDisplay.exibirMensagem("Senha Final:", "", 2);
            break;
        case A08_FINALIZAR_PUZZLE:
            executarAcao(A06_FEEDBACK_SUCESSO_ETAPA);
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

/************************************************************************
 iniciaMaquinaEstados
 *************************************************************************/
void iniciaMaquinaEstados() {
    for (int i = 0; i < NUM_ESTADOS; i++) {
        for (int j = 0; j < NUM_EVENTOS; j++) {
            proximo_estado_matrizTransicaoEstados[i][j] = i;
            acao_matrizTransicaoEstados[i][j] = NENHUMA_ACAO;
        }
    }

    // Mapeamento do Fluxo do Jogo
    proximo_estado_matrizTransicaoEstados[AGUARDANDO][BOTAO_INICIAR_PRESSIONADO] = VERIFICANDO_INCLINACAO;
    acao_matrizTransicaoEstados[AGUARDANDO][BOTAO_INICIAR_PRESSIONADO] = A02_PREPARAR_INCLINACAO;

    proximo_estado_matrizTransicaoEstados[VERIFICANDO_INCLINACAO][INCLINACAO_CORRETA] = VERIFICANDO_LDR;
    acao_matrizTransicaoEstados[VERIFICANDO_INCLINACAO][INCLINACAO_CORRETA] = A03_PREPARAR_LDR;

    proximo_estado_matrizTransicaoEstados[VERIFICANDO_LDR][LDR_COBERTO] = JOGO_GENIUS;
    acao_matrizTransicaoEstados[VERIFICANDO_LDR][LDR_COBERTO] = A04_PREPARAR_GENIUS;

    proximo_estado_matrizTransicaoEstados[JOGO_GENIUS][SEQUENCIA_GENIUS_CORRETA] = ENTRADA_CODIGO_SECRETO;
    acao_matrizTransicaoEstados[JOGO_GENIUS][SEQUENCIA_GENIUS_CORRETA] = A05_PREPARAR_CODIGO;

    proximo_estado_matrizTransicaoEstados[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_CORRETO] = FINALIZADO;
    acao_matrizTransicaoEstados[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_CORRETO] = A08_FINALIZAR_PUZZLE;

    proximo_estado_matrizTransicaoEstados[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_INCORRETO] = ENTRADA_CODIGO_SECRETO;
    acao_matrizTransicaoEstados[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_INCORRETO] = A07_FEEDBACK_ERRO_CODIGO;

    for(int i=0; i < NUM_ESTADOS; i++) {
        proximo_estado_matrizTransicaoEstados[i][BOTAO_RESET_PRESSIONADO] = AGUARDANDO;
        acao_matrizTransicaoEstados[i][BOTAO_RESET_PRESSIONADO] = A09_RESETAR_JOGO;
    }
}

/************************************************************************
 obterEvento
 *************************************************************************/
int obterEvento() {
    if (botaoReset.foiPressionado()) return BOTAO_RESET_PRESSIONADO;

    switch (estadoAtual) {
        case AGUARDANDO:
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
    }
    return NENHUM_EVENTO;
}

/************************************************************************
 obterAcao e obterProximoEstado (Helpers)
 *************************************************************************/
int obterAcao(int estado, int evento) {
    return acao_matrizTransicaoEstados[estado][evento];
}

int obterProximoEstado(int estado, int evento) {
    return proximo_estado_matrizTransicaoEstados[estado][evento];
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
}

void loop() {
  
    if (estadoAtual == VERIFICANDO_INCLINACAO) {
      meuIMU.atualizar(); // Atualiza o filtro AHRS
      meuIMU.imprimirLeituras(); // Imprime no Serial para debug
      meuDisplay.exibirDadosIMU(meuIMU.getRoll(), meuIMU.getPitch()); // Usa o novo método do display

    }
    if (estadoAtual == JOGO_GENIUS) {
        meuJogoGenius.loop();
    }

    if (eventoInterno == NENHUM_EVENTO) {
        codigoEvento = obterEvento();
    } else {
        codigoEvento = eventoInterno;
        eventoInterno = NENHUM_EVENTO;
    }

    if (codigoEvento != NENHUM_EVENTO) {
        codigoAcao = obterAcao(estadoAtual, codigoEvento);
        estadoAtual = obterProximoEstado(estadoAtual, codigoEvento);
        eventoInterno = executarAcao(codigoAcao);
    }
}