#ifndef C8VM_HPP
#define C8VM_HPP

#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <SDL2/SDL.h>

class VM
{
public:
    // Estruturas da VM
    std::array<uint8_t, 4096> RAM{};        // 4KB de memória
    std::array<uint8_t, 16> V{};            // 16 registradores V0-VF
    std::array<uint16_t, 16> STACK{};       // Pilha com 16 níveis
    std::array<uint8_t, 64 * 32> DISPLAY{}; // Tela 64x32 pixels
    std::array<uint8_t, 16> KEYS{};         // Estado das 16 teclas

    // Registradores especiais
    uint16_t PC;    // Program Counter
    uint8_t SP;     // Stack Pointer
    uint16_t I;     // Registrador de endereço
    uint8_t DT;     // Delay Timer
    uint8_t ST;     // Sound Timer

    // SDL - componentes gráficos
    SDL_Window* janela;
    SDL_Renderer* renderizador;
    SDL_Texture* textura;
    
    // SDL - áudio
    SDL_AudioDeviceID dispositivo_audio;
    bool audio_inicializado;
    
    // Config
    int escala;           // fator de zoom da janela
    int velocidade_cpu;   // Hz (instruções por segundo)
    bool rodando;         // controla o loop principal

    // Construtor e destrutor
    VM(uint16_t pc_inicial = 0x200);
    ~VM();

    // Métodos principais da VM
    void carregarROM(const std::string &arquivo, uint16_t pc_inicial = 0x200);
    void executarInstrucao();
    void decrementarTimers();
    void imprimirRegistradores() const;

    // Métodos SDL - vídeo
    bool inicializarSDL(int escala_janela = 10);
    void fecharSDL();
    void renderizar();
    void processarInput();
    void loopPrincipal();
    
    // Métodos SDL - áudio
    bool inicializarAudio();
    void fecharAudio();
    void atualizarAudio();
};

#endif