#ifndef PPU_H
#define PPU_H

#include "lcd.h"

#define VRAM_INICIO 0x8000
#define VRAM_FINAL  0xA000

namespace GB{

enum class tile_pixel: uint8_t{
  BLACK = 0,
  LGRAY,
  DGRAY,
  WHITE
};

struct Tile{
  std::vector<uint8_t> linha1(8);
  std::vector<uint8_t> linha2(8);
  std::vector<tile_pixel> cor(8); 
};

struct PPU{
  uint8_t *vram{};
  std::array<Tile, 384> tile_set;
};

}

#endif
