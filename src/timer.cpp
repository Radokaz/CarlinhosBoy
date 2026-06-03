#include "cpu.h"

namespace GB{

void Timer::step(Memorybus& bus){
  for(size_t i {}; i < 4; ++i){
    ++div_count;
    bus.memoria[0xFF04] = this->get_div();
    
    uint8_t apu_temp = (bus.memoria[0xFF04] >> 4) & 0x01;
    if(!apu_temp && apu->div_prev){
      apu->frame_sequencer();
    }
    apu->div_prev = apu_temp;

    uint8_t tac = bus.memoria[0xFF07];
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
      if(!bit_atual && prev_bit){ //checa o falling edge
        uint8_t& tima = bus.memoria[0xFF05];
        if(tima == 0xFF){
          tima = 0;
          timaoverflow_count = 4;
        }
        else
          ++tima;
      }

      prev_bit = bit_atual;
    }

    if(timaoverflow_count){
        --timaoverflow_count;

      if(!timaoverflow_count){
        bus.memoria[0xFF05] = bus.memoria[0xFF06]; //atribui TMA em TIMA
        bus.memoria[0xFF0F] |= BIT_TIMER; //ativa a flag em IF
      }
    }

  }
}

}
