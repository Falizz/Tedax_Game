#include "modulos.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// Mapeamento de letras A-I para números usando matrizes 3x3
// Matriz de letras (sempre a mesma):
// A | B | C
// D | E | F
// G | H | I

// Matriz de números (única para todas as dificuldades):
// 9 | 5 | 2
// 6 | 8 | 1
// 7 | 3 | 4
static const char mapeamento_hash[9] = {
    '9', '5', '2',  // A, B, C
    '6', '8', '1',  // D, E, F
    '7', '3', '4'   // G, H, I
};

// Gera uma sequência aleatória de letras A-I com tamanho baseado na dificuldade
static void gerar_sequencia_senha(char *buffer, int tamanho_max, Dificuldade dificuldade) {
    int tamanho;
    switch (dificuldade) {
        case DIFICULDADE_FACIL:
            tamanho = 3 + (rand() % 2); // 3 ou 4 caracteres
            break;
        case DIFICULDADE_MEDIO:
            tamanho = 4 + (rand() % 2); // 4 ou 5 caracteres
            break;
        case DIFICULDADE_DIFICIL:
            tamanho = 5 + (rand() % 2); // 5 ou 6 caracteres
            break;
        default:
            tamanho = 4;
            break;
    }
    
    // Garantir que não exceda o tamanho máximo
    if (tamanho > tamanho_max - 1) {
        tamanho = tamanho_max - 1;
    }
    
    // Gerar sequência aleatória usando apenas A-I
    for (int i = 0; i < tamanho; i++) {
        buffer[i] = 'A' + (rand() % 9); // A até I (0-8)
    }
    buffer[tamanho] = '\0';
}

// Cores para módulo de fios
static const char* cores_fios[] = {"R", "G", "B", "Y", "W", "K"}; // Vermelho, Verde, Azul, Amarelo, Branco, Preto


// Gera um módulo de botão
void gerar_modulo_botao(Modulo *mod, Dificuldade dificuldade) {
    (void)dificuldade; // Parâmetro não usado, mas mantido para consistência
    mod->tipo = TIPO_BOTAO;
    
    // Sortear cor (0 = Vermelho, 1 = Verde, 2 = Azul)
    int cor_aleatoria = rand() % 3;
    mod->dados.botao.cor = (CorBotao)cor_aleatoria;
    
    // Definir instrução correta baseada na cor
    switch (mod->dados.botao.cor) {
        case COR_VERMELHO:
            strcpy(mod->instrucao_correta, "p");
            break;
        case COR_VERDE:
            strcpy(mod->instrucao_correta, "pp");
            break;
        case COR_AZUL:
            strcpy(mod->instrucao_correta, "ppp");
            break;
    }
}

// Gera um módulo de senha
void gerar_modulo_senha(Modulo *mod, Dificuldade dificuldade) {
    mod->tipo = TIPO_SENHA;
    
    // Gerar sequência aleatória de letras A-I com tamanho baseado na dificuldade
    gerar_sequencia_senha(mod->dados.senha.hash, 32, dificuldade);
    
    // Gerar senha numérica baseada no mapeamento
    const char* mapeamento = mapeamento_hash;
    int len = strlen(mod->dados.senha.hash);
    mod->dados.senha.senha_correta[0] = '\0';
    
    for (int i = 0; i < len; i++) {
        char c = mod->dados.senha.hash[i];
        if (c >= 'A' && c <= 'I') {
            char num = mapeamento[c - 'A'];
            char num_str[2] = {num, '\0'};
            strcat(mod->dados.senha.senha_correta, num_str);
        }
    }
    
    // Armazenar índice de dificuldade para validação
    mod->dados.senha.mapeamento[0] = (char)dificuldade;
    mod->dados.senha.mapeamento[1] = '\0';
    
    // A instrução correta é a senha numérica
    strcpy(mod->instrucao_correta, mod->dados.senha.senha_correta);
}

// Padrões de regras para módulo de fios
// Padrão 0: Cortar o primeiro fio de cor primária (R, G, B)
// Padrão 1: Cortar o último fio se houver fio amarelo
// Padrão 2: Cortar o segundo fio se não houver fio preto
// Padrão 3: Cortar o fio de cor que aparece mais vezes
// Padrão 4: Cortar o fio na posição do número de fios dividido por 2 (arredondado)

