#ifndef MODULOS_H
#define MODULOS_H

#include "../game/game.h"

// Funções para gerar módulos de cada tipo
void gerar_modulo_botao(Modulo *mod, Dificuldade dificuldade);
void gerar_modulo_senha(Modulo *mod, Dificuldade dificuldade);
void gerar_modulo_fios(Modulo *mod, Dificuldade dificuldade);

// Funções para validar instruções de cada tipo
int validar_instrucao_botao(const Modulo *mod, const char *instrucao);
int validar_instrucao_senha(const Modulo *mod, const char *instrucao);
int validar_instrucao_fios(const Modulo *mod, const char *instrucao);

// Função genérica para validar instrução baseada no tipo
int validar_instrucao_modulo(const Modulo *mod, const char *instrucao);

// Funções auxiliares para obter informações de exibição
const char* nome_tipo_modulo(TipoModulo tipo);
void obter_info_exibicao_modulo(const Modulo *mod, char *buffer, size_t tamanho);

#endif // MODULOS_H

