// PuzzleBox.ino

// --- 1. Includes para as Classes dos Componentes ---

#include "DisplayOLED.h"       // Para o display OLED
#include "MPU6050.h"           // Para o acelerômetro MPU6050
#include "LDR.h"               // Para o sensor LDR
#include "Button.h"            // Para os botões (iniciar, reset, genius)
#include "LEDController.h"     // Para controlar os LEDs (especialmente do Genius)
#include "Buzzer.h"            // Para o buzzer
#include "Keypad.h"            // Para o teclado
#include "Genius.h"            // Para a lógica do jogo Genius 

// --- 2. Definições do Sistema  ---

// Estados da Máquina 
enum Estado {
    IDLE,                      // Sistema ligado, aguardando início 
    VERIFICANDO_INCLINACAO,    // Jogador ajusta inclinação 
    VERIFICANDO_LDR,           // Jogador ajusta luminosidade no LDR 
    JOGO_GENIUS_MOSTRAR,       // Sistema mostra sequência de LEDs 
    JOGO_GENIUS_JOGADOR,       // Jogador tenta repetir a sequência 
    ENTRADA_CODIGO_SECRETO,    // Jogador digita código no keypad 
    FINALIZADO,                // Puzzle resolvido, caixa aberta 
    NUM_ESTADOS                // Helper para tamanho de arrays
};

// Eventos que podem ocorrer
enum Evento {
    NENHUM_EVENTO = 0,
    BOTAO_INICIAR_PRESSIONADO, // Botão para começar o jogo
    INCLINACAO_CORRETA,        // Acelerômetro detectou inclinação correta 
    LDR_COBERTO,               // LDR detectou "zona escura" 
    SEQUENCIA_GENIUS_MOSTRADA, // Evento interno após Genius mostrar a sequência
    BOTAO_GENIUS_PRESSIONADO,  // Jogador pressionou um botão do Genius
    SEQUENCIA_GENIUS_CORRETA,  // Jogador acertou a sequência Genius
    SEQUENCIA_GENIUS_INCORRETA,// Jogador errou a sequência Genius
    CODIGO_KEYPAD_RECEBIDO,    // Um dígito ou código completo foi inserido
    CODIGO_SECRETO_CORRETO,    // Código secreto validado como correto 
    CODIGO_SECRETO_INCORRETO,  // Código secreto validado como incorreto
    BOTAO_RESET_PRESSIONADO,   // Botão de reset foi pressionado 
    NUM_EVENTOS                // Helper para tamanho de arrays
};

// Ações que a máquina de estados pode executar
enum Acao {
    NENHUMA_ACAO = 0,
    ACAO_INICIALIZAR_DISPLAY_BEMVINDO, // Mostra "Puzzle Box Pronta" 
    ACAO_PREPARAR_DESAFIO_INCLINACAO,  // Prepara e instrui sobre a inclinação
    ACAO_FEEDBACK_INCLINACAO_OK,     // Indica sucesso na inclinação 
    ACAO_PREPARAR_DESAFIO_LDR,         // Prepara e instrui sobre o LDR
    ACAO_FEEDBACK_LDR_OK,            // Indica sucesso no LDR 
    ACAO_MOSTRAR_SEQUENCIA_GENIUS,     // Genius mostra a sequência de LEDs 
    ACAO_AGUARDAR_JOGADOR_GENIUS,      // Prepara para receber input do Genius
    ACAO_PROCESSAR_JOGADA_GENIUS,    // Processa o botão pressionado pelo jogador
    ACAO_FEEDBACK_GENIUS_SUCESSO,    // Indica que a sequência Genius foi correta 
    ACAO_FEEDBACK_GENIUS_ERRO,       // Indica erro e reinicia o Genius (volta para MOSTRAR)
    ACAO_PREPARAR_DESAFIO_CODIGO,      // Prepara para entrada do código secreto
    ACAO_PROCESSAR_ENTRADA_CODIGO,   // Processa os dígitos do keypad
    ACAO_ABRIR_CAIXA_SUCESSO,        // Código correto, "abre" a caixa, toca buzzer 
    ACAO_FEEDBACK_CODIGO_INCORRETO,  // Indica que o código está errado 
    ACAO_RESETAR_JOGO_COMPLETO       // Reseta todo o sistema para o estado IDLE 
};

