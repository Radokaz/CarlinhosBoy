#include "apu.h"

namespace GB{

bool CH1::is_length_enabled(){
    return static_cast<bool>((memoria[0xFF14] & 0x40) != 0);
}

void CH1::init_ch1(void){
    periodo_shadow = (((memoria[0xFF14] & 0x07) << 8) | (memoria[0xFF13]));
    periodo_divider = periodo_shadow;

    periodo_pace = ((memoria[0xFF10] & 0x70) >> 4);
    periodo_count = (!periodo_pace) ? 8 : periodo_pace;
    ind_step = memoria[0xFF10] & 0x07;
    direcao_periodo = ((memoria[0xFF10] & 0x08) >> 3);
    sweep_enabled = (ind_step || periodo_pace);
    negate_mode = false;

    if(ind_step){
      if(!direcao_periodo && ((periodo_shadow + (periodo_shadow >> ind_step)) > 0x7FF)){
        memoria[0xFF26] &= ~APU_CH1_ON;
        return;
      }
      if(direcao_periodo){
        negate_mode = true;
      }
    }

    this->seta_length();
    this->seta_envelope();
}

void CH1::seta_length(void){
    if(!length_timer)
      length_timer = 64;
}

void CH1::seta_envelope(void){
    initial_volume = ((memoria[0xFF12] & 0xF0) >> 4);
    direcao_envelope = ((memoria[0xFF12] & 0x08) >> 3);
    envelope_pace = (memoria[0xFF12] & 0x07);
    envelope_count = envelope_pace;
    envelope = initial_volume;
}

void CH1::sweep_periodo(void){
    if(!is_channel1_on(memoria) || !dac || !sweep_enabled) return;
    if(periodo_count){
      --periodo_count;
      if(!periodo_count){

        uint16_t offset = (periodo_shadow >> ind_step);
        uint16_t novo_periodo {};

        ind_step = memoria[0xFF10] & 0x07;
        periodo_pace = ((memoria[0xFF10] & 0x70) >> 4);
        direcao_periodo = ((memoria[0xFF10] & 0x08) >> 3);

        if(!direcao_periodo){
          novo_periodo = periodo_shadow + offset;
          if(novo_periodo > 0x7FF){
            memoria[0xFF26] &= ~APU_CH1_ON;
            return;
          }
        }
        else{
          novo_periodo = periodo_shadow - offset;
          if(!negate_mode && periodo_pace)
            negate_mode = true;
        }

        if(ind_step && periodo_pace){
          periodo_shadow = novo_periodo;
          memoria[0xFF13] = (periodo_shadow & 0xFF);
          memoria[0xFF14] = ((memoria[0xFF14] & 0xF8) | (periodo_shadow >> 8) & 0x07);
          if(!direcao_periodo && ((periodo_shadow + (periodo_shadow >> ind_step)) > 0x7FF)){
            memoria[0xFF26] &= ~APU_CH1_ON;
            return;
          }
        }

        periodo_count = (!periodo_pace) ? 8 : periodo_pace;
      }
    }
}

void CH1::sweep_envelope(void){
    if(!is_channel1_on(memoria) || !dac) return;
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

void CH1::incrementa_divider(void){
    if(!is_channel1_on(memoria) || !dac) return;

    ++periodo_divider;
    if(periodo_divider > 2047){
      periodo_divider = (((memoria[0xFF14] & 0x07) << 8) | (memoria[0xFF13]));
      duty_step = (duty_step + 1) % 8;
      this->amplifier();
    }
}

void CH1::sweep_length(void){
    if(!is_length_enabled()) return;
    if(length_timer){
      --length_timer;
      if(!length_timer){
        memoria[0xFF26] &= ~APU_CH1_ON;
      }
    }
}

uint8_t CH1::get_sample(void){
    uint8_t duty_cycle = ((memoria[0xFF11] & 0xC0) >> 6);
    return (tabela_onda[duty_cycle][duty_step]) ? envelope : 0;
}

void CH1::amplifier(void){
  uint8_t sample = this->get_sample();
  mixer(sample, ch1_prev, is_ch1_left(memoria) && (APU::canais_ativos & APU_CANAL1),
      is_ch1_right(memoria) && (APU::canais_ativos & APU_CANAL1));
}

void CH1::clear(void){
  periodo_divider = 0;
  periodo_shadow = 0;
  dac = false;

  periodo_pace = 0;
  ind_step = 0;
  direcao_periodo = 0;

  duty_step = 0;
  ch1_prev = 0;

  envelope = 0;
  initial_volume = 0;
  envelope_pace = 0;
  direcao_envelope = 0;
    
  envelope_count = 0;
  periodo_count = 0;

  memoria[0xFF10] = 0;
  memoria[0xFF11] &= 0x3F;
  memoria[0xFF12] = 0;
  memoria[0xFF13] = 0;
  memoria[0xFF14] = 0;
}

bool CH2::is_length_enabled(){
    return static_cast<bool>((memoria[0xFF19] & 0x40) != 0);
}

void CH2::init_ch2(void){
    periodo_shadow = (((memoria[0xFF19] & 0x07) << 8) | (memoria[0xFF18]));
    periodo_divider = periodo_shadow;

    this->seta_length();
    this->seta_envelope();
}

void CH2::seta_length(void){
    if(!length_timer)
      length_timer = 64;
}

void CH2::seta_envelope(void){
    initial_volume = ((memoria[0xFF17] & 0xF0) >> 4);
    direcao_envelope = ((memoria[0xFF17] & 0x08) >> 3);
    envelope_pace = (memoria[0xFF17] & 0x07);
    envelope_count = envelope_pace;
    envelope = initial_volume;
}

void CH2::sweep_envelope(void){
    if(!is_channel2_on(memoria) || !dac) return;
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

void CH2::incrementa_divider(void){
    if(!is_channel2_on(memoria) || !dac) return;

    ++periodo_divider;
    if(periodo_divider > 2047){
      periodo_shadow = (((memoria[0xFF19] & 0x07) << 8) | (memoria[0xFF18]));
      periodo_divider = periodo_shadow;
      duty_step = (duty_step + 1) % 8;
      this->amplifier();
    }
}

void CH2::sweep_length(void){
    if(!is_length_enabled()) return;
    if(length_timer){
      --length_timer;
      if(!length_timer){
        memoria[0xFF26] &= ~APU_CH2_ON;
      }
    }
}

uint8_t CH2::get_sample(void){
    uint8_t duty_cycle = ((memoria[0xFF16] & 0xC0) >> 6);
    return (tabela_onda[duty_cycle][duty_step]) ? envelope : 0;
}

void CH2::amplifier(void){
  uint8_t sample = this->get_sample();
  mixer(sample, ch2_prev, is_ch2_left(memoria) && (APU::canais_ativos & APU_CANAL2),
      is_ch2_right(memoria) && (APU::canais_ativos & APU_CANAL2));
}

void CH2::clear(void){
  periodo_divider = 0;
  periodo_shadow = 0;
  dac = false;

  duty_step = 0;
  ch2_prev = 0;

  envelope = 0;
  initial_volume = 0;
  envelope_pace = 0;
  direcao_envelope = 0; //0 - reduz volume, 1 - aumenta volume
    
  envelope_count = 0;
  memoria[0xFF16] &= 0x3F;
  memoria[0xFF17] = 0;
  memoria[0xFF18] = 0;
  memoria[0xFF19] = 0;
}


}

