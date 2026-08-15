#include "cpu.h"

namespace GB{

void Timer::step(Memorybus& bus){
  bool double_speed = (bus.ppu->modo_cpu > 0 && (bus.memoria[0xFF4D] & 0x80) != 0);
  uint8_t tac = bus.memoria[0xFF07];
  uint8_t& tima = bus.memoria[0xFF05];
  constexpr uint8_t tac_table[4] = {9, 3, 5, 7};

  for(size_t i {}; i < 4; ++i){
    ++div_count;
    bus.memoria[0xFF04] = this->get_div();
    
    uint8_t apu_temp = (bus.memoria[0xFF04] >> ((double_speed) ? 5 : 4)) & 0x01;
    if(!apu_temp && apu->div_prev){
      apu->frame_sequencer();
    }
    apu->div_prev = apu_temp;

    if(tac & 0x04){
      uint8_t bit = tac_table[tac & 0x03];
      uint8_t bit_atual = (div_count >> bit) & 0x01;
      if(!bit_atual && prev_bit){ //checa o falling edge
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
        tima = bus.memoria[0xFF06]; //atribui TMA em TIMA
        bus.memoria[0xFF0F] |= BIT_TIMER; //ativa a flag em IF
      }
    }
  }
}

}
