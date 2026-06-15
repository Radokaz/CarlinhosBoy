#include "apu.h"

namespace GB{

bool CH4::is_length_enabled(void){
  return static_cast<bool>((memoria[0xFF23] & 0x40) != 0);
}

void CH4::init_ch4(void){
  this->seta_length();

  this->seta_envelope();
  this->seta_clock();
  lfsr = 0x7FFF;
  ultimo_bit = 1;
}

void CH4::seta_length(void){
  if(!length_timer)
    length_timer = 64;
}

void CH4::seta_clock(void){
  clock_shifter = (memoria[0xFF22] & 0xF0) >> 4;
  lfsr_width = (((memoria[0xFF22] & 0x08) >> 3) != 0) ? 7 : 15;
  clock_divider = (memoria[0xFF22] & 0x07);

  uint8_t divider = (!clock_divider) ? 8 : 16*clock_divider;
  period = divider << clock_shifter;
  clock_timer = period;
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

void CH4::sweep_clock(void){
  if(!is_channel4_on(memoria) || !dac) return;
  if(clock_timer){
    --clock_timer;
    if(!clock_timer){
      this->seta_clock();
      this->incrementa_clock();
    }
  }
}

void CH4::incrementa_clock(void){
  if(clock_shifter >= 14) return;

  uint8_t bit0 = (lfsr & 0x01);
  uint8_t bit1 = ((lfsr >> 1) & 0x01);
  uint8_t bit = bit0 ^ bit1;

  lfsr = (lfsr & 0x7FFF) | (bit << 15);

  if(lfsr_width == 7){
    lfsr = (lfsr & ~(1 << 7)) | (bit << 7);
  }

  ultimo_bit = (lfsr & 0x01);
  lfsr = lfsr >> 1;
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
  if(!is_channel4_on(memoria) || !dac) return 67;
  return (ultimo_bit) ? 0 : envelope;
}

void CH4::clear(void){
  dac = false;

  period = 8;
  clock_timer = 0;

  lfsr = 0;
  clock_shifter = 0;
  lfsr_width = 0;
  clock_divider = 0;
  ultimo_bit = 0;

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
