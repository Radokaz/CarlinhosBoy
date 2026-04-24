#include "cpu.h"

namespace GB{

void Timer::step(uint8_t ciclos, Memorybus& bus){
      if(timaoverflow){
        bus.read_byte(0xFF05) = bus.read_byte(0xFF06);
        bus.read_byte(0xFF0F) |= BIT_TIMER;
        timaoverflow = false;
      }

      div_count+=ciclos;
      bus.read_byte(0xFF04) = this->get_div();

      uint8_t tac = bus.read_byte(0xFF07);
      if(!(tac & 0x04)) return;

      uint16_t limite{};
      switch(tac & 0x03){
        case 0x00:
          limite = 1024;
          break;
        case 0x01:
          limite = 16;
          break;
        case 0x02:
          limite = 64;
          break;
        case 0x03:
          limite = 256;
          break;
      }

      tima_count+=ciclos;
      if(tima_count >= limite){
        tima_count -= limite;
        uint8_t tima = bus.read_byte(0xFF05);
        if(tima == 0xFF){
          bus.read_byte(0xFF05) = 0;
          timaoverflow = true;
        }
        else
          ++bus.read_byte(0xFF05);
      }
}

}
