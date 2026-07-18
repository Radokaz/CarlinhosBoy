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
#include "mbc.h"

#define ECHO_RAM_INICIO 0xE000
#define ECHO_RAM_FIM 0xFE00

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

struct DMA{
  MBC *mbc {nullptr};
  uint8_t *wram {nullptr};
  bool ativo {false};
  uint8_t byte {};
  uint8_t valor {};
  int8_t atraso {};

  void start(uint8_t valor);
  void step(Memorybus *bus);
};

struct HDMA{
  uint16_t restante {};
  uint16_t destino {};
  uint16_t origem {};
  uint8_t hblank_count {};
  bool modo_hblank {false};
  bool ativo {false};

  void init_transfer(uint8_t vdma);
  void step(Memorybus *bus);
};

//todos os que tem escrita bloqueada quando o APU está desligado
static constexpr std::array<uint16_t, 16> audio_registers{
  0xFF10, 0xFF12, 0xFF13, 0xFF14, 0xFF17, 0xFF18, 0xFF19, 0xFF1A, 0xFF1C, 0xFF1D,
  0xFF1E, 0xFF21, 0xFF22, 0xFF23, 0xFF24, 0xFF25};

struct Memorybus{
  std::array<uint8_t, 0xFFFF + 1> memoria{};
  DMA dma;
  HDMA hdma;
  Timer *timer {};
  Joypad *pad {};
  PPU *ppu {};
  std::unique_ptr<uint8_t[]> cgb_wram {};
  std::unique_ptr<MBC> mbc {};
  std::function<void()> restaura_rom;
  uint8_t dma_hack {0xFF};
  uint8_t serial_count {};
  bool tem_rtc {false};
  bool key0_blocked {false};
  bool opri_blocked {false};
  
  Memorybus(Timer *tm, Joypad *p, PPU *pp): timer{tm}, pad{p}, ppu{pp} {
    mbc = nullptr;
    cgb_wram = nullptr;
    ppu->memoria = memoria.data();
    ppu->hdma_hblank = &hdma.modo_hblank;
    ppu->hdma_ativo = &hdma.ativo;
    pad->p1 = &memoria[0xFF00];
  }

