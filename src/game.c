#include "game.h"
#include "ui.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <ncurses.h>

// Inicializa o estado do jogo com a dificuldade escolhida
void inicializar_jogo(GameState *g, Dificuldade dificuldade, int num_tedax, int num_bancadas) {
    // Validar parâmetros
    if (num_tedax < 1) num_tedax = 1;
    if (num_tedax > 3) num_tedax = 3;
    if (num_bancadas < 1) num_bancadas = 1;
    if (num_bancadas > 3) num_bancadas = 3;
    
    g->dificuldade = dificuldade;
    g->tempo_total_partida = 120;  // 120 segundos de partida
    g->tempo_restante = g->tempo_total_partida;
    g->qtd_modulos = 0;
    g->proximo_id_modulo = 1;
    
    // Inicializar múltiplos tedax
    g->qtd_tedax = num_tedax;
    for (int i = 0; i < num_tedax; i++) {
        g->tedax[i].id = i + 1;
        g->tedax[i].estado = TEDAX_LIVRE;
        g->tedax[i].modulo_atual = -1;
        g->tedax[i].bancada_atual = -1;
    }
    
    // Inicializar múltiplas bancadas
    g->qtd_bancadas = num_bancadas;
    for (int i = 0; i < num_bancadas; i++) {
        g->bancadas[i].id = i + 1;
        g->bancadas[i].estado = BANCADA_LIVRE;
        g->bancadas[i].tedax_ocupando = -1;
    }
    
    // Inicializar semente do rand (deve ser antes de usar rand())
    srand(time(NULL));
    
    // Configuração baseada na dificuldade
    switch (dificuldade) {
        case DIFICULDADE_FACIL:
            g->modulos_necessarios = 4;
            g->intervalo_geracao = 100; // 20 segundos (100 ticks * 0.2s)
            break;
        case DIFICULDADE_MEDIO:
            g->modulos_necessarios = 8;
            g->intervalo_geracao = 75; // 15 segundos (75 ticks * 0.2s)
            break;
        case DIFICULDADE_DIFICIL:
            g->modulos_necessarios = 12;
            g->intervalo_geracao = 50; // 10 segundos (50 ticks * 0.2s)
            break;
    }
    
    g->max_modulos = g->modulos_necessarios; // máximo = necessário para vencer
    g->ticks_desde_ultimo_modulo = 0;
    g->jogo_rodando = 1;
    g->jogo_terminou = 0;
    
    // Inicializar mutex e condition variables
    pthread_mutex_init(&g->mutex_jogo, NULL);
    pthread_cond_init(&g->cond_modulo_disponivel, NULL);
    pthread_cond_init(&g->cond_bancada_disponivel, NULL);
    pthread_cond_init(&g->cond_tela_atualizada, NULL);
    
    // Gerar primeiro módulo imediatamente
    pthread_mutex_lock(&g->mutex_jogo);
    gerar_novo_modulo(g);
    pthread_mutex_unlock(&g->mutex_jogo);
}

// Finaliza o jogo e libera recursos
void finalizar_jogo(GameState *g) {
    g->jogo_rodando = 0;
    
    // Destruir mutex e condition variables
    pthread_mutex_destroy(&g->mutex_jogo);
    pthread_cond_destroy(&g->cond_modulo_disponivel);
    pthread_cond_destroy(&g->cond_bancada_disponivel);
    pthread_cond_destroy(&g->cond_tela_atualizada);
}

