# Slay the Spire - Trabalho Prático PDS1, UFMG

## Descrição
Jogo de cartas roguelike desenvolvido em C usando a biblioteca Allegro 5. O jogador enfrenta ondas de inimigos usando um deck de cartas com diferentes efeitos.

## Pré-requisitos

### Dependências
- GCC (compilador C)
- Allegro 5 (biblioteca gráfica, áudio, primitivas e fontes)

### Instalação das Dependências

**Windows (MSYS2):**
```bash
pacman -S mingw-w64-ucrt-x86_64-allegro
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get install liballegro5-dev liballegro-audio5-dev liballegro-acodec5-dev
```

**Linux (Arch):**
```bash
sudo pacman -S allegro
```

## Compilação
O projeto usa um Makefile multiplataforma e gera os artefatos dentro de `build/`.

### Linux / MSYS2 shell (quando `make` existe no PATH)
```bash
make clean
make
```

### Windows PowerShell / CMD (quando apenas `mingw32-make` existe)
```bash
mingw32-make clean
mingw32-make
```

> Dica: se quiser usar `make` no PowerShell, crie um alias para `mingw32-make`.
> Nota: o build padrao usa `-DNDEBUG` (sem logs de debug). Para ver o log de resolucao de assets na inicializacao, remova `-DNDEBUG` de `CFLAGS` no Makefile.

## Execução
Após compilar, execute:
```bash
./build/game
```

No Windows (PowerShell):
```powershell
.\build\game.exe
```

## Teste automático do loop de gameplay

O executável inclui modo de auto-teste determinístico sem abrir janela gráfica.

```bash
make selftest SELFTEST_ROUNDS=1000
```

Ou no PowerShell/CMD:
```bash
mingw32-make selftest SELFTEST_ROUNDS=1000
```

Você também pode executar diretamente:
```bash
./build/game --self-test 1000
```

No Windows:
```powershell
.\build\game.exe --self-test 1000
```


## Estrutura do Projeto

```text
SlayTS/
│
├── Makefile                    # Build multiplataforma (build, run, selftest, clean)
├── README.md                   # Documentação do projeto
├── .gitignore
│
├── assets/                     # Recursos visuais e de áudio
│   ├── background.jpg          # Fundo principal
│   ├── player.png              # Sprite do jogador
│   ├── inimigo_fraco.png       # Sprite de inimigo fraco
│   ├── inimigo_forte.png       # Sprite de inimigo forte
│   ├── boss.png                # Sprite do boss
│   ├── carta_*.png             # Sprites das cartas
│   ├── deck.png                # Ícone da pilha de compra
│   ├── descarte.png            # Ícone da pilha de descarte
│   ├── icon_attack.png         # Ícone de ataque
│   ├── icon_block.png          # Ícone de escudo
│   └── music_*.wav/.ogg        # Trilhas de combate e game over
│
├── includes/                   # Headers públicos
│   ├── card.h
│   ├── combat.h
│   ├── constants.h             # Constantes do jogo
│   ├── deck.h
│   ├── enemy.h
│   ├── player.h
│   ├── renderer.h
│   └── utils.h
│
├── src/
│   ├── main.c                  # Ponto de entrada, loop principal e modo --self-test
│   └── entities/
│       ├── card.c              # Definição e efeitos das cartas
│       ├── combat.c            # Regras de turno e fluxo de combate
│       ├── deck.c              # Compra, descarte e embaralhamento
│       ├── enemy.c             # IA e comportamento dos inimigos
│       ├── player.c            # Estado e atributos do jogador
│       ├── renderer.c          # Renderização, UI e carregamento de assets
│       └── utils.c             # Funções utilitárias gerais
│
└── build/                      # Artefatos gerados pelo Makefile
```

### Tipos de Cartas
- **Ataque**: Dá dano no alvo selecionado
- **Escudo**: Dá escudo ao usuário
- **Cura**: Cura o usuário
- **Ataque e Cura**: Dá dano no alvo selecionado e cura o usuário
- **Coringa**: Dá dano no alvo selecionado e dá escudo ao usuário
- **Buff Dano**: Aumenta o dano de ataque do usuário permanentemente
- **Debuff Escudo**: Aplica vulnerabilidade ao alvo selecionado, fazendo-o levar 50% a mais de dano por ataque
- **Especial**: Embaralha as pilhas de compra e descarte e compra outra mão de cartas

### Tipos de Inimigos
- **Fracos**: HP baixo, dano moderado
- **Fortes**: HP alto, dano alto
- **Boss**: Aparece no turno 11, HP e dano altos

## Controles

- **Setas direcionais**: Selecionam cartas e alvos
- **Enter**: Jogar carta ou confirmar ação
- **ESC**: Finaliza o turno do jogador
- **Z**: Começa o jogo
- **R**: Reinicia o jogo em caso de vitória ou derrota
- **Espaço**: Mata os inimigos na tela e avança o turno em 1
- **X**: Deixa o player com 1 de vida
- **Q**: Sai do jogo

## Sistema de Combate

1. No início do turno, jogador recebe energia e compra 5 cartas
2. O jogador joga cartas gastando energia (cada carta custa energia igual ao seu custo)
3. Cartas que requerem alvo permitem selecionar inimigos com as setas
4. Após passar o turno (ESC), os inimigos atacam
5. O bloqueio de cada criatura é removido no início de cada turno
6. No turno 11, o boss aparece

## Observações Técnicas

- Resolução: 1920x1080 (fullscreen window)
- Taxa de atualização: 30 FPS (Podendo ser rodado em FPS mais alto)
- Compatível com Windows e Linux

## Créditos

**Desenvolvedor:** Pedro Henrique Costa Lima 
**Disciplina:** PDS1 - Programação e Desenvolvimento de Software I  
**Período:** 2025.2  
**Instituição:** UFMG - Universidade Federal de Minas Gerais

## Licença

Este projeto foi desenvolvido para fins educacionais.
