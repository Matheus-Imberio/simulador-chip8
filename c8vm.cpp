#include "c8vm.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <iomanip>


// Sprites dos dígitos hexadecimais (0-F)
static const uint8_t FONT_SET[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};



VM::VM(uint16_t pc_inicial) : PC(pc_inicial), SP(0), I(0)
{
    RAM.fill(0);
    V.fill(0);
    STACK.fill(0);
    DISPLAY.fill(0);
    KEYS.fill(0);

    // Carregar sprites dos dígitos na memória(0x000 - 0x04F)
    for (int i = 0; i < 80; ++i) {
        RAM[i] = FONT_SET[i];
    }
    
    // Inicializar gerador de números aleatórios
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

}

void VM::carregarROM(const std::string &arquivo, uint16_t pc_inicial)
{
    std::ifstream rom(arquivo, std::ios::binary | std::ios::ate);
    if (!rom.is_open())
    {
        throw std::runtime_error("Erro ao abrir ROM: " + arquivo);
    }

    std::streamsize tam_rom = rom.tellg();
    rom.seekg(0, std::ios::beg);

    std::vector<char> buffer(tam_rom);
    if (!rom.read(buffer.data(), tam_rom))
    {
        throw std::runtime_error("Erro ao ler ROM");
    }

    for (size_t i = 0; i < buffer.size(); ++i)
    {
        if (pc_inicial + i < RAM.size())
        {
            RAM[pc_inicial + i] = static_cast<uint8_t>(buffer[i]);
        }
    }

    std::cout << "ROM carregada (" << tam_rom << " bytes)\n";
}