// Gera um novo módulo e adiciona ao jogo
// NOTA: Deve ser chamada com mutex_jogo já travado
void gerar_novo_modulo(GameState *g) {
    if (g->qtd_modulos >= 100) {
        return; // Limite máximo de módulos atingido
    }
    
    Modulo *novo = &g->modulos[g->qtd_modulos];
    
    // Atribuir ID
    novo->id = g->proximo_id_modulo++;
    
    // Sortear cor (0 = Vermelho, 1 = Verde, 2 = Azul)
    int cor_aleatoria = rand() % 3;
    novo->cor = (CorBotao)cor_aleatoria;
    
    // Definir tempo total (entre 3 e 8 segundos)
    novo->tempo_total = 3 + (rand() % 6);
    novo->tempo_restante = novo->tempo_total;
    
    // Definir instrução correta baseada na cor
    switch (novo->cor) {
        case COR_VERMELHO:
            strcpy(novo->instrucao_correta, "p");
            break;
        case COR_VERDE:
            strcpy(novo->instrucao_correta, "pp");
            break;
        case COR_AZUL:
            strcpy(novo->instrucao_correta, "ppp");
            break;
    }
    
    // Limpar instrução digitada
    novo->instrucao_digitada[0] = '\0';
    
    // Estado inicial: pendente
    novo->estado = MOD_PENDENTE;
    novo->tempo_desde_resolvido = -1; // -1 significa não resolvido ainda
    
    g->qtd_modulos++;
    
    // Sinalizar que há um novo módulo disponível
    pthread_cond_broadcast(&g->cond_modulo_disponivel);
}

// Função removida - lógica movida para thread_mural

// Função removida - lógica movida para thread_tedax

// Conta quantos módulos foram resolvidos
// NOTA: Deve ser chamada com mutex_jogo já travado
int contar_modulos_resolvidos(const GameState *g) {
    int resolvidos = 0;
    for (int i = 0; i < g->qtd_modulos; i++) {
        if (g->modulos[i].estado == MOD_RESOLVIDO) {
            resolvidos++;
        }
    }
    return resolvidos;
}

// Verifica se há módulos pendentes
// NOTA: Deve ser chamada com mutex_jogo já travado
int tem_modulos_pendentes(const GameState *g) {
    for (int i = 0; i < g->qtd_modulos; i++) {
        if (g->modulos[i].estado == MOD_PENDENTE) {
            return 1; // Há pelo menos um módulo pendente
        }
    }
    return 0; // Não há módulos pendentes
}

// Verifica se todos os módulos necessários foram resolvidos
// NOTA: Deve ser chamada com mutex_jogo já travado
int todos_modulos_resolvidos(const GameState *g) {
    int resolvidos = contar_modulos_resolvidos(g);
    // Vitória quando resolver pelo menos o número necessário de módulos
    return (resolvidos >= g->modulos_necessarios);
}

// Retorna o nome da cor como string
const char* nome_cor(CorBotao cor) {
    switch (cor) {
        case COR_VERMELHO:
            return "Vermelho";
        case COR_VERDE:
            return "Verde";
        case COR_AZUL:
            return "Azul";
        default:
            return "Desconhecida";
    }
}

// Retorna o estado do módulo como string
const char* nome_estado_modulo(EstadoModulo estado) {
    switch (estado) {
        case MOD_PENDENTE:
            return "PENDENTE";
        case MOD_EM_EXECUCAO:
            return "EM_EXECUCAO";
        case MOD_RESOLVIDO:
            return "RESOLVIDO";
        default:
            return "DESCONHECIDO";
    }
}

// Retorna o nome da dificuldade como string
const char* nome_dificuldade(Dificuldade dificuldade) {
    switch (dificuldade) {
        case DIFICULDADE_FACIL:
            return "FACIL";
        case DIFICULDADE_MEDIO:
            return "MEDIO";
        case DIFICULDADE_DIFICIL:
            return "DIFICIL";
        default:
            return "DESCONHECIDA";
    }
}

// ============================================================================
// IMPLEMENTAÇÃO DAS THREADS
// ============================================================================

