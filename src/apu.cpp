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

void APU::limpa_registradores(void){ //limpa todos menos os de lenght e o NR52
  sample_count = 0;
  memoria[0xFF24] = 0;
  memoria[0xFF25] = 0;
  memoria[0xFF26] &= 0xF0;

  ch1.clear();
  ch2.clear();
  ch3.clear();
  ch4.clear();
}

void APU::atualiza_volume(void){
  uint8_t master_volume = memoria[0xFF24];
  volume_dir = (master_volume & 0x07) + 1;
  volume_esq = ((master_volume & 0x70) >> 4) + 1;
}

void APU::frame_sequencer(void){
  div_apu = (div_apu + 1) % 8;
  if(!is_audio_on(memoria)) return;

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