// --- 3. Variáveis Globais da Máquina de Estados ---
int estadoAtual = IDLE;
int codigoEventoAtual = NENHUM_EVENTO;
int eventoInterno = NENHUM_EVENTO; // Eventos gerados por ações
int codigoAcaoAtual;

// Matrizes de Transição 
int proximoEstadoDaTransicao[NUM_ESTADOS][NUM_EVENTOS];
int acaoDaTransicao[NUM_ESTADOS][NUM_EVENTOS];

// --- 4. Instanciação dos Objetos dos Componentes ---
DisplayOLED displayOLED; // Objeto para o display
MPU6050 acelerometro; // Objeto para o MPU6050
LDR ldrSensor; // Objeto para o LDR

// Exemplo para botões:
Button botaoIniciar(PINO_BOTAO_INICIAR);
Button botaoReset(PINO_BOTAO_RESET);

LEDController ledsGenius; // Ou gerenciado dentro da classe Genius
Buzzer buzzer;
Keypad keypad;

Genius jogoGenius; // Esta classe vai encapsular a lógica do Genius 
                       // e provavelmente usará 'ledsGenius' e objetos 'Button'

// Variável para armazenar o código secreto
const char* CODIGO_PREDEFINIDO = "1234"; // Exemplo de código

// --- 5. Função setup() ---
void setup() {
    Serial.begin(115200); // Taxa de comunicação comum para ESP32
    Serial.println("Iniciando Puzzle Box...");

    // Inicializar todos os componentes
    displayOLED.inicializar(); // (Exemplo de método de inicialização)
    acelerometro.inicializar();
    ldrSensor.inicializar();
    botaoIniciar.inicializar();
    botaoReset.inicializar();
    ledsGenius.inicializar(); // (Se for uma classe separada)
    buzzer.inicializar();
    keypad.inicializar();
    jogoGenius.inicializar(&ledsGenius /*, &botoesGenius*/); // Passar referências se necessário

    // Inicializar a máquina de estados (popular as tabelas)
    iniciaMaquinaEstados();

    // Estado inicial e ação inicial
    estadoAtual = IDLE;
    eventoInterno = NENHUM_EVENTO; // Garante que não haja evento interno pendente
    executarAcao(ACAO_INICIALIZAR_DISPLAY_BEMVINDO); // Mostra "Puzzle Box Pronta"

    Serial.println("Puzzle Box Pronta.");
    displayOLED.exibirMensagem("Puzzle Box", "Pronta!"); //  (Exemplo)
}

// --- 6. Função loop() - Motor da Máquina de Estados ---
void loop() {
    if (eventoInterno == NENHUM_EVENTO) {
        codigoEventoAtual = obterEvento(); // Verifica eventos externos
    } else {
        codigoEventoAtual = eventoInterno; // Processa evento interno primeiro
        eventoInterno = NENHUM_EVENTO;     // Limpa o evento interno
    }

    if (codigoEventoAtual != NENHUM_EVENTO) {
        codigoAcaoAtual = obterAcaoDaTabela(estadoAtual, codigoEventoAtual);
        estadoAtual = obterProximoEstadoDaTabela(estadoAtual, codigoEventoAtual);

        // Log para debug
        Serial.print("Estado: "); Serial.print(estadoAtual);
        Serial.print(" Evento: "); Serial.print(codigoEventoAtual);
        Serial.print(" Acao: "); Serial.println(codigoAcaoAtual);

        eventoInterno = executarAcao(codigoAcaoAtual); // Executa a ação e pode gerar novo evento interno
    }
    delay(10); // Pequeno delay para estabilidade, ajuste conforme necessário
}

