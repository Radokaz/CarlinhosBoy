#ifndef MEMORYBUS_H
#define MEMORYBUS_H

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <array>
#include <stdexcept>
#include "joypad.h"
#include "lcd.h"
#include "dma.h"
#include <raylib.h>
#include <memory>

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

#define OAM_INICIO 0xFE00
#define OAM_FIM 0xFEA0

namespace GB{

enum class tile_pixel: uint8_t{
  INDEX_ZERO = 0,
  INDEX_ONE,
  INDEX_TWO,
  INDEX_THREE,
  INDEX_NULO
};

//cada tile possui 64 pixels em que cada pixel usa 2 bits para representar sua cor,
//dando 64*2 = 128 bits = 16 bytes em cada tile

struct Tile{
  std::array<std::array<tile_pixel, 8>, 8> pixels;

  Tile(){
    for(size_t i {}; i < pixels.size(); ++i)
      pixels[i].fill(tile_pixel::INDEX_NULO);
  }
};

struct Sprite{
  uint8_t y;
  uint8_t x;
  uint8_t tile_index;
  uint8_t flags;
};

struct PPU_fetcher{
  std::array<tile_pixel, 16> fila;
  uint8_t ultimo{};
  uint8_t prim {};
  uint8_t size {};

  PPU_fetcher(){
    fila.fill(tile_pixel::INDEX_NULO);
  }

  void push(tile_pixel alvo);
  tile_pixel pop(void);
  void clear(void);
};

struct Memorybus;

struct PPU{
  std::array<std::array<uint32_t, 160>, 144> framebuffer;
  std::array<Tile, (TILE_END - FIRST_TILE1)/16> tileset{};
  PPU_fetcher fetcher{};
  std::array<Sprite, 10> sprites_sel{};
  uint8_t sprites_count {};
  uint8_t win_line {};
  Memorybus *bus {};
  uint16_t ciclos {};
  screen_mode modo_atual {screen_mode::SOAMRAM};
  bool stat_prev {false};

  PPU(){
    for(size_t i {}; i < framebuffer.size(); ++i)
      framebuffer[i].fill(0xFFFFFFFF);
  }

  void write_vram(uint16_t endereco, uint8_t valor);
  void step(uint8_t cpu_ciclos, Texture2D& texture);
  void scan_oam(void);
  void discard_first_tile(void);
  void merge_sprites();
  void draw_window(std::array<tile_pixel, 160>& pixels);
  void draw_background(std::array<tile_pixel, 160>& pixels);
  void draw_framebuffer(const std::array<tile_pixel, 160>& pixels);
  void draw_line(void);

  uint16_t atual_wintilemap(void);
  uint16_t atual_bgtiledata(void);
  uint16_t atual_bgtilemap(void);
  uint8_t atual_spritesize(void);
  bool is_lcd_enabled(void);
  bool is_win_enabled(void);
  bool is_bg_enabled(void);
  bool is_sprite_enabled(void);
  uint8_t& get_scrolly(void);
  uint8_t& get_scrollx(void);
  uint8_t& get_winx(void);
  uint8_t& get_winy(void);
  void set_mode(screen_mode modo);
  bool check_stat(void);
  void check_stat_interruption(void);
  //ly é o registrador que marca a linha sendo scaneada no momento
  void avanca_ly(void);
  uint32_t decide_bg_color(tile_pixel px);
  uint32_t decide_obj_color(const Sprite& sprite, tile_pixel pos);
};

struct Memorybus{
  std::array<uint8_t, 0xFFFF + 1> memoria{};
  uint16_t *div_count;
  uint8_t dma_hack {0xFF};
  Joypad *pad {};
  PPU *ppu {};
  std::unique_ptr<DMA> dma;

  Memorybus(uint16_t *div, Joypad *p, PPU *pp): div_count{div}, pad{p}, ppu{pp} {
    dma = std::make_unique<DMA>();
  }

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
    if(endereco >= OAM_INICIO && endereco < OAM_FIM && (dma->ativo || ppu->modo_atual == screen_mode::DRAWING || ppu->modo_atual == screen_mode::SOAMRAM)){
      dma_hack = 0xFF;
      return dma_hack;
    }
    if(endereco >= VRAM_INICIO && endereco < VRAM_FINAL && (dma->ativo || ppu->modo_atual == screen_mode::DRAWING)){
      dma_hack = 0xFF;
      return dma_hack;
    }

    return memoria[endereco];
  }

  void write_byte(uint16_t endereco, uint8_t valor){
    if(endereco >= VRAM_INICIO && endereco < VRAM_FINAL){
      if(ppu->modo_atual != screen_mode::DRAWING)
        ppu->write_vram(endereco, valor);

      return;
    }
    if(endereco >= OAM_INICIO && endereco < OAM_FIM && (dma->ativo || ppu->modo_atual == screen_mode::DRAWING || ppu->modo_atual == screen_mode::SOAMRAM)){
      return;
    }
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
      char c = memoria[0xFF01];
      std::cout << c << std::flush;
      memoria[0xFF01] = 0xFF;
      memoria[0xFF02] &= ~BIT_SERIAL;
      memoria[0xFF0F] |= BIT_SERIAL;
      return;
    }
    if(endereco == 0xFF46){ //dma
      dma->start(valor);
      return;
    }
    memoria[endereco] = valor;
  }
};

}

#endif
