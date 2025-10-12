#include "c8vm.hpp"
#include <iostream>
#include <stdexcept>

int main(int argc, char *argv[])
{
    // Verifica argumentos
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <arquivo_rom>\n";
        std::cerr << "Exemplo: " << argv[0] << " roms/pong.ch8\n";
        return 1;
    }

    try {
        // Cria VM
        VM vm;
        
        // configura velocidade (pode ser passada por argumento dps)
        vm.velocidade_cpu = 500;  // 500Hz (500 instruções por segundo)

        // Carrega ROM
        vm.carregarROM(argv[1]);

        // inicializa SDL(escala 10 = janela 640x320)
        if (!vm.inicializarSDL(10)) {
            return 1;
        }

        // Inicia o loop principal
        vm.loopPrincipal();

    } catch (const std::exception &e) {
        std::cerr << "Erro: " << e.what() << "\n";
        return 1;
    }

    return 0;
}