# 🎮 Emulador Chip-8

Implementação completa de uma máquina virtual Chip-8 com suporte a gráficos, áudio e entrada de teclado usando SDL2.

---


### Compilador C++

> [!IMPORTANT]
> Se ainda não tiver o compilador C++ instalado no seu sistema, execute:
```bash
sudo apt install build-essential g++
```

Verificando a instalação:
```bash
g++ --version
```

### Biblioteca SDL2

> [!IMPORTANT]
> A SDL2 é necessária para gráficos e áudio. Para instalar:

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install libsdl2-dev
```


> [!NOTE]
> Verifique se a SDL2 foi instalada corretamente:
```bash
pkg-config --modversion sdl2
```
Deve mostrar a versão (ex: `2.0.20` ou superior)

---

## Como compilar

### Método 1: Compilação Direta 

```bash
# Clone ou entre na pasta do projeto
cd simulador-chip8

# Compile o projeto
g++ -o chip8 main.cpp c8vm.cpp -std=c++17 -lSDL2

# Verifique se o executável foi criado
ls -lh chip8
```


## Depois é só

```bash
./chip8 <arquivo_rom>
```

**Exemplo:**
```bash
./chip8 1-chip8-logo.ch8
```

### Ver Todas as Opções

```bash
./chip8 --help
```

---

## Opções de Linha de Comando

### `--clock <hz>`
Define a velocidade da CPU em Hz (instruções por segundo).

**Padrão:** 500 Hz  
**Exemplo:**
```bash
# CPU mais lenta (jogos mais fáceis)
./chip8 --clock 300 pong.ch8

# CPU mais rápida (jogos mais difíceis)
./chip8 --clock 1000 tetris.ch8
```

---

### `--scale <fator>`
Define o fator de escala da janela.

**Padrão:** 10 (janela 640x320 pixels)  
**Exemplo:**
```bash
# Janela pequena (fator 5 = 320x160)
./chip8 --scale 5 jogo.ch8

# Janela grande (fator 20 = 1280x640)
./chip8 --scale 20 jogo.ch8
```

---

### `--load-addr <endereço>`
Define o endereço de memória onde a ROM será carregada (em hexadecimal).

**Padrão:** 0x200  
**Exemplo:**
```bash
# Carregar em endereço customizado
./chip8 --load-addr 0x600 programa.ch8
```

> [!WARNING]
> Use esta opção apenas se souber o que está fazendo. A maioria das ROMs espera ser carregada em 0x200.

---

### Combinando Opções

```bash
# CPU rápida + janela grande
./chip8 --clock 800 --scale 15 tetris.ch8

# Todas as opções juntas
./chip8 --clock 1000 --scale 20 --load-addr 0x200 jogo.ch8
```

---

##  Controles

O Chip-8 possui um teclado hexadecimal de 16 teclas (0-F). Estas são mapeadas para o teclado físico:

| Chip-8 | Teclado Físico |
|--------|----------------|
| `1 2 3 C` | `1 2 3 4` |
| `4 5 6 D` | `Q W E R` |
| `7 8 9 E` | `A S D F` |
| `A 0 B F` | `Z X C V` |

**Teclas Especiais:**
- `ESC` - Sair do emulador
- `X` (fechar janela) - Sair do emulador

---

## Áudio

O emulador suporta áudio via Sound Timer (ST).

- Quando uma ROM ativa o Sound Timer, você ouvirá um **beep contínuo** (440Hz - nota Lá)
- O som para automaticamente quando o timer chega a zero
- Nem todas as ROMs usam som

> [!NOTE]
> Se não ouvir som, verifique:
> - Volume do sistema operacional
> - Se a ROM realmente usa áudio (ex: Pong)
> - Se apareceu "Áudio inicializado: 44100Hz" no console

---

## Testando o Emulador

### Teste 1: Verificar Gráficos

```bash
./chip8 1-chip8-logo.ch8
```

Deve abrir uma janela mostrando um logo.

### Teste 2: Verificar Áudio

```bash
# Criar ROM de teste que toca beep
echo -ne '\x60\x15\xF0\x18\x12\x04' > test_sound.ch8

# Executar
./chip8 test_sound.ch8
```

Deve ouvir um beep contínuo. Pressione ESC para sair.

### Teste 3: Verificar Argumentos

```bash
# Mudar velocidade e tamanho da janela
./chip8 --clock 700 --scale 15 1-chip8-logo.ch8
```

A janela deve ficar maior (960x480 pixels).


##  ROMs de Exemplo

### Baixar ROMs Públicas

```bash
# Criar pasta para ROMs
mkdir -p roms
cd roms

# Pong (clássico com som)
wget https://github.com/kripod/chip8-roms/raw/master/games/Pong%20%5BPaul%20Vervalin%2C%201990%5D.ch8 -O pong.ch8

cd ..

# Jogar
./chip8 --clock 700 roms/pong.ch8
```

---

---

## Modo Debug

Para ver cada instrução sendo executada:

1. Edite `defs.hpp`:
```cpp
#define DEBUG 1  // Ativa debug
```

2. Recompile:
```bash
g++ -o chip8 main.cpp c8vm.cpp -std=c++17 -lSDL2
```

3. Execute:
```bash
./chip8 jogo.ch8
```

Você verá no console:
```
PC: 0x200 | Instrução: 0x00E0
PC: 0x202 | Instrução: 0x6101
...
```

> [!TIP]
> Desative o debug antes de jogar para melhor performance!

---
