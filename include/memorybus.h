#ifndef MEMORYBUS_H
#define MEMORYBUS_H

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <memory>
#include <ranges>
#include <functional>
#include "joypad.h"
#include "ppu.h"
#include "apu.h"
#include "dma.h"
#include "mbc.h"

namespace GB{

struct Memorybus;

struct Timer{
    APU *apu {};
    uint16_t div_count {0xABCC};
    bool prev_bit {};
    uint8_t timaoverflow_count {};

    void step(Memorybus& bus);
    uint8_t get_div(void) { return static_cast<uint8_t>((div_count >> 8) & 0xFF); }
};

//todos os que tem escrita bloqueada quando o APU está desligado
static constexpr std::array<uint16_t, 20> audio_registers{
  0xFF25, 0xFF24, 0xFF10, 0xFF11, 0xFF12, 0xFF13, 0xFF14, 0xFF16, 0xFF17, 0xFF18, 0xFF19, 0xFF1A, 0xFF1B, 0xFF1C, 0xFF1D,
  0xFF1E, 0xFF20, 0xFF21, 0xFF22, 0xFF23};

struct Memorybus{
  std::array<uint8_t, 0xFFFF + 1> memoria{};
  DMA dma;
  Timer *timer {};
  Joypad *pad {};
  PPU *ppu {};
  std::unique_ptr<MBC> mbc {};
  std::function<void()> restaura_rom;
  uint8_t dma_hack {0xFF};
  uint8_t serial_count {};
  bool tem_save {false};

  Memorybus(Timer *tm, Joypad *p, PPU *pp): timer{tm}, pad{p}, ppu{pp} {
    mbc = nullptr;
  }

  uint8_t& read_byte(uint16_t endereco){
    if((endereco < 0x8000 || (endereco >= 0xA000 && endereco < 0xC000)) && mbc){
      return mbc->read(endereco);
    }
    switch(endereco){
        case 0xFF07: //tac
          memoria[endereco] |= 0b11111000; 
          return memoria[endereco];
        case 0xFF0F: //if
          memoria[endereco] |= 0b11100000; 
          return memoria[endereco];
        case 0xFF00: //joypad
          return pad->get_output(); 
        case 0xFF41: //stat
          memoria[endereco] |= 0b10000000; 
          return memoria[endereco];

        //registradores de som
        case 0xFF10:
          dma_hack = memoria[0xFF10] | 0x80;
          return dma_hack;
        case 0xFF11:
          dma_hack = memoria[0xFF11] | 0x3F;
          return dma_hack;
        case 0xFF14:
          dma_hack = memoria[0xFF14] | 0xBF;
          return dma_hack;
        case 0xFF16:
          dma_hack = memoria[0xFF16] | 0x3F;
          return dma_hack;
        case 0xFF19:
          dma_hack = memoria[0xFF19] | 0xBF;
          return dma_hack;
        case 0xFF1A:
          dma_hack = memoria[0xFF1A] | 0x7F;
          return dma_hack;
        case 0xFF1C:
          dma_hack = memoria[0xFF1C] | 0x9F;
          return dma_hack;
        case 0xFF1E:
          dma_hack = memoria[0xFF1E] | 0xBF;
          return dma_hack;
        case 0xFF23:
          dma_hack = memoria[0xFF23] | 0xBF;
          return dma_hack;
        case 0xFF26:
          dma_hack = memoria[0xFF26] | 0x70;
          return dma_hack;
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
          dma_hack = 0xFF;
          return dma_hack;

        default: break;
    }
    if(endereco >= OAM_INICIO && endereco < OAM_FIM && (dma.ativo || ppu->modo_atual == screen_mode::DRAWING || ppu->modo_atual == screen_mode::SOAMRAM)){
      dma_hack = 0xFF;
      return dma_hack;
    }
    if(endereco >= VRAM_INICIO && endereco < VRAM_FINAL && (ppu->modo_atual == screen_mode::DRAWING)){
      dma_hack = 0xFF;
      return dma_hack;
    }
    if(is_channel3_on(memoria.data()) && endereco >= WAVE_RAM_INICIO && endereco < WAVE_RAM_FIM){
      dma_hack = memoria[WAVE_RAM_INICIO + timer->apu->ch3.wram_index/2];
      return dma_hack;
    }

    return memoria[endereco];
  }