  uint8_t& read_byte(uint16_t endereco){
    if(endereco >= ECHO_RAM_INICIO && endereco < ECHO_RAM_FIM){
      return read_byte(endereco - 0x2000);
    }
    if(mbc && (endereco < 0x8000 || (endereco >= 0xA000 && endereco < 0xC000))){
      return mbc->read(endereco);
    }
    if(cgb_wram && endereco >= 0xC000 && endereco < 0xD000){
      return cgb_wram[endereco - 0xC000];
    }
    if(cgb_wram && endereco >= 0xD000 && endereco < 0xE000){
      uint8_t bank = memoria[0xFF70] & 0x07;
      if(!bank)
        bank = 1;

      return cgb_wram[bank*0x1000 + (endereco - 0xD000)];
    }
    if(endereco >= OAM_INICIO && endereco < OAM_FIM && 
        (dma.ativo || ppu->modo_atual == screen_mode::DRAWING || ppu->modo_atual == screen_mode::SOAMRAM)){
      dma_hack = 0xFF;
      return dma_hack;
    }
    if(endereco >= VRAM_INICIO && endereco < VRAM_FINAL){
      if(ppu->modo_atual == screen_mode::DRAWING){
        dma_hack = 0xFF;
        return dma_hack;
      }

      return ppu->read_vram(endereco);
    }
    if(is_channel3_on(memoria.data()) && endereco >= WAVE_RAM_INICIO && endereco < WAVE_RAM_FIM){
      if(ppu->paleta_cgb || (timer->apu->ch3.periodo_divider > 2046 && !timer->apu->ch3.delay_hack)){
        dma_hack = memoria[WAVE_RAM_INICIO + timer->apu->ch3.last_byte];
        return dma_hack;
      }

      dma_hack = 0xFF;
      return dma_hack;
    }
    if(endereco >= AUDIO_INICIO && endereco < AUDIO_FIM){
      return timer->apu->read(endereco);
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
        case 0xFF4F: //vram_bank
          if(!ppu->modo_cpu){
            dma_hack = (ppu->paleta_cgb) ? 0xFE : 0xFF;
            return dma_hack;
          }

          memoria[endereco] |= 0b11111110;
          return memoria[endereco];
        case 0xFF4C:{ //key0
          if(!ppu->paleta_cgb){
            dma_hack = 0xFF;
            return dma_hack;
          }

          return memoria[endereco];
        }
        case 0xFF4D:{ //key1
          if(!ppu->modo_cpu){
            dma_hack = 0xFF;
            return dma_hack;
          }
          
          dma_hack = (memoria[endereco] & 0x81) | 0x7E;
          return dma_hack;
        }
        case 0xFF51:
        case 0xFF52:
        case 0xFF53:
        case 0xFF54:
          dma_hack = 0xFF;
          return dma_hack;
        case 0xFF55:{ //hdma
          if(!ppu->modo_cpu){
            dma_hack = 0xFF;
            return dma_hack;
          }

          return memoria[endereco];
        }
        case 0xFF69:{ //BGPD
          if(ppu->modo_atual == screen_mode::DRAWING || !ppu->paleta_cgb){
            dma_hack = 0xFF;
            return dma_hack;
          }

          uint8_t indice = memoria[0xFF68] & 0x3F;
          return ppu->bg_palette_ram[indice];
        }
        case 0xFF6B:{ //OBPD
          if(ppu->modo_atual == screen_mode::DRAWING || !ppu->paleta_cgb){
            dma_hack = 0xFF;
            return dma_hack;
          }
          
          uint8_t indice = memoria[0xFF6A] & 0x3F;
          return ppu->obj_palette_ram[indice];
        }
        case 0xFF70:{ //Wram bank
          if(!ppu->modo_cpu){
            dma_hack = 0xFF;
            return dma_hack;
          }

          memoria[endereco] |= 0xF8;
          return memoria[endereco];
        }
        case 0xFF74:{
          if(!ppu->modo_cpu){
            dma_hack = 0xFF;
            return dma_hack;
          }

          return memoria[endereco];
        }
        case 0xFF75:{
          if(!ppu->modo_cpu){
            dma_hack = 0xFF;
            return dma_hack;
          }

          memoria[endereco] &= 0x70;
          return memoria[endereco];
        }
        default: break;
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
    if(mbc && (endereco < 0x8000 || (endereco >= 0xA000 && endereco < 0xC000))){
      mbc->write(endereco, valor);
      return;
    }
    if(endereco < 0x8000){
      return;
    }

    if(endereco >= 0xC000 && endereco < 0xD000){
      if(cgb_wram)
        cgb_wram[endereco - 0xC000] = valor;
      else
        memoria[endereco] = valor;

      return;
    }
    if(endereco >= 0xD000 && endereco < 0xE000){
      if(cgb_wram){
        uint8_t bank = memoria[0xFF70] & 0x07;
        if(!bank)
          bank = 1;

        cgb_wram[bank*0x1000 + (endereco - 0xD000)] = valor;
      }
      else
        memoria[endereco] = valor;

      return;
    }
    if(endereco >= ECHO_RAM_INICIO && endereco < ECHO_RAM_FIM){
      write_byte(endereco - 0x2000, valor);
      return;
    }

    if(endereco >= VRAM_INICIO && endereco < VRAM_FINAL){
      ppu->write_vram(endereco, valor);
      return;
    }
    if(endereco >= OAM_INICIO && endereco < OAM_FIM && 
        (dma.ativo || ppu->modo_atual == screen_mode::DRAWING || ppu->modo_atual == screen_mode::SOAMRAM)){
      return;
    }
    if(is_channel3_on(memoria.data()) && endereco >= WAVE_RAM_INICIO && endereco < WAVE_RAM_FIM){
      if(ppu->paleta_cgb || (timer->apu->ch3.periodo_divider > 2046 && !timer->apu->ch3.delay_hack)){
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
      case 0xFF40:{ //LCDC
        if(ppu->paleta_cgb && !(valor & 0x20))
          ppu->fetcher.gbc_window_desativada = true;

        memoria[endereco] = valor;
        return;
      }
      case 0xFF41:{ //stat
        if(!ppu->paleta_cgb && ppu->is_lcd_enabled() && 
            (ppu->modo_atual != screen_mode::DRAWING || (memoria[0xFF41] & LYC_Comparison_Signal))){
          ppu->stat_bug = 2;
          ppu->stat_cache = valor;
          memoria[endereco] |= 0x78;
        }
        else
          memoria[endereco] = (valor & 0x78) | (memoria[endereco] & 0x07);

        ppu->check_stat_interruption();
        return;
      }
      case 0xFF44:{ //ly
        return;
      }
      case 0xFF46:{ //dma
        dma.start(valor);
        return;
      }
      case 0xFF50:{ //bank
        if(valor){
          restaura_rom();
          key0_blocked = true;
          opri_blocked = true;
        }
        return;
      }
      case 0xFF4C:{ //key0
        if(key0_blocked || !ppu->modo_cpu) return;

        memoria[endereco] = valor;
        return;
      }
      case 0xFF4D:{ //key1
        if(!ppu->modo_cpu) return;
        memoria[endereco] = (memoria[endereco] & 0xFE) | (valor & 0x01);
        return;
      }
      case 0xFF55:{ //hdma
        if(!ppu->modo_cpu) return;
        if(ppu->modo_atual == screen_mode::HBLANK && (valor & 0x80)) return;
        if(hdma.modo_hblank && !(valor & 0x80)){
          hdma.ativo = false;
          hdma.modo_hblank = false;
          hdma.restante = 0;
          hdma.hblank_count = 0;
          memoria[endereco] |= 0x80;
          return;
        }

        uint16_t dest = ((memoria[0xFF53] << 8) | memoria[0xFF54]);
        uint16_t src = ((memoria[0xFF51] << 8) | memoria[0xFF52]);

        memoria[endereco] = valor & 0x7F;
        hdma.destino = dest & 0x1FF0;
        hdma.origem = src & 0xFFF0;
        hdma.init_transfer(valor);

        return;
      }
      case 0xFF69:{ //bgpd
        if(ppu->modo_atual == screen_mode::DRAWING || !ppu->paleta_cgb)
          return;

        uint8_t bgpi = memoria[0xFF68];
        uint8_t indice = bgpi & 0x3F;
        ppu->bg_palette_ram[indice] = valor;
        if(bgpi & 0x80){
          indice = (indice + 1) % 64; 
          memoria[0xFF68] = (memoria[0xFF68] & 0x80) | indice;
        }

        return;
      }
      case 0xFF6B:{ //obpd
        if(ppu->modo_atual == screen_mode::DRAWING || !ppu->paleta_cgb)
          return;
        
        uint8_t obpi = memoria[0xFF6A];
        uint8_t indice = obpi & 0x3F;
        ppu->obj_palette_ram[indice] = valor;
        if(obpi & 0x80){
          indice = (indice + 1) % 64; 
          memoria[0xFF6A] = (memoria[0xFF6A] & 0x80) | indice;
        }

        return;
      }
      case 0xFF6C:{ //opri
        if(opri_blocked || !ppu->modo_cpu) return;

        memoria[endereco] = valor;
        return;
      }
      case 0xFF74:{
        if(!ppu->modo_cpu) return;

        memoria[endereco] = valor;
        return;
      }
      case 0xFF75:{
        if(!ppu->modo_cpu) return;

        memoria[endereco] = valor & 0x70;
        return;
      }
      default: break;
    }

    memoria[endereco] = valor;
  }
};

}

#endif
