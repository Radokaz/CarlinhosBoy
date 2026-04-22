#ifndef MEMORYBUS_H
#define MEMORYBUS_H

#include "joypad.h"
#include "lcd.h"

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

  PPU_fetcher(){
    fila.fill(tile_pixel::NULO);
  }

  void push(tile_pixel *alvo);
  tile_pixel *pop(void);
};

struct PPU{
  std::array<Tile, (TILE_END - FIRST_TILE1)/16> tile_set;
  PPU_fetcher fetcher;
  Memorybus *bus {};
  uint16_t ciclos {};
  uint8_t sprite_size{8};
  bool stat_prev {false};

  void write_vram(uint16_t endereco, uint8_t valor);
  void step();
  void fetch();

  uint16_t atual_tilemap(void){
    return (bus.read_byte(0xFF40) & LCDC_WIN_MAP) ? 0x9C00 : 0x9800;
  }

  uint16_t atual_bgtiledata(void){
    return (bus.read_byte(0xFF40) & LCDC_TILE_DATA) ? 0x8000 : 0x8800;
  }

  uint16_t atual_bgtilemap(void){
    return (bus.read_byte(0xFF40) & LCDC_BG_MAP) ? 0x9C00 : 0x9800;
  }

  uint8_t atual_spritesize(void){
    return (bus.read_byte(0xFF40) & LCDC_OBJ_SIZE) ? 16 : 8;
  }

  void set_screen(screen_mode modo){
    uint8_t& stat = bus.read_byte(0xFF41);
    stat = (stat & 0b11111100) | std::to_underlying<screen_mode>(modo);
  }

  screen_mode get_mode(void){
    return static_cast<screen_mode>(bus.read_byte(0xFF41) & 0x03);
  }

  bool check_stat(void){
    if((bus.read_byte(0xFF40) & LCDC_ENABLE) != LCDC_ENABLE)
      return false;

    uint8_t ly = bus.read_byte(0xFF44);
    uint8_t lyc = bus.read_byte(0xFF45);
    uint8_t& stat = bus.read_byte(0xFF41);
    screen_mode atual = get_mode(bus);
  
    return (((ly == lyc) && (stat & LYC_Comparison_Signal)) ||
    ((atual == screen_mode::HBLANK) && (stat & HBLANK_ENABLE )) ||
    ((atual == screen_mode::SOAMRAM) && (stat & OAM_ENABLE)) ||
    ((atual == screen_mode::VBLANK) && ((stat & VBLANK_ENABLE) || (stat & OAM_ENABLE))));
  }

  void check_stat_interruption(void){
    bool stat_atual = this->check_stat();
    if(stat_atual && !stat_prev)
      cpu->get_if() |= BIT_LCDSTAT;

    stat_prev = stat_atual;
  }

  void aumenta_ly(void){
    uint8_t ly = void.read_byte(0xFF44);
    ++ly;
    if(ly > 153)
      ly = 0;
  }

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
      std::cout << c << std::flush;*/
      return;
    }
    memoria[endereco] = valor;
  }
};


}

#endif
