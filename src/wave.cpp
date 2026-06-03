#include "apu.h"

namespace GB{

bool CH3::is_length_enabled(void){
  return static_cast<bool>((memoria[0xFF1E] & 0x40) != 0);
}

void CH3::init_ch3(void){
  periodo_shadow = (((memoria[0xFF1E] & 0x07) << 8) | memoria[0xFF1D]);
  periodo_divider = periodo_shadow;

  if(!length_timer)
    this->seta_length(true);

  this->seta_output();
  wram_index = 1;
}

void CH3::seta_output(void){
  output_level = (memoria[0xFF1C] & 0x60) >> 5;
}

void CH3::seta_length(bool trigger){
  if(is_length_enabled())
    length_timer = (trigger) ? 256 : 256 - memoria[0xFF1B];
  else
    length_timer = (trigger) ? 256 : 0;
}

void CH3::sweep_length(void){
  if(!is_length_enabled()) return;
  if(length_timer){
    --length_timer;
    if(!length_timer){
      memoria[0xFF26] &= ~APU_CH3_ON;
    }
  }
}

void CH3::incrementa_divider(void){
  if(!is_channel3_on(memoria) || !dac) return;
  ++periodo_divider;
  if(periodo_divider > 2047){

    if(wram_index % 2) //ímpar: lower_nibble
      last_sample = memoria[WAVE_RAM_INICIO + wram_index/2] & 0x0F;
    else
      last_sample = ((memoria[WAVE_RAM_INICIO + wram_index/2] & 0xF0) >> 4);

    wram_index = (wram_index + 1) % 32;
    periodo_shadow = (((memoria[0xFF1E] & 0x07) << 8) | memoria[0xFF1D]);
    periodo_divider = periodo_shadow;
  }
}

uint8_t CH3::get_sample(void){
  if(!is_channel3_on(memoria) || !dac) return 0;
  if(!output_level) return 0;

  uint8_t bit = output_level - 1;
  return (last_sample >> bit);
}

void CH3::clear(void){
  dac = false;
  last_sample = 0;
  wram_index = 0;
  periodo_divider = 0;
  periodo_shadow = 0;
  output_level = 0;
  memoria[0xFF1A] = 0;
  memoria[0xFF1C] = 0;
  memoria[0xFF1D] = 0;
  memoria[0xFF1E] = 0;
}

}