// --- 7. Função iniciaMaquinaEstados() ---
void iniciaMaquinaEstados() {
    // Inicializa as matrizes com valores padrão (permanecer no mesmo estado, nenhuma ação)
    for (int i = 0; i < NUM_ESTADOS; i++) {
        for (int j = 0; j < NUM_EVENTOS; j++) {
            proximoEstadoDaTransicao[i][j] = i; // Padrão: permanece no estado
            acaoDaTransicao[i][j] = NENHUMA_ACAO;    // Padrão: nenhuma ação
        }
    }

    // Agora, defina as transições e ações específicas baseadas no seu diagrama de estados
    // Exemplo para o estado IDLE:
    proximoEstadoDaTransicao[IDLE][BOTAO_INICIAR_PRESSIONADO] = VERIFICANDO_INCLINACAO;
    acaoDaTransicao[IDLE][BOTAO_INICIAR_PRESSIONADO] = ACAO_PREPARAR_DESAFIO_INCLINACAO;

    // Exemplo para o estado VERIFICANDO_INCLINACAO:
    proximoEstadoDaTransicao[VERIFICANDO_INCLINACAO][INCLINACAO_CORRETA] = VERIFICANDO_LDR;
    acaoDaTransicao[VERIFICANDO_INCLINACAO][INCLINACAO_CORRETA] = ACAO_FEEDBACK_INCLINACAO_OK; // E ACAO_PREPARAR_DESAFIO_LDR (pode ser uma ação combinada ou duas separadas)

    proximoEstadoDaTransicao[VERIFICANDO_INCLINACAO][BOTAO_RESET_PRESSIONADO] = IDLE;
    acaoDaTransicao[VERIFICANDO_INCLINACAO][BOTAO_RESET_PRESSIONADO] = ACAO_RESETAR_JOGO_COMPLETO;

    // Exemplo para o estado VERIFICANDO_LDR:
    proximoEstadoDaTransicao[VERIFICANDO_LDR][LDR_COBERTO] = JOGO_GENIUS_MOSTRAR;
    acaoDaTransicao[VERIFICANDO_LDR][LDR_COBERTO] = ACAO_FEEDBACK_LDR_OK; // E ACAO_MOSTRAR_SEQUENCIA_GENIUS

    proximoEstadoDaTransicao[VERIFICANDO_LDR][BOTAO_RESET_PRESSIONADO] = IDLE;
    acaoDaTransicao[VERIFICANDO_LDR][BOTAO_RESET_PRESSIONADO] = ACAO_RESETAR_JOGO_COMPLETO;

    // Exemplo para JOGO_GENIUS_MOSTRAR:
    // Este estado pode transitar automaticamente para JOGO_GENIUS_JOGADOR após mostrar a sequência
    // Isso pode ser feito com um evento interno gerado pela ACAO_MOSTRAR_SEQUENCIA_GENIUS
    proximoEstadoDaTransicao[JOGO_GENIUS_MOSTRAR][SEQUENCIA_GENIUS_MOSTRADA] = JOGO_GENIUS_JOGADOR;
    acaoDaTransicao[JOGO_GENIUS_MOSTRAR][SEQUENCIA_GENIUS_MOSTRADA] = ACAO_AGUARDAR_JOGADOR_GENIUS;

    // Exemplo para JOGO_GENIUS_JOGADOR:
    proximoEstadoDaTransicao[JOGO_GENIUS_JOGADOR][SEQUENCIA_GENIUS_CORRETA] = ENTRADA_CODIGO_SECRETO;
    acaoDaTransicao[JOGO_GENIUS_JOGADOR][SEQUENCIA_GENIUS_CORRETA] = ACAO_FEEDBACK_GENIUS_SUCESSO; // E ACAO_PREPARAR_DESAFIO_CODIGO

    proximoEstadoDaTransicao[JOGO_GENIUS_JOGADOR][SEQUENCIA_GENIUS_INCORRETA] = JOGO_GENIUS_MOSTRAR; // Volta a mostrar a sequência 
    acaoDaTransicao[JOGO_GENIUS_JOGADOR][SEQUENCIA_GENIUS_INCORRETA] = ACAO_FEEDBACK_GENIUS_ERRO; // E ACAO_MOSTRAR_SEQUENCIA_GENIUS

    proximoEstadoDaTransicao[JOGO_GENIUS_JOGADOR][BOTAO_RESET_PRESSIONADO] = IDLE;
    acaoDaTransicao[JOGO_GENIUS_JOGADOR][BOTAO_RESET_PRESSIONADO] = ACAO_RESETAR_JOGO_COMPLETO;

    // Exemplo para ENTRADA_CODIGO_SECRETO:
    proximoEstadoDaTransicao[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_CORRETO] = FINALIZADO;
    acaoDaTransicao[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_CORRETO] = ACAO_ABRIR_CAIXA_SUCESSO;

    proximoEstadoDaTransicao[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_INCORRETO] = ENTRADA_CODIGO_SECRETO; // Permanece para nova tentativa 
    acaoDaTransicao[ENTRADA_CODIGO_SECRETO][CODIGO_SECRETO_INCORRETO] = ACAO_FEEDBACK_CODIGO_INCORRETO;

    proximoEstadoDaTransicao[ENTRADA_CODIGO_SECRETO][BOTAO_RESET_PRESSIONADO] = IDLE;
    acaoDaTransicao[ENTRADA_CODIGO_SECRETO][BOTAO_RESET_PRESSIONADO] = ACAO_RESETAR_JOGO_COMPLETO;

    // Exemplo para FINALIZADO:
    proximoEstadoDaTransicao[FINALIZADO][BOTAO_RESET_PRESSIONADO] = IDLE;
    acaoDaTransicao[FINALIZADO][BOTAO_RESET_PRESSIONADO] = ACAO_RESETAR_JOGO_COMPLETO;

    // IMPORTANTE: Adicionar transições de BOTAO_RESET_PRESSIONADO para todos os estados relevantes,
    // levando de volta para IDLE e executando ACAO_RESETAR_JOGO_COMPLETO.
    // O exemplo acima já mostra para alguns estados.
}


