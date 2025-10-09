#include "c8vm.hpp"
#include <iostream>
#include <stdexcept>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Uso: " << argv[0] << " <arquivo_rom>\n";
        return 1;
    }

    try
    {
        VM vm;
        vm.carregarROM(argv[1]);

        std::cout << "Executando instruções da ROM...\n";

        // Executar só algumas instruções de exemplo
        for (int i = 0; i < 5; ++i)
        {
            vm.executarInstrucao();
            vm.imprimirRegistradores();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Erro: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