static void aplicar_padrao_fios(Modulo *mod) {
    char sequencia[32];
    strcpy(sequencia, mod->dados.fios.sequencia);
    
    // Contar fios (separados por /)
    int num_fios = 0;
    char* token = strtok(sequencia, "/");
    char cores[10][4];
    while (token != NULL && num_fios < 10) {
        strcpy(cores[num_fios], token);
        num_fios++;
        token = strtok(NULL, "/");
    }
    
    int padrao = mod->dados.fios.padrao;
    int posicao_cortar = -1;
    
    switch (padrao) {
        case 0: {
            // Cortar o primeiro fio de cor primária (R, G, B)
            for (int i = 0; i < num_fios; i++) {
                if (strcmp(cores[i], "R") == 0 || strcmp(cores[i], "G") == 0 || strcmp(cores[i], "B") == 0) {
                    posicao_cortar = i + 1; // +1 porque posições começam em 1
                    break;
                }
            }
            if (posicao_cortar == -1) posicao_cortar = 1; // Default: primeiro fio
            break;
        }
        case 1: {
            // Cortar o último fio se houver fio amarelo
            int tem_amarelo = 0;
            for (int i = 0; i < num_fios; i++) {
                if (strcmp(cores[i], "Y") == 0) {
                    tem_amarelo = 1;
                    break;
                }
            }
            posicao_cortar = tem_amarelo ? num_fios : 1;
            break;
        }
        case 2: {
            // Cortar o segundo fio se não houver fio preto
            int tem_preto = 0;
            for (int i = 0; i < num_fios; i++) {
                if (strcmp(cores[i], "K") == 0) {
                    tem_preto = 1;
                    break;
                }
            }
            posicao_cortar = tem_preto ? 1 : 2;
            break;
        }
        case 3: {
            // Cortar o fio de cor que aparece mais vezes
            int contadores[6] = {0}; // R, G, B, Y, W, K
            for (int i = 0; i < num_fios; i++) {
                if (strcmp(cores[i], "R") == 0) contadores[0]++;
                else if (strcmp(cores[i], "G") == 0) contadores[1]++;
                else if (strcmp(cores[i], "B") == 0) contadores[2]++;
                else if (strcmp(cores[i], "Y") == 0) contadores[3]++;
                else if (strcmp(cores[i], "W") == 0) contadores[4]++;
                else if (strcmp(cores[i], "K") == 0) contadores[5]++;
            }
            
            int max_idx = 0;
            for (int i = 1; i < 6; i++) {
                if (contadores[i] > contadores[max_idx]) {
                    max_idx = i;
                }
            }
            
            // Encontrar primeira posição da cor mais frequente
            const char* cor_alvo = cores_fios[max_idx];
            for (int i = 0; i < num_fios; i++) {
                if (strcmp(cores[i], cor_alvo) == 0) {
                    posicao_cortar = i + 1;
                    break;
                }
            }
            if (posicao_cortar == -1) posicao_cortar = 1;
            break;
        }
        case 4: {
            // Cortar o fio na posição do número de fios dividido por 2 (arredondado)
            posicao_cortar = (num_fios + 1) / 2;
            break;
        }
    }
    
    // Converter posição para string
    sprintf(mod->instrucao_correta, "%d", posicao_cortar);
}

// Gera um módulo de fios
void gerar_modulo_fios(Modulo *mod, Dificuldade dificuldade) {
    (void)dificuldade; // Parâmetro não usado, mas mantido para consistência
    mod->tipo = TIPO_FIOS;
    
    // Escolher padrão aleatório (0-4)
    mod->dados.fios.padrao = rand() % 5;
    
    // Gerar sequência de fios (5 fios)
    int num_fios = 5;
    mod->dados.fios.sequencia[0] = '\0';
    
    for (int i = 0; i < num_fios; i++) {
        int cor_idx = rand() % 6; // 6 cores disponíveis
        strcat(mod->dados.fios.sequencia, "/");
        strcat(mod->dados.fios.sequencia, cores_fios[cor_idx]);
    }
    strcat(mod->dados.fios.sequencia, "/");
    
    // Aplicar padrão para determinar instrução correta
    aplicar_padrao_fios(mod);
}

// Valida instrução para módulo de botão
int validar_instrucao_botao(const Modulo *mod, const char *instrucao) {
    return strcmp(instrucao, mod->instrucao_correta) == 0;
}

// Valida instrução para módulo de senha
int validar_instrucao_senha(const Modulo *mod, const char *instrucao) {
    return strcmp(instrucao, mod->instrucao_correta) == 0;
}

// Valida instrução para módulo de fios
int validar_instrucao_fios(const Modulo *mod, const char *instrucao) {
    return strcmp(instrucao, mod->instrucao_correta) == 0;
}

// Função genérica para validar instrução
int validar_instrucao_modulo(const Modulo *mod, const char *instrucao) {
    switch (mod->tipo) {
        case TIPO_BOTAO:
            return validar_instrucao_botao(mod, instrucao);
        case TIPO_SENHA:
            return validar_instrucao_senha(mod, instrucao);
        case TIPO_FIOS:
            return validar_instrucao_fios(mod, instrucao);
        default:
            return 0;
    }
}

// Retorna o nome do tipo de módulo
const char* nome_tipo_modulo(TipoModulo tipo) {
    switch (tipo) {
        case TIPO_BOTAO:
            return "Botao";
        case TIPO_SENHA:
            return "Senha";
        case TIPO_FIOS:
            return "Fios";
        default:
            return "Desconhecido";
    }
}

// Obtém informações para exibição do módulo
void obter_info_exibicao_modulo(const Modulo *mod, char *buffer, size_t tamanho) {
    switch (mod->tipo) {
        case TIPO_BOTAO: {
            const char* cor_nome = (mod->dados.botao.cor == COR_VERMELHO) ? "Vermelho" :
                                   (mod->dados.botao.cor == COR_VERDE) ? "Verde" : "Azul";
            snprintf(buffer, tamanho, "%s %s", nome_tipo_modulo(mod->tipo), cor_nome);
            break;
        }
        case TIPO_SENHA: {
            snprintf(buffer, tamanho, "%s Hash: %s", nome_tipo_modulo(mod->tipo), mod->dados.senha.hash);
            break;
        }
        case TIPO_FIOS: {
            snprintf(buffer, tamanho, "%s %s (Padrao %d)", nome_tipo_modulo(mod->tipo), 
                    mod->dados.fios.sequencia, mod->dados.fios.padrao);
            break;
        }
        default:
            snprintf(buffer, tamanho, "Desconhecido");
            break;
    }
}

