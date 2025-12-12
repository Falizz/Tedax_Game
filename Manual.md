# Manual de Instruções

O jogo possui módulos que precisam ser desarmados. Cada módulo necessita de uma instrução expecifica para ser resolvida baseada em algumas pistas, neste manual você vai ver como identificar os módulos e pegar suas instruções para serem informadas aos tedaxes.

Formato do input: `T<tedax>B<bancada>M<modulo>:<instrucao>`

---

### Módulo de Botão

Cada módulo de botão possui uma cor e você deve apertar ele uma cenrta quantidade de vezes para conseguir resolver esse módulo, para enviar a instrução para o tedax desarmá-lo você deve digitar p a quantidade de vezes que é necessária apertar o botão, segue os casos:

- **Vermelho** → Digite `p` e pressione ENTER
  - Requer **1 aperto** do botão
  - Instrução: `p`

- **Verde** → Digite `pp` e pressione ENTER
  - Requer **2 apertos** do botão
  - Instrução: `pp`

- **Azul** → Digite `ppp` e pressione ENTER
  - Requer **3 apertos** do botão
  - Instrução: `ppp`

---

### Módulo de Senha

O módulo de senha exibe um hash (sequência de letras de A a I) e você deve descobrir a senha numérica correta usando as tabelas de mapeamento abaixo. O tamanho do hash varia conforme a dificuldade:
- **Fácil**: 3 ou 4 caracteres
- **Médio**: 4 ou 5 caracteres
- **Difícil**: 5 ou 6 caracteres

**Como resolver:**
1. Observe o hash exibido (ex: "ABCE")
2. Use a tabela de mapeamento correspondente à dificuldade do jogo
3. Para cada letra do hash, encontre o número correspondente na tabela
4. Digite a sequência numérica completa como instrução

**Tabela de Mapeamento:**

```
Letras:          Números:
A | B | C       9 | 5 | 2
D | E | F       6 | 8 | 1
G | H | I       7 | 3 | 4
```

**Exemplo:**
- Hash: `ABCE`
- Conversão usando a tabela: A→9, B→5, C→2, E→8
- Senha: `9528`

**Importante:**
- O hash sempre usa apenas letras de A a I (9 caracteres distintos)
- O mapeamento é sempre o mesmo, independente da dificuldade
- A senha deve ser digitada como uma sequência numérica contínua (ex: `9528`)

---

### Módulo de Fios Coloridos

O módulo de fios exibe uma sequência de fios coloridos no formato `/R/G/B/Y/W/` onde cada letra representa uma cor:
- **R** = Vermelho (Red)
- **G** = Verde (Green)
- **B** = Azul (Blue)
- **Y** = Amarelo (Yellow)
- **W** = Branco (White)
- **K** = Preto (Black)

**Como resolver:**
O módulo usa um dos 5 padrões de regras abaixo. Você deve identificar qual padrão está ativo e cortar o fio na posição correta.

**Padrão 0 - Primeiro Fio Primário:**
- Corte o primeiro fio que for de cor primária (R, G ou B)
- Se não houver fio primário, corte o primeiro fio

**Padrão 1 - Último Fio com Amarelo:**
- Se houver um fio amarelo (Y) na sequência, corte o último fio
- Caso contrário, corte o primeiro fio

**Padrão 2 - Segundo Fio sem Preto:**
- Se não houver fio preto (K) na sequência, corte o segundo fio
- Caso contrário, corte o primeiro fio

**Padrão 3 - Cor Mais Frequente:**
- Identifique a cor que aparece mais vezes na sequência
- Corte o primeiro fio dessa cor

**Padrão 4 - Posição Central:**
- Corte o fio na posição central (número de fios dividido por 2, arredondado para cima)
- Exemplo: 5 fios → corte o fio na posição 3

**Instrução:**
- Digite o número da posição do fio a ser cortado (1, 2, 3, 4 ou 5)
- Exemplo: Se precisar cortar o terceiro fio, digite `3`

---

# Como Enviar Instruções

O jogo utiliza um sistema de comandos onde você especifica qual tedax, bancada e módulo deseja usar. Caso deseje enviar apenas a instrução, os componentes (tedax, bancada e módulo) serão considerados default. Você também pode expecificar apenas um componente.

Relembrando, o formato do input é: `T<tedax>B<bancada>M<modulo>:<instrucao>`

#### Regras de Default

Se você não especificar algum componente:
- **Tedax não especificado**: Usa o tedax livre com o índice mais baixo
- **Bancada não especificada**: Usa a bancada livre com o índice mais baixo
- **Módulo não especificado**: Usa o primeiro módulo pendente da fila

### Importante

- Cada tedax só pode desarmar um módulo por vez
- Cada tedax precisa de uma bancada livre para trabalhar
- Se o tedax ou bancada especificados estiverem ocupados, o comando será ignorado
- Se a instrução estiver **correta**, o módulo é desarmado
- Se a instrução estiver **incorreta**, o módulo volta para o mural de módulos pendentes e você pode tentar novamente
- Cada módulo tem um tempo limite para ser desarmado
- Você pode especificar parcialmente o comando (apenas tedax, apenas bancada, apenas módulo, ou combinações)
- O sistema valida automaticamente se todos os recursos especificados estão disponíveis
