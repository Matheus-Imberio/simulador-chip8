
#include "c8vm.hpp"
#include <iostream>
#include <chrono>
#include <thread>

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