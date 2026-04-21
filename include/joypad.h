#ifndef JOYPAD_H
#define JOYPAD_H

#include "cpu.h"

namespace GB{

  struct Joypad{
    char botoes[2][4] {
      {'T', 'S', 'B', 'A'},
      {'D', 'U', 'L', 'R'}
    };

    void input(uint8_t in, Memorybus& bus){
      bus.read_byte(0xFF00) = 
    }
  };

}


#endif
