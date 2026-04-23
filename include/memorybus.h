#ifndef MEMORYBUS_H
#define MEMORYBUS_H

#include "joypad.h"
#include "lcd.h"
#include "dma.h"
#include <raylib.h>

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
#define OAM_FIM 0xFE9F

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
    fila.fill(tile_pixel::NULO);
  }

  void push(tile_pixel alvo);
  tile_pixel pop(void);
  void clear(void);
};

struct PPU{
  std::array<Color, 160*144> framebuffer{};
  std::array<Tile, (TILE_END - FIRST_TILE1)/16> tileset;
  PPU_fetcher fetcher;
  std::array<Sprite, 10> sprites_sel{};
  uint8_t sprites_count {};
  Memorybus *bus {};
  uint16_t ciclos {};
  screen_mode modo_atual {screen_mode::SOAMRAM};
  bool stat_prev {false};

  void write_vram(uint16_t endereco, uint8_t valor);
  void step(uint8_t cpu_ciclos);
  void scan_oam(void);
  void merge_sprites(std::array<tile_pixel, 160>& pixels);
  void draw_line(void);
  void ppu_draw(const std::array<tile_pixel, 160>& pixels);

  uint16_t atual_wintilemap(void){
    return (bus->memoria[0xFF40] & LCDC_WIN_MAP) ? 0x9C00 : 0x9800;
  }

  uint16_t atual_bgtiledata(void){
    return (bus->memoria[0xFF40] & LCDC_TILE_DATA) ? 0x8000 : 0x8800;
  }

  uint16_t atual_bgtilemap(void){
    return (bus->memoria[0xFF40] & LCDC_BG_MAP) ? 0x9C00 : 0x9800;
  }

  uint8_t atual_spritesize(void){
    return (bus->memoria[0xFF40] & LCDC_OBJ_SIZE) ? 16 : 8;
  }

  uint8_t& get_scrolly(void) { return bus->memoria[0xFF42]; }
  uint8_t& get_scrollx(void) { return bus->memoria[0xFF43]; }

  void set_mode(screen_mode modo){
    uint8_t& stat = bus->memoria[0xFF41];
    stat = (stat & 0b11111100) | std::to_underlying<screen_mode>(modo);
    this->modo_atual = modo;
    this->check_stat_interruption();
  }

  screen_mode get_mode(void){
    return static_cast<screen_mode>(bus->memoria[0xFF41] & 0x03);
  }

  bool check_stat(void){
    if((bus->memoria[0xFF40] & LCDC_ENABLE) != LCDC_ENABLE)
      return false;

    uint8_t ly = bus->memoria[0xFF44];
    uint8_t lyc = bus->memoria[0xFF45];
    uint8_t& stat = bus->memoria[0xFF41];
    screen_mode atual = get_mode(bus);
  
    return (((ly == lyc) && (stat & LYC_Comparison_Signal)) ||
    ((atual == screen_mode::HBLANK) && (stat & HBLANK_ENABLE )) ||
    ((atual == screen_mode::SOAMRAM) && (stat & OAM_ENABLE)) ||
    ((atual == screen_mode::VBLANK) && ((stat & VBLANK_ENABLE) || (stat & OAM_ENABLE))));
  }

  void check_stat_interruption(void){
    bool stat_atual = this->check_stat();
    if(stat_atual && !stat_prev)
      bus->memoria[0xFF0F] |= BIT_LCDSTAT;

    stat_prev = stat_atual;
  }

  //ly é o registrador que marca a linha sendo scaneada no momento
  void avanca_ly(void){
    uint8_t ly = bus->memoria[0xFF44];
    ++ly;

    if(ly > 153)
      ly = 0;

    this->check_stat_interruption();
  }

};

struct Memorybus{
  std::array<uint8_t, 0xFFFF + 1> memoria{};
  uint16_t *div_count;
  uint8_t dma_hack {0xFF};
  Joypad *pad {};
  PPU *ppu {};
  DMA *dma;

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
    if(endereco >= OAM_INICIO && endereco < OAM_FIM && dma->ativo){
      dma_hack = 0xFF;
      return dma_hack;
    }
    return memoria[endereco];
  }

  void write_byte(uint16_t endereco, uint8_t valor){
    if(endereco >= VRAM_INICIO && endereco < VRAM_FINAL){
      if(ppu->modo_atual != screen_mode::HBLANK)
        ppu->write_vram(endereco, valor);
      else
        memoria[endereco] = valor;

      return;
    }
    if(endereco >= OAM_INICIO && endereco < OAM_FIM && dma->ativo){
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
      memoria[0xFF01] = 0xFF;
      memoria[0xFF02] &= ~BIT_SERIAL;
      memoria[0xFF0F] |= BIT_SERIAL;
     /* char c = memoria[0xFF01];
      std::cout << c << std::flush;
      c = memoria[0xFF02];
      std::cout << c << std::flush;*/
      return;
    }
    if(endereco == 0xFF46){
      dma->start(valor);
      return;
    }
    memoria[endereco] = valor;
  }
};

void draw_ppu(std::array<tile_pixel, 160> pixels);

}

#endif
