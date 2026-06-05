#include "apu.h"
#include <atomic>

namespace GB{

struct RingBuffer {
    std::array<int16_t, 4096> samples{};
    std::atomic<uint32_t> write_pos{0};
    std::atomic<uint32_t> read_pos{0};

    void push(int16_t l, int16_t r){
        uint32_t wp = write_pos.load(std::memory_order_relaxed);
        samples[wp % samples.size()] = l;
        samples[(wp + 1) % samples.size()] = r;
        write_pos.fetch_add(2, std::memory_order_release);
    }

    uint32_t available(void){
        return write_pos.load(std::memory_order_acquire) - read_pos.load(std::memory_order_relaxed);
    }

    int16_t pop(){
        uint32_t rp = read_pos.fetch_add(1, std::memory_order_relaxed);
        return samples[rp % samples.size()];
    }
};

static RingBuffer ring;

void audio_callback(void* buffer, unsigned int frames){
    int16_t* out = reinterpret_cast<int16_t*>(buffer);
    uint32_t samples_needed = frames*2;

    for(uint32_t i = 0; i < samples_needed; ++i){
        out[i] = (ring.available() > 0) ? ring.pop() : 0;
    }
}

uint8_t& APU::read(uint16_t endereco){

    switch(endereco){
        case 0xFF10:
          apu_hack = memoria[0xFF10] | 0x80;
          return apu_hack;
        case 0xFF11:
          apu_hack = memoria[0xFF11] | 0x3F;
          return apu_hack;
        case 0xFF14:
          apu_hack = memoria[0xFF14] | 0xBF;
          return apu_hack;
        case 0xFF16:
          apu_hack = memoria[0xFF16] | 0x3F;
          return apu_hack;
        case 0xFF19:
          apu_hack = memoria[0xFF19] | 0xBF;
          return apu_hack;
        case 0xFF1A:
          apu_hack = memoria[0xFF1A] | 0x7F;
          return apu_hack;
        case 0xFF1C:
          apu_hack = memoria[0xFF1C] | 0x9F;
          return apu_hack;
        case 0xFF1E:
          apu_hack = memoria[0xFF1E] | 0xBF;
          return apu_hack;
        case 0xFF23:
          apu_hack = memoria[0xFF23] | 0xBF;
          return apu_hack;
        case 0xFF26:
          apu_hack = memoria[0xFF26] | 0x70;
          return apu_hack;
        case 0xFF13:
        case 0xFF15:
        case 0xFF18:
        case 0xFF1B:
        case 0xFF1D:
        case 0xFF1F:
        case 0xFF20:
        case 0xFF27:
        case 0xFF28:
        case 0xFF29:
        case 0xFF2A:
        case 0xFF2B:
        case 0xFF2C:
        case 0xFF2D:
        case 0xFF2E:
        case 0xFF2F:
          apu_hack = 0xFF;
          return apu_hack;
    }

    return memoria[endereco];
}

void APU::write(uint16_t endereco, uint8_t valor){
  switch(endereco){
    case 0xFF26:{
        uint8_t bit_prev = (memoria[0xFF26] & 0x80);
        memoria[0xFF26] = ((memoria[0xFF26] & 0x0F) | (valor & 0xF0));
        if(!(valor & 0x80))
          this->limpa_registradores();
        else if(!bit_prev && (valor & 0x80)){
          this->power_on();
        }

        return;
      }
      case 0xFF10:{
        uint8_t direcao_prev = (memoria[0xFF10] & 0x08);
        memoria[0xFF10] = valor;
        if(direcao_prev && this->ch1.negate_mode && !(valor & 0x08)){
          memoria[0xFF26] &= ~APU_CH1_ON;
        }
        return;
      }
      case 0xFF11:{
        memoria[0xFF11] = (is_audio_on(memoria)) ? valor : ((memoria[0xFF11] & 0xC0) | (valor & 0x3F));
        ch1.length_timer = 64 - (memoria[0xFF11] & 0x3F);
        return;
      }
      case 0xFF12:{
        ch1.dac = ((valor & 0xF8) != 0);
        if(!ch1.dac){
          memoria[0xFF26] &= ~APU_CH1_ON;
        }

        memoria[0xFF12] = valor;
        return;
      }
      case 0xFF14:{
        bool length_prev = ((memoria[0xFF14] & 0x40) != 0);
        memoria[0xFF14] = valor;

        //comportamento obscuro do contador de length
        //segunda metade do período: quando div_apu é par
        if(!length_prev && (valor & 0x40) && !(div_apu % 2)){
          ch1.sweep_length();
        }

        if(valor & 0x80){
          if(ch1.dac)
            memoria[0xFF26] |= APU_CH1_ON;

          ch1.init_ch1();

          if(ch1.length_timer == 64 && (valor & 0x40) && !(div_apu % 2)){
            --ch1.length_timer;
          }

        }
        
        return;
      }
      case 0xFF16:{
        memoria[0xFF16] = (is_audio_on(memoria)) ? valor : ((memoria[0xFF16] & 0xC0) | (valor & 0x3F));
        ch2.length_timer = 64 - (memoria[0xFF16] & 0x3F);
        return;
      }
      case 0xFF17:{
        ch2.dac = ((valor & 0xF8) != 0);
        if(!ch2.dac){
          memoria[0xFF26] &= ~APU_CH2_ON;
        }

        memoria[0xFF17] = valor;
        return;
      }
      case 0xFF19:{
        bool length_prev = ((memoria[0xFF19] & 0x40) != 0);
        memoria[0xFF19] = valor;
        if(!length_prev && (valor & 0x40) && !(div_apu % 2)){
          ch2.sweep_length();
        }
        if(valor & 0x80){
          this->ch2.init_ch2();

          if(ch2.length_timer == 64 && (valor & 0x40) && !(div_apu % 2)){
            --ch2.length_timer;
          }

          if(ch2.dac)
            memoria[0xFF26] |= APU_CH2_ON;
        }
        return;
      }
      case 0xFF1A:{
        ch3.dac = ((valor & 0x80) != 0);
        if(!ch3.dac){
          memoria[0xFF26] &= ~APU_CH3_ON;
        }

        memoria[0xFF1A] = valor;
        return;
      }
      case 0xFF1B:{
        memoria[0xFF1B] = valor;
        ch3.length_timer = 256 - memoria[0xFF1B];
        return;
      }
      case 0xFF1E:{
        bool length_prev = ((memoria[0xFF1E] & 0x40) != 0);
        memoria[0xFF1E] = valor;
        if(!length_prev && (valor & 0x40) && !(div_apu % 2)){
          ch3.sweep_length();
        }
        if(valor & 0x80){
          ch3.init_ch3();

          if(ch3.length_timer == 256 && (valor & 0x40) && !(div_apu % 2)){
            --ch3.length_timer;
          }

          if(ch3.dac){
            memoria[0xFF26] |= APU_CH3_ON;
          }
        }
        return;
      }
      case 0xFF20:{
        memoria[0xFF20] = valor;
        ch4.length_timer = 64 - (memoria[0xFF20] & 0x3F);
        return;
      }
      case 0xFF21:{
        ch4.dac = ((valor & 0xF8) != 0);
        if(!ch4.dac)
            memoria[0xFF26] &= ~APU_CH4_ON;

        memoria[0xFF21] = valor;
        return;
      }
      case 0xFF23:{
        bool length_prev = ((memoria[0xFF23] & 0x40) != 0);
        memoria[0xFF23] = valor;
        if(!length_prev && (valor & 0x40) && !(div_apu % 2)){
          ch4.sweep_length();
        }
        if(valor & 0x80){
          ch4.init_ch4();

          if(ch4.length_timer == 64 && (valor & 0x40) && !(div_apu % 2)){
            --ch4.length_timer;
          }

          if(ch4.dac)
            memoria[0xFF26] |= APU_CH4_ON;
        }
        return;
      }
  }

  memoria[endereco] = valor;
}


void APU::limpa_registradores(void){ //limpa todos menos os de lenght e o NR52
  sample_count = 0;
  memoria[0xFF24] = 0;
  memoria[0xFF25] = 0;
  memoria[0xFF26] = 0;

  ch1.clear();
  ch2.clear();
  ch3.clear();
  ch4.clear();
}

void APU::power_on(void){
  div_apu = 7;
  ch1.duty_step = 0;
  ch2.duty_step = 0;
  ch3.last_sample = 0;
}

void APU::atualiza_volume(void){
  uint8_t master_volume = memoria[0xFF24];
  volume_dir = (master_volume & 0x07) + 1;
  volume_esq = ((master_volume & 0x70) >> 4) + 1;
}

void APU::frame_sequencer(void){
  if(!is_audio_on(memoria)) return;
  div_apu = (div_apu + 1) % 8;

  if(div_apu % 2 == 0){
    ch1.sweep_length();
    ch2.sweep_length();
    ch3.sweep_length();
    ch4.sweep_length();
  }
  if(div_apu == 2 || div_apu == 6){
    ch1.sweep_periodo();
  }
  else if(div_apu == 7){
    ch1.sweep_envelope();
    ch2.sweep_envelope();
    ch4.sweep_envelope();
  }
  
}

void APU::mixer(void){
  uint16_t sample {};
  sample_esq = 0;
  sample_dir = 0;

  sample = ch1.get_sample();
  if(is_ch1_left(memoria))
    sample_esq+=sample;
  if(is_ch1_right(memoria))
    sample_dir+=sample;

  sample = ch2.get_sample();
  if(is_ch2_left(memoria))
    sample_esq+=sample;
  if(is_ch2_right(memoria))
    sample_dir+=sample;

  sample = ch3.get_sample();
  if(is_ch3_left(memoria))
    sample_esq+=sample;
  if(is_ch3_right(memoria))
    sample_dir+=sample;

  sample = ch4.get_sample();
  if(is_ch4_left(memoria))
    sample_esq+=sample;
  if(is_ch4_right(memoria))
    sample_dir+=sample;


}

void APU::amplifier(void){
  this->mixer();
  this->atualiza_volume();

  sample_esq*=volume_esq;
  sample_dir*=volume_dir;

  sample_esq = (sample_esq*0xFFFF)/480 - 0x8000;
  sample_dir = (sample_dir*0xFFFF)/480 - 0x8000;
}

void APU::step(void){
  if(!is_audio_on(memoria)) return;
  
  for(size_t i {}; i < 4; ++i){
    ++sample_count;
    ++ch4.ciclos;
    
    if(sample_count % 2 == 0){
      ch3.incrementa_divider();
    }
    if(ch4.ciclos % ch4.period == 0){
      ch4.ciclos = 0;
      ch4.incrementa_clock();
    }

    if(sample_count % 4 == 0){
      ch1.incrementa_divider();
      ch2.incrementa_divider();
    }

    if(sample_count >= CICLOS_POR_SAMPLE){
      sample_count = 0;
      this->amplifier();
      ring.push(sample_esq, sample_dir);
    }
  }
}

}
