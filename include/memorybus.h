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
#include <functional>

//flags de interrupção
#define BIT_VBLANK (1 << 0)
#define BIT_LCDSTAT (1 << 1)
#define BIT_TIMER (1 << 2)
#define BIT_SERIAL (1 << 3)
#define BIT_JOYPAD (1 << 4)

#define VRAM_INICIO 0x8000
#define VRAM_FINAL  0xA000

#define WRAM_INICIO 0xC000
#define WRAM_FINAL 0xD000

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

enum class fetcher_estado: uint8_t{
  READ_ID,
  READ_LOW,
  READ_HIGH,
  PUSH
};

//cada tile possui 64 pixels em que cada pixel usa 2 bits para representar sua cor,
//dando 64*2 = 128 bits = 16 bytes em cada tile

struct Sprite{
  uint8_t y;
  uint8_t x;
  uint8_t tile_index;
  uint8_t flags;
};

struct Memorybus;
struct PPU;

struct PPU_fetcher{
  std::array<tile_pixel, 16> fila;
  uint8_t ultimo{};
  uint8_t prim {};
  uint8_t size {};

  uint8_t tile_id {};
  uint8_t tile_low {};
  uint8_t tile_high {};

  uint8_t ciclos {};
  uint8_t x_pos {};
  uint8_t tiles_buscados {};
  uint8_t drop_pixels {};
  uint8_t win_line {};
  fetcher_estado atual {fetcher_estado::READ_ID};
  bool window_ativa {false};

  PPU_fetcher(){
    fila.fill(tile_pixel::INDEX_NULO);
  }

  void push(tile_pixel alvo);
  tile_pixel pop(void);
  void clear(void);

  void step(PPU *ppu);
};


struct PPU{
  std::array<uint32_t, 160*144> framebuffer;
  PPU_fetcher fetcher{};
  std::array<Sprite, 10> sprites_sel{};
  uint8_t sprites_count {};
  Memorybus *bus {};
  Texture2D *raylib_texture;
  uint16_t ciclos {};
  uint16_t draw_ciclos {};
  screen_mode modo_atual {screen_mode::SOAMRAM};
  bool stat_prev {false};
  bool frame_pronto {false};

  PPU(Texture2D *texture): raylib_texture{texture}{
    framebuffer.fill(0xFFFFFFFF);
  }

  void write_vram(uint16_t endereco, uint8_t valor);
  void step(void);
  void scan_oam(void);
  uint32_t merge_sprites(uint8_t x_atual, tile_pixel bg_cor);
  void draw_step(void);

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

struct Timer{
    uint16_t div_count {0xAB00};
    bool prev_bit {};
    bool timaoverflow {};
    uint8_t timaoverflow_count {};

    void step(Memorybus& bus);
    uint8_t get_div(void) { return static_cast<uint8_t>((div_count >> 8) & 0xFF); }
};

struct Memorybus{
  std::array<uint8_t, 0xFFFF + 1> memoria{};
  Timer *timer;
  Joypad *pad {};
  PPU *ppu {};
  std::unique_ptr<DMA> dma;
  std::function<void()> restaura_rom;
  uint8_t dma_hack {0xFF};
  uint8_t serial_count {};

  Memorybus(Timer *tm, Joypad *p, PPU *pp): timer{tm}, pad{p}, ppu{pp} {
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
    if(endereco >= VRAM_INICIO && endereco < VRAM_FINAL && (ppu->modo_atual == screen_mode::DRAWING)){
      dma_hack = 0xFF;
      return dma_hack;
    }

    return memoria[endereco];
  }

  void write_byte(uint16_t endereco, uint8_t valor){
    if(endereco >= VRAM_INICIO && endereco < VRAM_FINAL){
      //if(ppu->modo_atual != screen_mode::DRAWING)
        ppu->write_vram(endereco, valor);
      
      return;
    }
    else if(endereco >= OAM_INICIO && endereco < OAM_FIM && (dma->ativo || ppu->modo_atual == screen_mode::DRAWING || ppu->modo_atual == screen_mode::SOAMRAM)){
      return;
    }
    else if(endereco == 0xFF04){ //div
      memoria[endereco] = 0;
      timer->div_count = 0;
      return;
    }
    else if(endereco == 0xFF00){ //joypad
      memoria[0xFF00] = (memoria[0xFF00] & 0x0F) | (valor & 0x30);
      return;
    }
    else if(endereco == 0xFF02 && (valor & 0x81) == 0x81){ //serial
      char c = memoria[0xFF01];
      std::cout << c << std::flush;
      memoria[0xFF02] = valor;
      serial_count = 128;
      return;
    }
    else if(endereco == 0xFF46){ //dma
      dma->start(valor);
      return;
    }
    else if(endereco == 0xFF50 && valor != 0){
      restaura_rom();
      return;
    }
    memoria[endereco] = valor;
  }
};

}

#endif