// Thread do Mural de Módulos Pendentes
void* thread_mural(void* arg) {
    GameState *g = (GameState*)arg;
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 200000000L; // 0.2 segundos
    
    while (g->jogo_rodando && !g->jogo_terminou) {
        pthread_mutex_lock(&g->mutex_jogo);
        
        if (g->qtd_modulos < g->max_modulos) {
            g->ticks_desde_ultimo_modulo++;
            
            if (g->ticks_desde_ultimo_modulo >= g->intervalo_geracao) {
                gerar_novo_modulo(g);
                g->ticks_desde_ultimo_modulo = 0;
            }
        }
        
        // Se não há módulos pendentes e ainda não gerou o máximo, gerar imediatamente
        if (!tem_modulos_pendentes(g) && g->qtd_modulos < g->max_modulos) {
            gerar_novo_modulo(g);
        }
        
        pthread_mutex_unlock(&g->mutex_jogo);
        
        nanosleep(&ts, NULL);
    }
    
    return NULL;
}

// Thread de Exibição de Informações
void* thread_exibicao(void* arg) {
    GameState *g = (GameState*)arg;
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 200000000L; // 0.2 segundos
    
    // Buffer de instrução compartilhado (gerenciado pela thread coordenador)
    extern char buffer_instrucao_global[16];
    
    // Inicializar ncurses nesta thread
    inicializar_ncurses();
    
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    }
    
    while (g->jogo_rodando && !g->jogo_terminou) {
        pthread_mutex_lock(&g->mutex_jogo);
        
        // Desenhar tela
        desenhar_tela(g, buffer_instrucao_global);
        
        pthread_mutex_unlock(&g->mutex_jogo);
        
        nanosleep(&ts, NULL);
    }
    
    finalizar_ncurses();
    
    return NULL;
}

// Thread de um Tedax
void* thread_tedax(void* arg) {
    typedef struct {
        GameState *g;
        int tedax_id;
    } TedaxArgs;
    
    TedaxArgs *args = (TedaxArgs*)arg;
    GameState *g = args->g;
    int tedax_id = args->tedax_id;
    Tedax *tedax = &g->tedax[tedax_id];
    
    struct timespec ts;
    ts.tv_sec = 1;  // 1 segundo
    ts.tv_nsec = 0;
    
    while (g->jogo_rodando && !g->jogo_terminou) {
        pthread_mutex_lock(&g->mutex_jogo);
        
        // Incrementar tempo desde resolvido para todos os módulos resolvidos
        for (int i = 0; i < g->qtd_modulos; i++) {
            if (g->modulos[i].estado == MOD_RESOLVIDO && g->modulos[i].tempo_desde_resolvido >= 0) {
                g->modulos[i].tempo_desde_resolvido++;
            }
        }
        
        if (tedax->estado == TEDAX_OCUPADO && tedax->modulo_atual >= 0) {
            Modulo *mod = &g->modulos[tedax->modulo_atual];
            
            // Decrementar tempo restante
            mod->tempo_restante--;
            
            // Quando o tempo acabar, verificar se a instrução estava correta
            if (mod->tempo_restante <= 0) {
            // Comparar instrução digitada com a correta
            if (strcmp(mod->instrucao_digitada, mod->instrucao_correta) == 0) {
                // Instrução correta: módulo resolvido
                mod->estado = MOD_RESOLVIDO;
                mod->tempo_desde_resolvido = 0; // Iniciar contador de tempo desde resolvido
            } else {
                // Instrução incorreta: módulo volta para o mural
                mod->estado = MOD_PENDENTE;
                mod->tempo_restante = mod->tempo_total; // Resetar tempo
                mod->instrucao_digitada[0] = '\0';      // Limpar instrução
                mod->tempo_desde_resolvido = -1;         // Resetar contador
            }
                
                    // Liberar bancada
                if (tedax->bancada_atual >= 0) {
                    g->bancadas[tedax->bancada_atual].estado = BANCADA_LIVRE;
                    g->bancadas[tedax->bancada_atual].tedax_ocupando = -1;
                    pthread_cond_broadcast(&g->cond_bancada_disponivel);
                }
                
                // Liberar tedax
                tedax->estado = TEDAX_LIVRE;
                tedax->modulo_atual = -1;
                tedax->bancada_atual = -1;
                
                // Se não há módulos pendentes e ainda não gerou o máximo, gerar um novo imediatamente
                if (!tem_modulos_pendentes(g) && g->qtd_modulos < g->max_modulos) {
                    gerar_novo_modulo(g);
                }
                
                // Sinalizar que há módulo disponível novamente
                pthread_cond_broadcast(&g->cond_modulo_disponivel);
            }
        }
        
        pthread_mutex_unlock(&g->mutex_jogo);
        
        nanosleep(&ts, NULL);
    }
    
    free(args);
    return NULL;
}

