#ifndef GAME_H
#define GAME_H

#include <pthread.h>
#include <semaphore.h>

// Estados possíveis de um módulo
typedef enum {
    MOD_PENDENTE,
    MOD_EM_EXECUCAO,
    MOD_RESOLVIDO
} EstadoModulo;

// Tipos de módulos disponíveis
typedef enum {
    TIPO_BOTAO,
    TIPO_SENHA,
    TIPO_FIOS
} TipoModulo;

// Cores possíveis do botão
typedef enum {
    COR_VERMELHO,
    COR_VERDE,
    COR_AZUL
} CorBotao;

// Cores para módulo de fios
typedef enum {
    COR_FIO_VERMELHO = 0,
    COR_FIO_VERDE = 1,
    COR_FIO_AZUL = 2,
    COR_FIO_AMARELO = 3,
    COR_FIO_BRANCO = 4,
    COR_FIO_PRETO = 5
} CorFio;

// Estados possíveis do tedax
typedef enum {
    TEDAX_LIVRE,
    TEDAX_OCUPADO,
    TEDAX_ESPERANDO
} EstadoTedax;

// Estados possíveis de uma bancada
typedef enum {
    BANCADA_LIVRE,
    BANCADA_OCUPADA
} EstadoBancada;

// Dificuldades do jogo
typedef enum {
    DIFICULDADE_FACIL,
    DIFICULDADE_MEDIO,
    DIFICULDADE_DIFICIL
} Dificuldade;

// Dados específicos do módulo de botão
typedef struct {
    CorBotao cor;
} DadosBotao;

// Dados específicos do módulo de senha
typedef struct {
    char hash[32];              // hash exibido (ex: "SAESI")
    char senha_correta[16];     // senha numérica correta (ex: "54351")
    char mapeamento[256];       // mapeamento de caracteres para números (para validação)
} DadosSenha;

// Dados específicos do módulo de fios
typedef struct {
    char sequencia[32];         // sequência de cores (ex: "/R/G/B/Y/W/")
    int padrao;                 // padrão de regra usado (0-4)
    char instrucao_correta[16]; // instrução correta baseada no padrão
} DadosFios;

// Union para dados específicos de cada tipo de módulo
typedef union {
    DadosBotao botao;
    DadosSenha senha;
    DadosFios fios;
} DadosModulo;

// Estrutura que representa um módulo da bomba
typedef struct {
    int id;
    TipoModulo tipo;            // tipo do módulo
    int tempo_total;            // tempo necessário para desarmar (em segundos)
    int tempo_restante;         // tempo restante quando estiver em execução
    EstadoModulo estado;
    
    DadosModulo dados;          // dados específicos do tipo de módulo
    char instrucao_correta[32]; // instrução correta (pode variar por tipo)
    char instrucao_digitada[32]; // o que o jogador enviou para este módulo
    
    int tempo_desde_resolvido;
} Modulo;

// Estrutura que representa um tedax
typedef struct {
    int id;
    EstadoTedax estado;
    int modulo_atual;           // índice do módulo que está desarmando, ou -1 se livre
    int bancada_atual;          // índice da bancada que está usando, ou -1 se livre
    pthread_t thread_id;        // ID da thread deste tedax
    
    // Fila de módulos em espera para este tedax
    int fila_modulos[1];        // índice do módulo em espera (máximo 1 módulo)
    int qtd_fila;               // quantidade de módulos na fila (0 ou 1)
} Tedax;

// Estrutura que representa uma bancada
typedef struct {
    int id;
    EstadoBancada estado;
    int tedax_ocupando;         // ID do tedax que está usando, ou -1 se livre
    int tedax_esperando;        // ID do tedax que está esperando por esta bancada, ou -1 se nenhum
} Bancada;

// Estado geral do jogo
typedef struct {
    Dificuldade dificuldade;    // dificuldade escolhida
    int tempo_total_partida;    // tempo total da partida em segundos
    int tempo_restante;         // tempo restante da partida
    
    Modulo modulos[100];        // fila de módulos
    int qtd_modulos;            // quantidade de módulos criados
    int proximo_id_modulo;      // próximo ID a ser atribuído
    int modulos_necessarios;    // número de módulos necessários para vencer
    
    // Múltiplos tedax e bancadas
    Tedax tedax[5];             // até 5 tedax
    Bancada bancadas[5];        // até 5 bancadas
    int qtd_tedax;              // quantidade de tedax disponíveis
    int qtd_bancadas;           // quantidade de bancadas disponíveis
    
    // Controle de geração de módulos
    int ticks_desde_ultimo_modulo; // ticks desde o último módulo gerado
    int intervalo_geracao;      // intervalo entre gerações (em ticks)
    int max_modulos;            // máximo de módulos a gerar (igual a modulos_necessarios)
    
    // Controle do jogo
    int jogo_rodando;           // flag para indicar se o jogo está rodando
    int jogo_terminou;          // flag para indicar se o jogo terminou (vitória ou derrota)
    
    // Sincronização
    pthread_mutex_t mutex_jogo; // mutex para proteger o estado do jogo
    pthread_cond_t cond_modulo_disponivel; // condition variable para quando há módulo disponível
    pthread_cond_t cond_bancada_disponivel; // condition variable para quando há bancada disponível
    pthread_cond_t cond_tela_atualizada;    // condition variable para atualizar a tela
    
    char mensagem_erro[64]; // mensagem de erro para exibir na UI
    int erros_cometidos;    // contador de erros (instruções incorretas)
} GameState;

// Funções do jogo

// Inicializa o estado do jogo com a dificuldade escolhida
// num_tedax: número de tedax (1-3)
// num_bancadas: número de bancadas (1-3)
void inicializar_jogo(GameState *g, Dificuldade dificuldade, int num_tedax, int num_bancadas);

// Finaliza o jogo e libera recursos
void finalizar_jogo(GameState *g);

// FUTURO: Esta função será uma thread (Mural de Módulos Pendentes)
// Gera um novo módulo e adiciona ao jogo
void gerar_novo_modulo(GameState *g);

// Função removida - lógica movida para thread_mural

// Função removida - lógica movida para thread_tedax

// Verifica se todos os módulos foram resolvidos
int todos_modulos_resolvidos(const GameState *g);

// Conta quantos módulos foram resolvidos
int contar_modulos_resolvidos(const GameState *g);

// Verifica se há módulos pendentes
int tem_modulos_pendentes(const GameState *g);

// Retorna o nome da cor como string
const char* nome_cor(CorBotao cor);

// Retorna o estado do módulo como string
const char* nome_estado_modulo(EstadoModulo estado);

// Retorna o nome da dificuldade como string
const char* nome_dificuldade(Dificuldade dificuldade);

// Funções de thread (serão chamadas pelas threads)
void* thread_mural(void* arg);
void* thread_exibicao(void* arg);
void* thread_tedax(void* arg);
void* thread_coordenador(void* arg);

// Função auxiliar para processar comando do formato T1B1M1:ppp
// Retorna 1 se sucesso, 0 se falha
// Parâmetros de saída: tedax_idx, bancada_idx, modulo_idx, instrucao
int processar_comando(const char* buffer, GameState *g, 
                      int *tedax_idx, int *bancada_idx, int *modulo_idx, 
                      char *instrucao);

#endif // GAME_H