// --- 8. Função obterEvento() ---
int obterEvento() {
    // Verificar botão de reset primeiro, pois ele tem prioridade e pode ocorrer em qualquer estado.
    if (botaoReset.foiPressionado()) { //  (Supondo que .foiPressionado() faz debounce e retorna true na borda de subida)
        return BOTAO_RESET_PRESSIONADO;
    }

    // Lógica de detecção de eventos baseada no estado atual
    switch (estadoAtual) {
        case IDLE:
            if (botaoIniciar.foiPressionado()) {
                return BOTAO_INICIAR_PRESSIONADO;
            }
            break;
        case VERIFICANDO_INCLINACAO:
            // Supondo que acelerometro.verificarInclinacaoAlcancada() retorna true se ok 
            if (acelerometro.verificarInclinacaoAlcancada()) {
                return INCLINACAO_CORRETA;
            }
            break;
        case VERIFICANDO_LDR:
            // Supondo que ldrSensor.zonaEscuraDetectada() retorna true se ok 
            if (ldrSensor.zonaEscuraDetectada()) {
                return LDR_COBERTO;
            }
            break;
        case JOGO_GENIUS_MOSTRAR:
            // Normalmente, a ACAO_MOSTRAR_SEQUENCIA_GENIUS geraria um evento interno
            // SEQUENCIA_GENIUS_MOSTRADA quando terminasse.
            // Se não, você pode precisar de um timer aqui ou uma flag na classe Genius.
            break;
        case JOGO_GENIUS_JOGADOR:
            // Aqui você chamaria um método da sua classe Genius para verificar
            // qual botão foi pressionado e se a jogada está correta, incorreta ou em andamento.
            // Ex: int resultadoJogada = jogoGenius.obterResultadoJogada();
            // if (resultadoJogada == Genius::JOGADA_CORRETA_PARCIAL) return BOTAO_GENIUS_PRESSIONADO; (se precisar de feedback por botão)
            // if (resultadoJogada == Genius::SEQUENCIA_COMPLETA_CORRETA) return SEQUENCIA_GENIUS_CORRETA;
            // if (resultadoJogada == Genius::SEQUENCIA_COMPLETA_INCORRETA) return SEQUENCIA_GENIUS_INCORRETA;
            break;
        case ENTRADA_CODIGO_SECRETO:
            // String codigoDigitado = keypad.obterCodigoCompleto(); // Ou processar dígito a dígito
            // if (codigoDigitado.length() > 0) { // Se um código foi finalizado (ex: com '*' ou após N dígitos)
            //    if (codigoDigitado.equals(CODIGO_PREDEFINIDO)) {
            //        return CODIGO_SECRETO_CORRETO;
            //    } else {
            //        return CODIGO_SECRETO_INCORRETO;
            //    }
            // }
            char tecla = keypad.obterTeclaPressionada(); //  (Exemplo)
            if (tecla != NO_KEY) { // NO_KEY é uma constante comum da lib Keypad.h
                // Aqui você acumularia as teclas e verificaria contra CODIGO_PREDEFINIDO
                // Para simplificar, vamos assumir que a classe keypad pode dizer se o código está completo e correto.
                // Este é um ponto que precisará de mais lógica na classe Keypad ou aqui.
                // Ex: if (keypad.codigoPronto()) {
                //          if (keypad.validarCodigo(CODIGO_PREDEFINIDO)) return CODIGO_SECRETO_CORRETO;
                //          else return CODIGO_SECRETO_INCORRETO;
                //      }
            }
            break;
        case FINALIZADO:
            // Geralmente nenhum evento novo, apenas aguarda reset.
            break;
    }
    return NENHUM_EVENTO;
}

