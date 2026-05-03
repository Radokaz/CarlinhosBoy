#include "cpu.h"

namespace GB{

void Timer::step(Memorybus& bus){
  for(size_t i {}; i < 4; ++i){
    ++div_count;
    bus.read_byte(0xFF04) = this->get_div();

    uint8_t tac = bus.read_byte(0xFF07);
    if(tac & 0x04){

      uint8_t bit{};
      switch(tac & 0x03){
      case 0x00:
        bit = 9;
        break;
      case 0x01:
        bit = 3;
        break;
      case 0x02:
        bit = 5;
        break;
      case 0x03:
        bit = 7;
        break;
      }

      uint8_t bit_atual = (div_count >> bit) & 0x01;
      if(!bit_atual && prev_bit){
        uint8_t& tima = bus.read_byte(0xFF05);
        if(tima == 0xFF){
          tima = 0;
          timaoverflow = true;
          timaoverflow_count += 4;
        }
        else
          ++tima;
      }

      prev_bit = bit_atual;
    }

    if(timaoverflow_count)
        --timaoverflow_count;

    if(timaoverflow && timaoverflow_count <= 0){
      bus.read_byte(0xFF05) = bus.read_byte(0xFF06);
      bus.read_byte(0xFF0F) |= BIT_TIMER;
      timaoverflow = false;
      timaoverflow_count = 0;
    }

  }
}

}
