#include "apu.h"

namespace GB{

bool CH4::is_length_enabled(void){
  return static_cast<bool>((memoria[0xFF23] & 0x40) != 0);
}

void CH4::init_ch4(void){
  if(!length_timer)
    this->seta_length(true);

  this->seta_envelope();
  this->seta_clock();
}

void CH4::seta_length(bool trigger){
  if(this->is_length_enabled())
    length_timer = (trigger) ? 64 : 64 - (memoria[0xFF20] & 0x3F);
  else
    length_timer = (trigger) ? 64 : 0;
}

void CH4::seta_clock(void){
  clock_shifter = (memoria[0xFF22] & 0xF0) >> 4;
  lfsr_width = (((memoria[0xFF22] & 0x08) >> 3) != 0) ? 7 : 15;
  clock_divider = (memoria[0xFF22] & 0x07);

  uint8_t divider = (!clock_divider) ? 8 : 16*clock_divider;
  ciclos = 0;
  period = divider << clock_shifter;
  lfsr = 0x7FFF;
}

void CH4::seta_envelope(void){
  initial_volume = (memoria[0xFF21] & 0xF0) >> 4;
  envelope = initial_volume;
  direcao_envelope = (memoria[0xFF21] & 0x08) >> 3;
  envelope_pace = memoria[0xFF21] & 0x07;
  envelope_count = envelope_pace;
}

void CH4::sweep_length(void){
  if(!is_length_enabled()) return;
  if(length_timer){
    --length_timer;
    if(!length_timer){
      memoria[0xFF26] &= ~APU_CH4_ON;
    }
  }
}

void CH4::incrementa_clock(void){
  if(!is_channel4_on(memoria) || !dac) return;
      uint8_t bit = ((lfsr & 0x01) ^ ((lfsr >> 1) & 0x01));
      lfsr = lfsr >> 1;
      lfsr |= (bit << 14);

      if(lfsr_width == 7){
        lfsr = (lfsr & ~(1 << 6)) | (bit << 6);
      }
}

void CH4::sweep_envelope(void){
  if(!is_channel4_on(memoria) || !dac) return;
  if(envelope_count){
    --envelope_count;
    if(!envelope_count){
      envelope = (!direcao_envelope) ? ((envelope > 0) ? envelope - 1 : 0) : envelope + 1;
      if(envelope > 15)
        envelope = 15;

      envelope_count = envelope_pace;
    }
  }
}

uint8_t CH4::get_sample(void){
  if(!is_channel4_on(memoria) || !dac) return 0;
  return (lfsr & 0x01) ? 0 : envelope;
}

void CH4::clear(void){
  dac = false;

  period = 8;
  ciclos = 0;

  lfsr = 0;
  clock_shifter = 0;
  lfsr_width = 0;
  clock_divider = 0;

  envelope = 0;
  initial_volume = 0;
  envelope_pace = 0;
  direcao_envelope = 0;
  envelope_count = 0;

  memoria[0xFF21] = 0;
  memoria[0xFF22] = 0;
  memoria[0xFF23] = 0;
}

  
}
