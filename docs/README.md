# Keep Solving and Nobody Explodes
Versão básica sequencial do jogo de desarmamento de bombas, implementada em C com ncurses.


## Requisitos

- Compilador GCC
- Biblioteca ncurses (desenvolvimento)

### Instalação do ncurses (Ubuntu/Debian)

```bash
sudo apt-get install libncurses5-dev libncursesw5-dev
```

## Compilação

### Usando Makefile

```bash
make
```

### Compilação manual

```bash
gcc -Wall -Wextra -std=c11 -Isrc src/main.c src/game.c src/ui.c -o jogo -lncurses
```

## Execução

```bash
./jogo
```

## Como Jogar

1. Ao iniciar o jogo, você verá um **menu de seleção de dificuldade** com três opções:
   - **Fácil**: 4 módulos, 1 a cada 20 segundos
   - **Médio**: 8 módulos, 1 a cada 15 segundos
   - **Difícil**: 12 módulos, 1 a cada 10 segundos
2. Use as setas do teclado para navegar e ENTER para selecionar a dificuldade.
3. O jogo começa com 1 módulo já gerado.
4. Cada módulo tem uma cor e requer uma sequência específica, que no arquivo de instruções está expecificado ao jogador
5. Digite a sequência de caracteres instruida pelo arquivo de instruções e pressione ENTER para designar o módulo ao tedax.
6. O tedax ocupará a bancada pelo tempo necessário para desarmar o módulo.
7. Se a instrução estiver correta, o módulo é desarmado. Se estiver incorreta, ele volta para o mural como ultimo da fila.
8. O jogo termina quando:
   - Todos os módulos necessários são desarmados (VITÓRIA)
   - O tempo acaba e ainda há módulos pendentes (DERROTA)

## Controles

- `BACKSPACE`: Remove o último caractere da instrução
- `ENTER`: Envia a instrução para o tedax (se ele estiver livre)
- `q`: Sair do jogo (força fim imediato)

## Estrutura do Projeto

```
bomb_game/
├── src/              # Código fonte
│   ├── main.c        # Loop principal e processamento de entrada do jogador (futuro Coordenador)
│   ├── game.h        # Definições e estruturas do jogo
│   ├── game.c        # Lógica do jogo, geração de módulos (futuro Mural) e atualização do tedax
│   ├── ui.h          # Definições da interface
│   └── ui.c          # Interface ncurses e exibição de informações (futura thread de Exibição)
├── docs/             # Documentação
│   ├── README.md     # Este arquivo
│   └── instrucoes.md # Instruções de jogo
├── Makefile          # Arquivo de compilação
└── .gitignore        # Arquivos ignorados pelo git
```

## Notas

- **Mural de Módulos Pendentes**: `gerar_novo_modulo()` e `atualizar_mural()`
- **Exibição de Informações**: `desenhar_tela()`
- **Coordenador (Jogador)**: `processar_entrada()`
- **Tedax**: `atualizar_tedax()`

