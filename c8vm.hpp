#ifndef C8VM_HPP
#define C8VM_HPP

#include <cstdint>
#include <vector>
#include <array>
#include <string>

class VM
{
public:
    std::array<uint8_t, 4096> RAM{};
    std::array<uint8_t, 16> V{};
    std::array<uint16_t, 16> STACK{};
    std::array<uint8_t, 64 * 32> DISPLAY{};
    // Estado das 16 teclas
    std::array<uint8_t, 16> KEYS{};            

    uint16_t PC;
    uint8_t SP;
    uint16_t I;
    uint8_t DT;   // Delay Timer
    uint8_t ST;   // Sound Timer

    VM(uint16_t pc_inicial = 0x200);

    void carregarROM(const std::string &arquivo, uint16_t pc_inicial = 0x200);
    void executarInstrucao();
    void imprimirRegistradores() const;
    void decrementarTimers();
};

#endif