void VM::executarInstrucao()
{
    uint16_t inst = (RAM[PC] << 8) | RAM[PC + 1];

    std::cout << "Instrução: 0x" << std::hex << std::uppercase
              << std::setw(4) << std::setfill('0') << inst << std::dec << "\n";

    uint8_t grupo = inst >> 12;
    uint8_t X = (inst & 0x0F00) >> 8;
    uint8_t Y = (inst & 0x00F0) >> 4;
    uint8_t N = inst & 0x000F;
    uint8_t NN = inst & 0x00FF;
    uint16_t NNN = inst & 0x0FFF;

    switch (grupo)
    {
    case 0x0:
        if (inst == 0x00E0) {
            // 00E0 - Limpa a tela
            DISPLAY.fill(0);
        } else if (inst == 0x00EE) {
            // 00EE - Retorna de sub-rotina
            if (SP == 0) {
                throw std::runtime_error("Stack underflow!");
            }
            SP--;
            PC = STACK[SP];
        } else {
            // 0NNN - Chama rotina RCA 1802 (ignorado em implementações modernas)
        }
        PC += 2;
        break;

    case 0x1: // 1NNN - Pula para endereço NNN
        PC = NNN;
        break;

    case 0x2: // 2NNN - Chama sub-rotina em NNN
        if (SP >= 16) {
            throw std::runtime_error("Stack overflow!");
        }
        STACK[SP] = PC;
        SP++;
        PC = NNN;
        break;

    case 0x3: // 3XNN - Pula próxima instrução se VX == NN
        if (V[X] == NN) {
            PC += 2;
        }
        PC += 2;
        break;

    case 0x4: // 4XNN - Pula próxima instrução se VX != NN
        if (V[X] != NN) {
            PC += 2;
        }
        PC += 2;
        break;

    case 0x5: // 5XY0 - Pula próxima instrução se VX == VY
        if (V[X] == V[Y]) {
            PC += 2;
        }
        PC += 2;
        break;

    case 0x6: // 6XNN - Define VX = NN
        V[X] = NN;
        PC += 2;
        break;

    case 0x7: // 7XNN - Adiciona NN a VX (sem carry)
        V[X] += NN;
        PC += 2;
        break;

    case 0x8:
        switch (N) {
        case 0x0: // 8XY0 - VX = VY
            V[X] = V[Y];
            break;
        case 0x1: // 8XY1 - VX = VX | VY
            V[X] |= V[Y];
            break;
        case 0x2: // 8XY2 - VX = VX & VY
            V[X] &= V[Y];
            break;
        case 0x3: // 8XY3 - VX = VX ^ VY
            V[X] ^= V[Y];
            break;
        case 0x4: { // 8XY4 - VX = VX + VY, VF = carry
            uint16_t soma = V[X] + V[Y];
            V[0xF] = (soma > 0xFF) ? 1 : 0;
            V[X] = soma & 0xFF;
            break;
        }
        case 0x5: // 8XY5 - VX = VX - VY, VF = NOT borrow
            V[0xF] = (V[X] >= V[Y]) ? 1 : 0;
            V[X] -= V[Y];
            break;
        case 0x6: // 8XY6 - VX = VY >> 1, VF = bit menos significativo
            V[0xF] = V[Y] & 0x1;
            V[X] = V[Y] >> 1;
            break;
        case 0x7: // 8XY7 - VX = VY - VX, VF = NOT borrow
            V[0xF] = (V[Y] >= V[X]) ? 1 : 0;
            V[X] = V[Y] - V[X];
            break;
        case 0xE: // 8XYE - VX = VY << 1, VF = bit mais significativo
            V[0xF] = (V[Y] & 0x80) >> 7;
            V[X] = V[Y] << 1;
            break;
        default:
            throw std::runtime_error("Opcode 8XYN inválido");
        }
        PC += 2;
        break;

    case 0x9: // 9XY0 - Pula próxima instrução se VX != VY
        if (V[X] != V[Y]) {
            PC += 2;
        }
        PC += 2;
        break;

    case 0xA: // ANNN - Define I = NNN
        I = NNN;
        PC += 2;
        break;

    case 0xB: // BNNN - Pula para endereço NNN + V0
        PC = NNN + V[0];
        break;

    case 0xC: // CXNN - VX = random & NN
        V[X] = (std::rand() % 256) & NN;
        PC += 2;
        break;

    case 0xD: { // DXYN - Desenha sprite na posição (VX, VY) com altura N
        uint8_t x = V[X] % 64;
        uint8_t y = V[Y] % 32;
        uint8_t height = N;
        V[0xF] = 0;

        for (int row = 0; row < height; ++row) {
            if (y + row >= 32) break;
            uint8_t spriteByte = RAM[I + row];
            
            for (int col = 0; col < 8; ++col) {
                if (x + col >= 64) break;
                
                if (spriteByte & (0x80 >> col)) {
                    int idx = (y + row) * 64 + (x + col);
                    if (DISPLAY[idx] == 1) {
                        V[0xF] = 1; // Colisão detectada
                    }
                    DISPLAY[idx] ^= 1;
                }
            }
        }
        PC += 2;
        break;
    }

    case 0xE:
        if (NN == 0x9E) { // EX9E - Pula se tecla VX está pressionada
            if (KEYS[V[X] & 0xF] == 1) {
                PC += 2;
            }
        } else if (NN == 0xA1) { // EXA1 - Pula se tecla VX não está pressionada
            if (KEYS[V[X] & 0xF] == 0) {
                PC += 2;
            }
        }
        PC += 2;
        break;

    case 0xF:
        switch (NN) {
        case 0x07: // FX07 - VX = DT
            V[X] = DT;
            break;
        case 0x0A: // FX0A - Aguarda tecla pressionada e armazena em VX
            {
                bool tecla_pressionada = false;
                for (int i = 0; i < 16; ++i) {
                    if (KEYS[i] == 1) {
                        V[X] = i;
                        tecla_pressionada = true;
                        break;
                    }
                }
                if (!tecla_pressionada) {
                    return; // Não avança PC - aguarda tecla
                }
            }
            break;
        case 0x15: // FX15 - DT = VX
            DT = V[X];
            break;
        case 0x18: // FX18 - ST = VX
            ST = V[X];
            break;
        case 0x1E: // FX1E - I = I + VX
            I += V[X];
            break;
        case 0x29: // FX29 - I = endereço do sprite do dígito VX
            I = (V[X] & 0xF) * 5;
            break;
        case 0x33: // FX33 - Armazena BCD de VX em I, I+1, I+2
            RAM[I] = V[X] / 100;
            RAM[I + 1] = (V[X] / 10) % 10;
            RAM[I + 2] = V[X] % 10;
            break;
        case 0x55: // FX55 - Armazena V0 a VX na memória a partir de I
            for (int i = 0; i <= X; ++i) {
                RAM[I + i] = V[i];
            }
            break;
        case 0x65: // FX65 - Carrega V0 a VX da memória a partir de I
            for (int i = 0; i <= X; ++i) {
                V[i] = RAM[I + i];
            }
            break;
        default:
            throw std::runtime_error("Opcode FX inválido");
        }
        PC += 2;
        break;

    default:
        throw std::runtime_error("Opcode não implementado");
    }
}    