  void write_byte(uint16_t endereco, uint8_t valor){
    if(!is_audio_on(memoria.data()) && std::ranges::contains(audio_registers, endereco)){
      return;
    }
    if((endereco < 0x8000 || (endereco >= 0xA000 && endereco < 0xC000)) && mbc){
      mbc->write(endereco, valor);
      return;
    }
    if(endereco < 0x8000){
      return;
    }
    if(endereco >= VRAM_INICIO && endereco < VRAM_FINAL){
      ppu->write_vram(endereco, valor);
      return;
    }
    if(endereco >= OAM_INICIO && endereco < OAM_FIM && (dma.ativo || ppu->modo_atual == screen_mode::DRAWING || ppu->modo_atual == screen_mode::SOAMRAM)){
      return;
    }
    if(is_channel3_on(memoria.data()) && endereco >= WAVE_RAM_INICIO && endereco < WAVE_RAM_FIM){
      return;
    }

    switch(endereco){
      case 0xFF04:{ //div
        memoria[endereco] = 0;
        timer->div_count = 0;
        return;
      }
      case 0xFF00:{ //joypad
        memoria[0xFF00] = (memoria[0xFF00] & 0x0F) | (valor & 0x30);
        return;
      }
      case 0xFF02:{
        if((valor & 0x81) == 0x81){ //serial
          memoria[0xFF02] = valor;
          serial_count = 128;
        }
        return;
      }
      case 0xFF46:{ //dma
        dma.start(valor);
        return;
      }
      case 0xFF50:{
        if(valor)
          restaura_rom();
        return;
      }
      case 0xFF26:{
        memoria[0xFF26] = ((memoria[0xFF26] & 0x0F) | (valor & 0xF0));
        if(!(valor & 0x80))
          this->timer->apu->limpa_registradores();
        return;
      }
      case 0xFF11:{
        memoria[0xFF11] = valor;
        this->timer->apu->ch1.seta_length();
        return;
      }
      case 0xFF12:{
        timer->apu->ch1.dac = ((valor & 0xF8) != 0);
        if(!timer->apu->ch1.dac){
          memoria[0xFF26] &= ~APU_CH1_ON;
        }

        memoria[0xFF12] = valor;
        return;
      }
      case 0xFF14:{
        memoria[0xFF14] = valor;
        if(valor & 0x80){
          this->timer->apu->ch1.init_ch1();

          if(timer->apu->ch1.dac)
            memoria[0xFF26] |= APU_CH1_ON;

        }
        return;
      }
      case 0xFF16:{
        memoria[0xFF16] = valor;
        this->timer->apu->ch2.seta_length();
        return;
      }
      case 0xFF17:{
        timer->apu->ch2.dac = ((valor & 0xF8) != 0);
        if(!timer->apu->ch2.dac){
          memoria[0xFF26] &= ~APU_CH2_ON;
        }

        memoria[0xFF17] = valor;
        return;
      }
      case 0xFF19:{
        memoria[0xFF19] = valor;
        if(valor & 0x80){
          this->timer->apu->ch2.init_ch2();

          if(timer->apu->ch2.dac)
            memoria[0xFF26] |= APU_CH2_ON;
        }
        return;
      }
      case 0xFF1A:{
        timer->apu->ch3.dac = ((valor & 0x80) != 0);
        if(!timer->apu->ch3.dac){
          memoria[0xFF26] &= ~APU_CH3_ON;
        }

        memoria[0xFF1A] = valor;
        return;
      }
      case 0xFF1B:{
        memoria[0xFF1B] = valor;
        this->timer->apu->ch3.seta_length();
        return;
      }
      case 0xFF1E:{
        memoria[0xFF1E] = valor;
        if(valor & 0x80){
          this->timer->apu->ch3.init_ch3();

          if(timer->apu->ch3.dac)
            memoria[0xFF26] |= APU_CH3_ON;
        }
        return;
      }
      case 0xFF20:{
        memoria[0xFF20] = valor;
        this->timer->apu->ch4.seta_length();
        return;
      }
      case 0xFF21:{
        timer->apu->ch4.dac = ((valor & 0xF8) != 0);
        if(!timer->apu->ch4.dac)
            memoria[0xFF26] &= ~APU_CH4_ON;

        memoria[0xFF21] = valor;
        return;
      }
      case 0xFF23:{
        memoria[0xFF23] = valor;
        if(valor & 0x80){
          this->timer->apu->ch4.init_ch4();

          if(timer->apu->ch4.dac)
            memoria[0xFF26] |= APU_CH4_ON;
        }
        return;
      }
    }

    memoria[endereco] = valor;
  }
};

}

#endif
