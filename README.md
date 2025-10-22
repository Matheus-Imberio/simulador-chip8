## 🎮 Emulador Chip-8

Implementação de uma máquina virtual Chip-8 em C++ com SDL2 (vídeo, teclado e áudio), seguindo os requisitos do trabalho do Prof. Dr. Juliano H. Foleis.

**Alunos:**
- Matheus Henrique Imberio
- Pedro Conrado Negreiro  
- João Victor Bonilha

Nota: Esta implementação foca no Chip‑8 clássico (não SCHIP/XO‑CHIP). Opcodes e recursos estendidos dessas variantes não são alvo deste projeto.

### Dependências

- Compilador C++17 (g++)
- pkg-config
- SDL2 (headers e libs de desenvolvimento)

Instalação (Ubuntu/Debian):
```bash
sudo apt update
sudo apt install -y build-essential pkg-config libsdl2-dev
```

Verificar SDL2:
```bash
pkg-config --modversion sdl2
```

---

## Como compilar (Makefile)

Dentro da pasta do projeto:
```bash
make        # gera o executável "chip8"
```

Outros alvos úteis:
```bash
make clean                                   # limpa binários/objetos
make deps                                    # dicas de instalação de deps
```

---

## Como usar

Sintaxe básica:
```bash
make run ROM="caminho/para/rom.ch8"
```

Com atributos:
```bash
make run ROM="caminho/para/rom.ch8" ARGS="--clock 700 --scale 15"
```

Opções disponíveis:
- `--clock <velocidade>`: Define a velocidade da CPU em Hz. Padrão: 500.
- `--scale <fator>`: Define o fator de escala da janela. Padrão: 10 (640x320).
- `--load-addr <hex>`: Endereço de carga em hexadecimal. Padrão: 0x200.

Exemplos:
```bash
make run ROM="roms/1-chip8-logo.ch8"
make run ROM="roms/pong.ch8" ARGS="--clock 1000 --scale 20"
make run ROM="roms/pong.ch8" ARGS="--load-addr 0x200"
```

Observações importantes:
- A tela é atualizada a 60Hz; os timers (DT/ST) decrementam a 60Hz.
- A velocidade da CPU é configurável e corresponde a instruções por segundo.
- A maioria das ROMs espera ser carregada em `0x200`.

---

### Baixar e rodar novas ROMs (.ch8)

1. Baixe a ROM (.ch8) de uma fonte confiável (ex.: repositórios de domínio público de Chip‑8).
2. Salve o arquivo na pasta `roms/` do projeto.
3. Execute:
```bash
make run ROM="roms/sua_rom.ch8"
# com opções:
make run ROM="roms/sua_rom.ch8" ARGS="--clock 700 --scale 15"
```

Dicas:
- A maioria das ROMs usa endereço de carga padrão `0x200`; se a ROM pedir outro, use `--load-addr 0x600` (exemplo).
- Pressione `ESC` para sair.

---

## Mapeamento do teclado

**Layout otimizado para jogos (WASD + Espaço):**

| Chip-8 | Teclado físico | Função |
|--------|-----------------|---------|
| 8 | W | Cima |
| 4 | A | Esquerda |
| 6 | D | Direita |
| 2 | S | Baixo |
| 5 | Espaço | Ação/Tiro |

**Layout original (hexadecimal):**

| Chip-8 | Teclado físico |
|--------|-----------------|
| 1 2 3 C | 1 2 3 4 |
| 4 5 6 D | Q W E R |
| 7 8 9 E | A S D F |
| A 0 B F | Z X C V |

- `ESC`: Sair

---

## Áudio

- O Sound Timer (ST) gera um beep (onda quadrada ~440Hz) enquanto `ST > 0`.
- O áudio é inicializado automaticamente; se falhar, a execução continua sem som.

Teste rápido de som:
```bash
echo -ne '\x60\x15\xF0\x18\x12\x04' > test_sound.ch8
make run ROM="roms/test_sound.ch8"
```

---

## Testes sugeridos

```bash
make run ROM="roms/1-chip8-logo.ch8"                  # splash/logo
make run ROM="roms/pong.ch8"                            # jogo clássico
make run ROM="roms/pong.ch8" ARGS="--clock 700 --scale 15"   # variação de parâmetros
```

---

## Resultados de teste (resumo)

- `1-chip8-logo.ch8`: janela abre e exibe gráfico; encerra normalmente (ESC).
- `pong.ch8`: entrada responsiva com mapeamento documentado; áudio inicializa.
- `test_sound.ch8` (ROM mínima gerada no README): emite beep enquanto `ST > 0`.

Obs.: para testes automatizados, o processo pode ser encerrado via `ESC` ou sinal externo.

---

## Debug (opcional)

Para logar as instruções executadas:
1. Edite `defs.hpp` e defina `#define DEBUG 1`.
2. Recompile com `make`.
3. Rode a ROM.

---

## Notas de implementação e aderência ao trabalho

- Memória: 4KB; fontes hex (0–F) pré-carregadas em `0x000`–`0x04F`.
- Registradores: V0–VF (VF como flag), I, PC, SP, DT, ST.
- Pilha: 16 níveis (endereços de 16 bits).
- Tela: 64x32 pixels; renderização via sprites de 8 pixels de largura; atualização a 60Hz.
- Entrada: teclado mapeado conforme tabela acima.
- Timers: DT e ST decrementam a 60Hz; som ativo enquanto `ST > 0`.
- CLI: clock, escala, endereço de carga e ajuda conforme especificação.
- Portabilidade: usa apenas C++17 e SDL2, compilável com `g++`/`clang++` em Linux.

Este README contém todas as informações necessárias para compilação, uso e entendimento do projeto.

---
