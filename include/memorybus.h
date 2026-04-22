#ifndef MEMORYBUS_H
#define MEMORYBUS_H

#include "joypad.h"

//flags de interrupção
#define BIT_VBLANK (1 << 0)
#define BIT_LCDSTAT (1 << 1)
#define BIT_TIMER (1 << 2)
#define BIT_SERIAL (1 << 3)
#define BIT_JOYPAD (1 << 4)

#define VRAM_INICIO 0x8000
#define VRAM_FINAL  0xA000

#define FIRST_TILE1 0x8000
#define SECOND_TILE1 0x8800
#define SECOND_TILE2 0x9000
#define TILE_END 0x9800

namespace GB{

struct Memorybus;

enum class tile_pixel: uint8_t{
  BLACK = 0,
  LGRAY,
  DGRAY,
  WHITE,
  NULO
};

//cada tile possui 64 pixels em que cada pixel usa 2 bits para representar sua cor,
//dando 64*2 = 128 bits = 16 bytes em cada tile

struct Tile{
  std::array<tile_pixel, 64> pixels;

  Tile(){
    pixels.fill(tile_pixel::NULO);
  }
};

struct PPU_fetcher{
  std::array<tile_pixel, 16> fila;
  uint8_t ultimo{};
  uint8_t prim {};
  uint16_t size {};

  void push(tile_pixel *alvo);
  tile_pixel *pop(void);
};

struct PPU{
  std::array<Tile, (TILE_END - FIRST_TILE1)/16> tile_set;
  Memorybus *bus {};
  uint16_t ciclos {};
  uint8_t sprite_size{8};

  void write_vram(uint16_t endereco, uint8_t valor);
  
};

struct Memorybus{
  std::array<uint8_t, 0xFFFF + 1> memoria{};
  uint16_t *div_count;
  Joypad *pad {};
  PPU *ppu {};

  Memorybus(uint16_t *div, Joypad *p, PPU *pp): div_count{div}, pad{p}, ppu{pp} {}

  uint8_t& read_byte(uint16_t endereco){
    switch(endereco){
        case 0xFF07: //tac
          memoria[endereco] |= 0b11111000; 
          break;
        case 0xFF0F: //if
          memoria[endereco] |= 0b11100000; 
          break;
        case 0xFF00: //joypad
          return pad->get_output(); 
        case 0xFF41: //stat
          memoria[endereco] |= 0b10000000; 
          break;
        default: break;
    }
    return memoria[endereco];
  }

  void write_byte(uint16_t endereco, uint8_t valor){
    /*if(endereco >= VRAM_INICIO && endereco < VRAM_FINAL){
      ppu->write_vram(endereco, valor);
      return;
    }*/
    if(endereco == 0xFF04){ //div
      memoria[endereco] = 0;
      *div_count = 0;
      return;
    }
    if(endereco == 0xFF00){ //joypad
      memoria[0xFF00] = (memoria[0xFF00] & 0x0F) | (valor & 0x30);
      return;
    }
    if(endereco == 0xFF02 && (valor & 0x81) == 0x81){ //serial
      memoria[0xFF01] = 0xFF;
      memoria[0xFF02] &= ~BIT_SERIAL;
      memoria[0xFF0F] |= BIT_SERIAL;
     /* char c = memoria[0xFF01];
      std::cout << c << std::flush;
      c = memoria[0xFF02];
      std::cout << c << std::flush;
      return;*/
    }
    memoria[endereco] = valor;
  }
};

inline void PPU::write_vram(uint16_t endereco, uint8_t valor){
    bus->memoria[endereco] = valor;

    if(endereco >= TILE_END) return;

    uint16_t index = 0xFFFE & endereco;

    uint8_t byte1 = bus->memoria[index];
    uint8_t byte2 = bus->memoria[index + 1];
      
    uint16_t tile_index = (endereco - FIRST_TILE1)/16;
    uint8_t linha = ((endereco - FIRST_TILE1) % 16)/2;

    for(size_t i {}; i < 8; ++i){
      uint8_t mask = 1 << (7 - i);
      uint8_t bit1 = ((byte1 & mask) >> (7 - i));
      uint8_t bit2 = ((byte2 & mask) >> (7 - i));
      switch((bit1 << 1) | bit2){
        case 0x00:
          tile_set[tile_index].pixels[linha*8 + i] = tile_pixel::BLACK;
          break;
        case 0x01:
          tile_set[tile_index].pixels[linha*8 + i] = tile_pixel::LGRAY;
          break;
        case 0x02:
          tile_set[tile_index].pixels[linha*8 + i] = tile_pixel::DGRAY;
          break;
        case 0x03:
          tile_set[tile_index].pixels[linha*8 + i] = tile_pixel::WHITE;
          break;
      }
    }

}



}

#endif
