#ifndef INIT_H
#define INIT_H

#include "actions.h"
#include <string>
#include <fstream>

namespace GB{

inline void init_rom(CPU *cpu, std::string_view src){
  std::fstream arquivo(src.data(), arquivo.binary | arquivo.in);
  if(!arquivo){
    std::cerr << "Não foi possível abrir a ROM.\n";
    exit(1);
  }

  arquivo.read(reinterpret_cast<char*>(cpu->bus.memoria.data()), 0x8000);
  cpu->bus.memoria[0xFF04] = 0xAB;
  cpu->bus.memoria[0xFF05] = 0x00;
  cpu->bus.memoria[0xFF06] = 0x00;
  cpu->bus.memoria[0xFF07] = 0xF8;
  cpu->bus.memoria[0xFF0F] = 0xE1;
  cpu->bus.memoria[0xFFFF] = 0x00;
  cpu->bus.memoria[0xFF40] = 0x91; // LCDC
  cpu->bus.memoria[0xFF41] = 0x85; // STAT
  cpu->bus.memoria[0xFF47] = 0xFC; // BGP

}


inline void init_game(CPU *cpu, char **argv){
  Timer timer;
  init_rom(cpu, argv[1]);

  cpu->bus.div_count = &timer.div_count;
  for(size_t i {}; i < 10; ++i){
    roda_cpu(cpu, timer);
    std::cout << "a: " << cpu->registradores.a << "\n";
    std::cout << "b: " << cpu->registradores.b << "\n";
    std::cout << "c: " << cpu->registradores.c << "\n";
    std::cout << "d: " << cpu->registradores.d << "\n";
    std::cout << "e: " << cpu->registradores.e << "\n";
    std::cout << "h: " << cpu->registradores.h << "\n";
    std::cout << "l: " << cpu->registradores.l << "\n";
    std::cout << "f: " << cpu->registradores.f << "\n";
    std::cout << "af: " << cpu->registradores.get_duplo(reg_target::AF) << "\n";
    std::cout << "bc: " << cpu->registradores.get_duplo(reg_target::BC) << "\n";
    std::cout << "de: " << cpu->registradores.get_duplo(reg_target::DE) << "\n";
    std::cout << "hl: " << cpu->registradores.get_duplo(reg_target::HL) << "\n";
    std::cout << "pc: " << cpu->pc << "\n";
    std::cout << "sp: " << cpu->sp << "\n";
  }
}


}

#endif
