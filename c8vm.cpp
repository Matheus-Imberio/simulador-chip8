#include "c8vm.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <iomanip>

VM::VM(uint16_t pc_inicial) : PC(pc_inicial), SP(0), I(0)
{
    RAM.fill(0);
    V.fill(0);
    STACK.fill(0);
    DISPLAY.fill(0);
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
        if (inst == 0x00E0)
            DISPLAY.fill(0);
        PC += 2;
        break;

    case 0x6: // 6XNN
        V[X] = NN;
        PC += 2;
        break;

    case 0xA: // ANNN
        I = NNN;
        PC += 2;
        break;
    case 0xD:
    { // DXYN - desenha sprite
        uint8_t x = V[X] % 64;
        uint8_t y = V[Y] % 32;
        uint8_t height = N;
        V[0xF] = 0;

        for (int row = 0; row < height; ++row)
        {
            uint8_t spriteByte = RAM[I + row];
            for (int col = 0; col < 8; ++col)
            {
                if (spriteByte & (0x80 >> col))
                {
                    int px = x + col;
                    int py = y + row;
                    if (px < 64 && py < 32)
                    {
                        int idx = py * 64 + px;
                        if (DISPLAY[idx] == 1)
                            V[0xF] = 1;    // colisão
                        DISPLAY[idx] ^= 1; // inverte pixel
                    }
                }
            }
        }

        PC += 2;
        break;
    }

    default:
        std::cout << "Grupo não implementado! Instrução: 0x"
                  << std::hex << inst << std::dec << "\n";
        throw std::runtime_error("Opcode não implementado");
    }
}

void VM::imprimirRegistradores() const
{
    std::cout << "PC: 0x" << std::hex << PC
              << " I: 0x" << I
              << " SP: 0x" << std::dec << static_cast<int>(SP) << "\n";

    for (int i = 0; i < 16; ++i)
    {
        std::cout << "V[" << std::hex << i << "]: 0x"
                  << std::setw(2) << std::setfill('0')
                  << static_cast<int>(V[i]) << " ";
    }
    std::cout << std::dec << "\n";
}