void VM::imprimirRegistradores() const
{
    std::cout << "PC: 0x" << std::hex << PC
              << " I: 0x" << I
              << " SP: 0x" << std::dec << static_cast<int>(SP) << "\n"
              << " DT: " << static_cast<int>(DT)
              << " ST: " << static_cast<int>(ST) << "\n";

    for (int i = 0; i < 16; ++i) {
        std::cout << "V" << std::hex << i << ": 0x"
                  << std::setw(2) << std::setfill('0')
                  << static_cast<int>(V[i]) << " ";
        if (i == 7) std::cout << "\n";
    }
    std::cout << std::dec << "\n";
}

void VM::decrementarTimers()
{
    if (DT > 0) {
        DT--;
    }
    if (ST > 0) {
        ST--;
    }
}

// Destrutor - fecha SDL ao destruir a VM
VM::~VM()
{
    fecharSDL();
}

// Inicializa janela, renderizador e textura do SDL
bool VM::inicializarSDL(int escala_janela)
{
    escala = escala_janela;
    rodando = true;

    // Inicia SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "Erro ao inicializar SDL: " << SDL_GetError() << "\n";
        return false;
    }

    // Cria janela (64*escala x 32*escala)
    janela = SDL_CreateWindow(
        "Chip-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        64 * escala,
        32 * escala,
        SDL_WINDOW_SHOWN
    );

    if (!janela) {
        std::cerr << "Erro ao criar janela: " << SDL_GetError() << "\n";
        return false;
    }

    // Cria renderizador
    renderizador = SDL_CreateRenderer(janela, -1, SDL_RENDERER_ACCELERATED);
    if (!renderizador) {
        std::cerr << "Erro ao criar renderizador: " << SDL_GetError() << "\n";
        return false;
    }

    // Cria textura 64x32 (será esticada pela janela)
    textura = SDL_CreateTexture(
        renderizador,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        64, 32
    );

    if (!textura) {
        std::cerr << "Erro ao criar textura: " << SDL_GetError() << "\n";
        return false;
    }

    std::cout << "SDL inicializado! Janela: " << 64 * escala << "x" << 32 * escala << "\n";
    return true;
}

// Fecha tudo do SDL
void VM::fecharSDL()
{
    if (textura) SDL_DestroyTexture(textura);
    if (renderizador) SDL_DestroyRenderer(renderizador);
    if (janela) SDL_DestroyWindow(janela);
    SDL_Quit();
}

// Renderiza o display na tela (60Hz)
void VM::renderizar()
{
    // Buffer de pixels (64x32, cada pixel = 4 bytes RGBA)
    uint32_t pixels[64 * 32];

    // Converte display (0 ou 1) para pixels (preto ou branco)
    for (int i = 0; i < 64 * 32; i++) {
        // Se pixel ligado = branco (0xFFFFFFFF), senão preto (0xFF000000)
        pixels[i] = DISPLAY[i] ? 0xFFFFFFFF : 0xFF000000;
    }

    // Atualiza textura com os pixels
    SDL_UpdateTexture(textura, nullptr, pixels, 64 * sizeof(uint32_t));

    // Limpa tela
    SDL_RenderClear(renderizador);

    // Desenha textura (SDL estica automaticamente pro tamanho da janela)
    SDL_RenderCopy(renderizador, textura, nullptr, nullptr);

    // Mostra na tela
    SDL_RenderPresent(renderizador);
}