// --- 9. Função executarAcao() ---
int executarAcao(int acao) {
    int eventoGeradoInternamente = NENHUM_EVENTO;

    if (acao == NENHUMA_ACAO) {
        return eventoGeradoInternamente;
    }

    switch (acao) {
        case ACAO_INICIALIZAR_DISPLAY_BEMVINDO:
            displayOLED.limpar();
            displayOLED.exibirMensagem("Puzzle Box", "Pronta!", 0, 0); //  (Exemplo)
            // Pode também verificar sensores aqui e exibir "ERRO" se algo falhar 
            break;
        case ACAO_PREPARAR_DESAFIO_INCLINACAO:
            displayOLED.limpar();
            displayOLED.exibirMensagem("Nivel 1:", "Incline a Caixa", 0, 0);
            // acelerometro.iniciarMonitoramento(); // Se necessário
            break;
        case ACAO_FEEDBACK_INCLINACAO_OK:
            displayOLED.limpar();
            displayOLED.exibirMensagem("Nivel 1:", "OK!", 0, 0); // 
            // buzzer.tocarSucessoCurto(); // Feedback sonoro opcional
            // eventoGeradoInternamente = ACAO_PREPARAR_DESAFIO_LDR; // Exemplo de encadear para proxima ação automaticamente
            break;
        case ACAO_PREPARAR_DESAFIO_LDR:
            displayOLED.limpar();
            displayOLED.exibirMensagem("Nivel 2:", "Cubra o Sensor", 0, 0);
            // ldrSensor.iniciarMonitoramento(); // Se necessário
            break;
        case ACAO_FEEDBACK_LDR_OK:
            displayOLED.limpar();
            displayOLED.exibirMensagem("Nivel 2:", "OK!", 0, 0); // 
            // buzzer.tocarSucessoCurto();
            break;
        case ACAO_MOSTRAR_SEQUENCIA_GENIUS:
            displayOLED.limpar();
            displayOLED.exibirMensagem("Nivel 3:", "Observe...", 0, 0);
            // jogoGenius.iniciarNovaSequenciaEMostrar(); //  Este método deve ser bloqueante ou usar callbacks/flags para indicar fim
            // Após mostrar, poderia gerar um evento interno para mudar de estado ou ação
            eventoGeradoInternamente = SEQUENCIA_GENIUS_MOSTRADA; // Transita para JOGO_GENIUS_JOGADOR
            break;
        case ACAO_AGUARDAR_JOGADOR_GENIUS:
            displayOLED.limpar();
            displayOLED.exibirMensagem("Nivel 3:", "Sua Vez!", 0, 0);
            // jogoGenius.prepararParaEntradaJogador();
            break;
        case ACAO_PROCESSAR_JOGADA_GENIUS:
            // A lógica de processamento da jogada Genius provavelmente estará em obterEvento()
            // ou dentro da classe Genius. Esta ação pode ser para dar feedback intermediário
            // ou verificar se o jogo terminou.
            // Ex: if (jogoGenius.jogadorAcertouUltimaJogada()) displayOLED.piscarFeedbackPositivo();
            //     else displayOLED.piscarFeedbackNegativo();
            break;
        case ACAO_FEEDBACK_GENIUS_SUCESSO:
            displayOLED.limpar();
            displayOLED.exibirMensagem("Nivel 3:", "OK!", 0, 0); // 
            // buzzer.tocarSucessoCurto();
            break;
        case ACAO_FEEDBACK_GENIUS_ERRO:
            displayOLED.limpar();
            displayOLED.exibirMensagem("Nivel 3:", "Erro! Tente Novamente.", 0, 0);
            // buzzer.tocarErro();
            // A transição para JOGO_GENIUS_MOSTRAR já cuida de reiniciar.
            break;
        case ACAO_PREPARAR_DESAFIO_CODIGO:
            displayOLED.limpar();
            displayOLED.exibirMensagem("Nivel Final:", "Digite o Codigo", 0, 0);
            // keypad.limparBuffer(); // Prepara para nova entrada
            break;
        case ACAO_PROCESSAR_ENTRADA_CODIGO:
            // Semelhante ao Genius, a maior parte da lógica pode estar em obterEvento()
            // Esta ação pode ser usada para feedback por dígito, se desejado.
            // displayOLED.adicionarDigitoAoDisplay(keypad.ultimoDigito());
            break;
        case ACAO_ABRIR_CAIXA_SUCESSO:
            displayOLED.limpar();
            displayOLED.exibirMensagem("PARABENS!", "CAIXA ABERTA!", 0, 0);
            buzzer.tocarSomVitoria(); // Som longo e LEDs piscando 
            // ledsGenius.piscarTodosPorUmTempo(); // Exemplo
            break;
        case ACAO_FEEDBACK_CODIGO_INCORRETO:
            displayOLED.exibirMensagem("Codigo:", "Incorreto!", 0, 2); // Exibe na linha 2 
            // buzzer.tocarErro();
            // keypad.limparBuffer(); // Para o jogador tentar novamente
            break;
        case ACAO_RESETAR_JOGO_COMPLETO:
            displayOLED.limpar();
            displayOLED.exibirMensagem("Sistema Resetado", "", 0, 0); // 
            // jogoGenius.resetar(); // Reseta estado interno do jogo Genius
            // acelerometro.resetar(); // Se houver algum estado interno
            // ldrSensor.resetar();    // Se houver algum estado interno
            // keypad.limparBuffer();
            delay(1000); // Mostra mensagem de reset por um tempo
            // A transição para IDLE e a ACAO_INICIALIZAR_DISPLAY_BEMVINDO (na próxima iteração do IDLE)
            // finalizarão o reset visual.
            // A primeira ação do estado IDLE (se BOTAO_INICIAR_PRESSIONADO) já reconfiguraria o display.
            // Poderia também chamar ACAO_INICIALIZAR_DISPLAY_BEMVINDO diretamente aqui se desejado.
            executarAcao(ACAO_INICIALIZAR_DISPLAY_BEMVINDO); // Para garantir que o display mostre "Pronta"
            break;
    }
    return eventoGeradoInternamente;
}

// --- 10. Funções Helper para a Tabela de Transição ---
int obterAcaoDaTabela(int estadoConsultar, int eventoConsultar) {
    if (estadoConsultar >= 0 && estadoConsultar < NUM_ESTADOS &&
        eventoConsultar >= 0 && eventoConsultar < NUM_EVENTOS) {
        return acaoDaTransicao[estadoConsultar][eventoConsultar];
    }
    return NENHUMA_ACAO; // Segurança
}

int obterProximoEstadoDaTabela(int estadoConsultar, int eventoConsultar) {
    if (estadoConsultar >= 0 && estadoConsultar < NUM_ESTADOS &&
        eventoConsultar >= 0 && eventoConsultar < NUM_EVENTOS) {
        return proximoEstadoDaTransicao[estadoConsultar][eventoConsultar];
    }
    return estadoConsultar; // Segurança: permanece no estado atual
}
