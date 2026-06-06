#include "apu.h"

namespace GB{

bool CH3::is_length_enabled(void){
  return static_cast<bool>((memoria[0xFF1E] & 0x40) != 0);
}

void CH3::init_ch3(void){
  this->seta_length();
  this->seta_output();

  if(is_channel3_on(memoria) && (periodo_divider <= periodo_shadow)){
    
    if(last_byte < 4){
      uint8_t byte = memoria[WAVE_RAM_INICIO + last_byte];
      memoria[WAVE_RAM_INICIO] = byte;
    }
    else{
      uint8_t index = last_byte;
      if(index >= 4 && index < 8)
        index = 4;
      else if(index >= 8 && index < 12)
        index = 8;
      else
        index = 12;

      for(size_t i {}; i < 4; ++i){
        memoria[WAVE_RAM_INICIO + i] = memoria[WAVE_RAM_INICIO + index + i];
      }
    }
  }

  periodo_shadow = (((memoria[0xFF1E] & 0x07) << 8) | memoria[0xFF1D]);
  periodo_divider = periodo_shadow;
  trigger_delay = 2;
  wram_index = 0;
}

void CH3::seta_output(void){
  output_level = (memoria[0xFF1C] & 0x60) >> 5;
}

void CH3::seta_length(void){
  if(!length_timer)
    length_timer = 256;
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
      last_sample = memoria[WAVE_RAM_INICIO + (wram_index >> 1)] & 0x0F;
    else
      last_sample = ((memoria[WAVE_RAM_INICIO + (wram_index >> 1)] & 0xF0) >> 4);

    last_byte = (wram_index >> 1);
    wram_index = (wram_index + 1) % 32;
    periodo_shadow = (((memoria[0xFF1E] & 0x07) << 8) | memoria[0xFF1D]);
    periodo_divider = periodo_shadow;
    if(trigger_delay)
      --trigger_delay;
  }
}

uint8_t CH3::get_sample(void){
  if(!is_channel3_on(memoria) || !dac) return 67;
  this->seta_output(); 
  if(!output_level) return 0;

  uint8_t bit = output_level - 1;
  return (last_sample >> bit);
}

void CH3::clear(void){
  dac = false;
  last_sample = 0;
  wram_index = 0;
  trigger_delay = 0;
  periodo_divider = 0;
  periodo_shadow = 0;
  output_level = 0;
  memoria[0xFF1A] = 0;
  memoria[0xFF1C] = 0;
  memoria[0xFF1D] = 0;
  memoria[0xFF1E] = 0;
}

}
