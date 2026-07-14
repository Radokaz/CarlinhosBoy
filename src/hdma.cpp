#include "memorybus.h"

namespace GB{

void HDMA::init_transfer(uint8_t vdma){
  uint16_t tamanho = ((vdma & 0x7F) + 1)*0x10;
  modo_hblank = (vdma & 0x80) != 0;
  restante = tamanho;
  if(modo_hblank)
    hblank_count = 16;
  else
    ativo = true;
}

void HDMA::step(Memorybus *bus){
  if(!ativo) return;
  
  size_t offset {2};
  if(bus->memoria[0xFF4D] & 0x80)
    offset = 1;

  if(!modo_hblank){
    for(size_t i {}; i < offset; ++i){
      uint8_t valor = bus->read_byte(origem);
      bus->ppu->write_vram(0x8000 + destino, valor, true);
      ++destino;
      ++origem;
      --restante;
      if(!restante){
        bus->memoria[0xFF55] = 0xFF;
        ativo = false;
        return;
      }
      
      if(0x8000 + destino >= VRAM_FINAL){
        ativo = false;
        hblank_count = 0;
        modo_hblank = false;
        bus->memoria[0xFF55] = 0xFF;
        return;
      }
    }
  }
  else{
    for(size_t i {}; i < offset; ++i){
      uint8_t valor = bus->read_byte(origem);
      bus->ppu->write_vram(0x8000 + destino, valor, true);
      ++destino;
      ++origem;
      --restante;
      --hblank_count;
      if(!restante){
        ativo = false;
        modo_hblank = false;
        hblank_count = 0;
        bus->memoria[0xFF55] = 0xFF;
        return;
      }
      if(!hblank_count){
        ativo = false;
        hblank_count = 16;
        bus->memoria[0xFF55] = (bus->memoria[0xFF55] & 0x7F) - 1;
        return;
      }
      if(0x8000 + destino >= VRAM_FINAL){
        ativo = false;
        hblank_count = 0;
        modo_hblank = false;
        bus->memoria[0xFF55] = 0xFF;
        return;
      }
    }
  }
}

}