// Thread do Coordenador (Jogador)
void* thread_coordenador(void* arg) {
    GameState *g = (GameState*)arg;
    extern char buffer_instrucao_global[16];
    int buffer_len = 0;
    
    while (g->jogo_rodando && !g->jogo_terminou) {
        int ch = getch();
        
        if (ch == ERR) {
            // Nenhuma tecla pressionada
        } else if (ch == 'q' || ch == 'Q') {
            pthread_mutex_lock(&g->mutex_jogo);
            g->jogo_rodando = 0;
            pthread_mutex_unlock(&g->mutex_jogo);
            break;
        } else {
            pthread_mutex_lock(&g->mutex_jogo);
            
            // Verificar se há tedax livre
            int tedax_livre = -1;
            for (int i = 0; i < g->qtd_tedax; i++) {
                if (g->tedax[i].estado == TEDAX_LIVRE) {
                    tedax_livre = i;
                    break;
                }
            }
            
            if (tedax_livre >= 0) {
                // Processar BACKSPACE
                if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
                    if (buffer_len > 0) {
                        buffer_len--;
                        buffer_instrucao_global[buffer_len] = '\0';
                    }
                }
                // Processar ENTER para enviar instrução
                else if (ch == '\n' || ch == '\r') {
                    // Procurar o primeiro módulo pendente
                    int modulo_encontrado = -1;
                    for (int i = 0; i < g->qtd_modulos; i++) {
                        if (g->modulos[i].estado == MOD_PENDENTE) {
                            modulo_encontrado = i;
                            break;
                        }
                    }
                    
                    if (modulo_encontrado >= 0) {
                        // Procurar bancada livre
                        int bancada_livre = -1;
                        for (int i = 0; i < g->qtd_bancadas; i++) {
                            if (g->bancadas[i].estado == BANCADA_LIVRE) {
                                bancada_livre = i;
                                break;
                            }
                        }
                        
                        if (bancada_livre >= 0) {
                            // Designar módulo para o tedax
                            Modulo *mod = &g->modulos[modulo_encontrado];
                            Tedax *t = &g->tedax[tedax_livre];
                            
                            // Copiar instrução digitada
                            strncpy(mod->instrucao_digitada, buffer_instrucao_global, 15);
                            mod->instrucao_digitada[15] = '\0';
                            
                            // Mudar estado do módulo
                            mod->estado = MOD_EM_EXECUCAO;
                            mod->tempo_restante = mod->tempo_total;
                            
                            // Ocupar tedax e bancada
                            t->estado = TEDAX_OCUPADO;
                            t->modulo_atual = modulo_encontrado;
                            t->bancada_atual = bancada_livre;
                            
                            g->bancadas[bancada_livre].estado = BANCADA_OCUPADA;
                            g->bancadas[bancada_livre].tedax_ocupando = t->id;
                            
                            // Limpar buffer
                            buffer_len = 0;
                            buffer_instrucao_global[0] = '\0';
                        }
                    }
                }
                // Aceitar qualquer caractere imprimível
                else if (ch >= 32 && ch <= 126) {
                    if (buffer_len < 15) {
                        buffer_instrucao_global[buffer_len] = (char)ch;
                        buffer_len++;
                        buffer_instrucao_global[buffer_len] = '\0';
                    }
                }
            }
            
            pthread_mutex_unlock(&g->mutex_jogo);
        }
        
        // Pequeno delay para não sobrecarregar
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 50000000L; // 50ms
        nanosleep(&ts, NULL);
    }
    
    return NULL;
}

