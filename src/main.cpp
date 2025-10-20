#include "c8vm.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <cstring>

// Imprime ajuda
void mostrarAjuda(const char* programa) {
    std::cout << "=== Chip-8 Emulator ===\n\n";
    std::cout << "Uso: " << programa << " [opções] <arquivo_rom>\n\n";
    std::cout << "Opções:\n";
    std::cout << "  --clock <hz>      Define velocidade da CPU em Hz (padrão: 500)\n";
    std::cout << "                    Exemplo: --clock 700\n\n";
    std::cout << "  --scale <fator>   Define fator de escala da janela (padrão: 10)\n";
    std::cout << "                    Fator 10 = janela 640x320 pixels\n";
    std::cout << "                    Exemplo: --scale 15\n\n";
    std::cout << "  --load-addr <end> Define endereço de carga da ROM (padrão: 0x200)\n";
    std::cout << "                    Deve ser em hexadecimal (ex: 0x200)\n";
    std::cout << "                    Exemplo: --load-addr 0x600\n\n";
    std::cout << "  --help            Mostra esta mensagem de ajuda\n\n";
    std::cout << "Exemplos:\n";
    std::cout << "  " << programa << " pong.ch8\n";
    std::cout << "  " << programa << " --clock 1000 --scale 20 tetris.ch8\n";
    std::cout << "  " << programa << " --load-addr 0x600 programa.ch8\n\n";
    std::cout << "Controles:\n";
    std::cout << "  Chip-8:     Teclado:\n";
    std::cout << "  1 2 3 C     1 2 3 4\n";
    std::cout << "  4 5 6 D     Q W E R\n";
    std::cout << "  7 8 9 E     A S D F\n";
    std::cout << "  A 0 B F     Z X C V\n";
    std::cout << "  ESC = Sair\n";
}

int main(int argc, char *argv[])
{
    // Valores padrão
    int clock = 500;           // 500Hz
    int escala = 10;           // janela 640x320
    uint16_t load_addr = 0x200; // endereço padrão
    std::string arquivo_rom = "";

    // Parsing de argumentos
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            mostrarAjuda(argv[0]);
            return 0;
        }
        else if (arg == "--clock") {
            if (i + 1 < argc) {
                clock = std::stoi(argv[++i]);
                if (clock <= 0) {
                    std::cerr << "Erro: velocidade deve ser maior que 0\n";
                    return 1;
                }
            } else {
                std::cerr << "Erro: --clock requer um valor\n";
                return 1;
            }
        }
        else if (arg == "--scale") {
            if (i + 1 < argc) {
                escala = std::stoi(argv[++i]);
                if (escala <= 0) {
                    std::cerr << "Erro: escala deve ser maior que 0\n";
                    return 1;
                }
            } else {
                std::cerr << "Erro: --scale requer um valor\n";
                return 1;
            }
        }
        else if (arg == "--load-addr") {
            if (i + 1 < argc) {
                // Converte hex string para int
                load_addr = std::stoi(argv[++i], nullptr, 16);
                if (load_addr >= 4096) {
                    std::cerr << "Erro: endereço deve estar entre 0x000 e 0xFFF\n";
                    return 1;
                }
            } else {
                std::cerr << "Erro: --load-addr requer um valor\n";
                return 1;
            }
        }
        else if (arg[0] == '-') {
            std::cerr << "Erro: opção desconhecida '" << arg << "'\n";
            std::cerr << "Use --help para ver opções disponíveis\n";
            return 1;
        }
        else {
            // Assume que é o arquivo ROM
            arquivo_rom = arg;
        }
    }

    // Verifica se ROM foi especificada
    if (arquivo_rom.empty()) {
        std::cerr << "Erro: nenhum arquivo ROM especificado\n";
        std::cerr << "Uso: " << argv[0] << " [opções] <arquivo_rom>\n";
        std::cerr << "Use --help para mais informações\n";
        return 1;
    }

    try {
        // Cria VM com endereço de carga personalizado
        VM vm(load_addr);
        
        // Configura velocidade
        vm.velocidade_cpu = clock;

        // Carrega ROM no endereço especificado
        vm.carregarROM(arquivo_rom, load_addr);

        // Inicializa SDL com escala configurada
        if (!vm.inicializarSDL(escala)) {
            return 1;
        }

        // Inicializa áudio
        if (!vm.inicializarAudio()) {
            std::cerr << "Aviso: falha ao inicializar áudio (continuando sem som)\n";
        }

        // Mostra configuração
        std::cout << "Configuração:\n";
        std::cout << "  CPU: " << clock << " Hz\n";
        std::cout << "  Escala: " << escala << "x (janela " 
                  << 64*escala << "x" << 32*escala << ")\n";
        std::cout << "  Endereço de carga: 0x" << std::hex << load_addr << std::dec << "\n\n";

        // Inicia o loop principal
        vm.loopPrincipal();

    } catch (const std::exception &e) {
        std::cerr << "Erro: " << e.what() << "\n";
        return 1;
    }

    return 0;
}