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
