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
static constexpr std::array<uint16_t, 16> audio_registers{
  0xFF10, 0xFF12, 0xFF13, 0xFF14, 0xFF17, 0xFF18, 0xFF19, 0xFF1A, 0xFF1C, 0xFF1D,
  0xFF1E, 0xFF21, 0xFF22, 0xFF23, 0xFF24, 0xFF25};

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
      dma_hack = (timer->apu->ch3.periodo_divider > 2046 && !timer->apu->ch3.trigger_delay) ? memoria[WAVE_RAM_INICIO + timer->apu->ch3.last_byte] : 0xFF;
      return dma_hack;
    }
    if(endereco >= AUDIO_INICIO && endereco < AUDIO_FIM){
      return timer->apu->read(endereco);
    }
    
    return memoria[endereco];
  }

  void write_byte(uint16_t endereco, uint8_t valor){
    if(!is_audio_on(memoria.data()) && std::ranges::contains(audio_registers, endereco)){
      return;
    }
    if(endereco >= AUDIO_INICIO && endereco < AUDIO_FIM){
      timer->apu->write(endereco, valor);
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
      if(timer->apu->ch3.periodo_divider > 2046 && !timer->apu->ch3.trigger_delay){
        memoria[WAVE_RAM_INICIO + timer->apu->ch3.last_byte] = valor;
      }
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
    }

    memoria[endereco] = valor;
  }
};

}

#endif