// Processa eventos de teclado e janela
void VM::processarInput()
{
    SDL_Event evento;

    // Pega todos os eventos da fila
    while (SDL_PollEvent(&evento)) {
        // Fecha janela ou ESC
        if (evento.type == SDL_QUIT) {
            rodando = false;
        }
        
        if (evento.type == SDL_KEYDOWN && evento.key.keysym.sym == SDLK_ESCAPE) {
            rodando = false;
        }

        // Mapeia teclado físico -> Chip-8
        // Chip-8:     Teclado físico:
        // 1 2 3 C     1 2 3 4
        // 4 5 6 D     Q W E R
        // 7 8 9 E     A S D F
        // A 0 B F     Z X C V

        if (evento.type == SDL_KEYDOWN) {
            switch (evento.key.keysym.sym) {
                case SDLK_1: KEYS[0x1] = 1; break;
                case SDLK_2: KEYS[0x2] = 1; break;
                case SDLK_3: KEYS[0x3] = 1; break;
                case SDLK_4: KEYS[0xC] = 1; break;
                case SDLK_q: KEYS[0x4] = 1; break;
                case SDLK_w: KEYS[0x5] = 1; break;
                case SDLK_e: KEYS[0x6] = 1; break;
                case SDLK_r: KEYS[0xD] = 1; break;
                case SDLK_a: KEYS[0x7] = 1; break;
                case SDLK_s: KEYS[0x8] = 1; break;
                case SDLK_d: KEYS[0x9] = 1; break;
                case SDLK_f: KEYS[0xE] = 1; break;
                case SDLK_z: KEYS[0xA] = 1; break;
                case SDLK_x: KEYS[0x0] = 1; break;
                case SDLK_c: KEYS[0xB] = 1; break;
                case SDLK_v: KEYS[0xF] = 1; break;
            }
        }

        // Solta tecla
        if (evento.type == SDL_KEYUP) {
            switch (evento.key.keysym.sym) {
                case SDLK_1: KEYS[0x1] = 0; break;
                case SDLK_2: KEYS[0x2] = 0; break;
                case SDLK_3: KEYS[0x3] = 0; break;
                case SDLK_4: KEYS[0xC] = 0; break;
                case SDLK_q: KEYS[0x4] = 0; break;
                case SDLK_w: KEYS[0x5] = 0; break;
                case SDLK_e: KEYS[0x6] = 0; break;
                case SDLK_r: KEYS[0xD] = 0; break;
                case SDLK_a: KEYS[0x7] = 0; break;
                case SDLK_s: KEYS[0x8] = 0; break;
                case SDLK_d: KEYS[0x9] = 0; break;
                case SDLK_f: KEYS[0xE] = 0; break;
                case SDLK_z: KEYS[0xA] = 0; break;
                case SDLK_x: KEYS[0x0] = 0; break;
                case SDLK_c: KEYS[0xB] = 0; break;
                case SDLK_v: KEYS[0xF] = 0; break;
            }
        }
    }
}

// Loop principal da VM
void VM::loopPrincipal()
{
    // Timing
    auto ultimo_ciclo = std::chrono::high_resolution_clock::now();
    auto ultimo_timer = std::chrono::high_resolution_clock::now();
    auto ultimo_frame = std::chrono::high_resolution_clock::now();

    // Intervalos (em microssegundos)
    const int intervalo_cpu = 1000000 / velocidade_cpu;  // ex: 500Hz = 2000us
    const int intervalo_timer = 1000000 / 60;            // 60Hz = ~16666us
    const int intervalo_frame = 1000000 / 60;            // 60Hz

    std::cout << "Loop iniciado! CPU: " << velocidade_cpu << "Hz\n";
    std::cout << "Pressione ESC para sair\n\n";

    while (rodando) {
        auto agora = std::chrono::high_resolution_clock::now();

        // Processa input a cada frame
        processarInput();

        // Executa instrução (na velocidade da CPU configurada)
        auto tempo_desde_ciclo = std::chrono::duration_cast<std::chrono::microseconds>(
            agora - ultimo_ciclo
        ).count();

        if (tempo_desde_ciclo >= intervalo_cpu) {
            executarInstrucao();
            ultimo_ciclo = agora;
        }

        // Decrementa timers a 60Hz
        auto tempo_desde_timer = std::chrono::duration_cast<std::chrono::microseconds>(
            agora - ultimo_timer
        ).count();

        if (tempo_desde_timer >= intervalo_timer) {
            decrementarTimers();
            ultimo_timer = agora;
        }

        // Renderiza a 60Hz
        auto tempo_desde_frame = std::chrono::duration_cast<std::chrono::microseconds>(
            agora - ultimo_frame
        ).count();

        if (tempo_desde_frame >= intervalo_frame) {
            renderizar();
            ultimo_frame = agora;
        }

        // Pequeno sleep pra não consumir 100% da CPU
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    std::cout << "Emulador encerrado\n";
}