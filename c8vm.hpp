#ifndef C8VM_HPP
#define C8VM_HPP

#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <SDL2/SDL.h>
#include <chrono>
#include <thread>

class VM
{
public:
    // estruturas da VM
    std::array<uint8_t, 4096> RAM{};        
    std::array<uint8_t, 16> V{};            
    std::array<uint16_t, 16> STACK{};       
    std::array<uint8_t, 64 * 32> DISPLAY{}; // Tela 64x32 pixels
    std::array<uint8_t, 16> KEYS{};         // Estado das 16 teclas

    // Registradores especiais
    uint16_t PC;    // program counter
    uint8_t SP;     // stack pointer
    uint16_t I;     // registrador de endereço
    uint8_t DT;     // delay timer
    uint8_t ST;     // sound timer

    // SDL->componentes gráficos
    SDL_Window* janela;
    SDL_Renderer* renderizador;
    SDL_Texture* textura;
    
    // Config
    int escala;           // fator de zoom da janela
    int velocidade_cpu;   // Hz (instrucao por segundo)
    bool rodando;         // controla o loop principal

    // Construtor e destrutor
    VM(uint16_t pc_inicial = 0x200);
    ~VM();

    // metodos principais da VM
    void carregarROM(const std::string &arquivo, uint16_t pc_inicial = 0x200);
    void executarInstrucao();
    void decrementarTimers();
    void imprimirRegistradores() const;

    // Métodos SDL
    bool inicializarSDL(int escala_janela = 10);
    void fecharSDL();
    void renderizar();
    void processarInput();
    void loopPrincipal();
};

#endif