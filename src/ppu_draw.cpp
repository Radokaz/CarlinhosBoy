#include "memorybus.h"
#include <algorithm>

namespace GB{

uint32_t PPU::decide_bg_color(tile_pixel px){

    uint8_t cor_final{};
    uint8_t bgp {this->bus->memoria[0xFF47]};
    switch(px){
      using namespace GB;
      using enum tile_pixel;

      case INDEX_ZERO: 
        cor_final = (bgp >> 0) & 0x03;
        break;
      case INDEX_ONE:
        cor_final = (bgp >> 2) & 0x03;
        break;
      case INDEX_TWO:
        cor_final = (bgp >> 4) & 0x03;
        break;
      case INDEX_THREE:
        cor_final = (bgp >> 6) & 0x03;;
        break;
      case INDEX_NULO:
        return 0xFF0000FF;
    }

    switch(cor_final){
      case 0:
        return 0xFFFFFFFF;
      case 1:
        return 0xFFAAAAAA;
      case 2:
        return 0xFF555555;
      case 3:
        return 0xFF000000;
      default:
        return 0xFFFFFFFF;
    }
}

uint32_t PPU::decide_obj_color(const Sprite& sprite, tile_pixel pos){
    using enum tile_pixel;
    uint8_t obj_palette = (sprite.flags & (1 << 4)) ? this->bus->memoria[0xFF49] : this->bus->memoria[0xFF48];
    uint8_t cor_final {};

    switch(pos){
      case INDEX_ZERO: 
        return 0x00; //transparente
        break;
      case INDEX_ONE:
        cor_final = (obj_palette >> 2) & 0x03;
        break;
      case INDEX_TWO:
        cor_final = (obj_palette >> 4) & 0x03;
        break;
      case INDEX_THREE:
        cor_final = (obj_palette >> 6) & 0x03;;
        break;
      case INDEX_NULO:
        return 0xFF00FF00;
    }

    switch(cor_final){
      case 0:
        return 0xFFFFFFFF;
      case 1:
        return 0xFFAAAAAA;
      case 2:
        return 0xFF555555;
      case 3:
        return 0xFF000000;
      default:
        return 0x00;
    }
}

void PPU::scan_oam(void){
  this->sprites_count = 0;
  uint8_t ly = this->bus->memoria[0xFF44];
  uint8_t sprite_sz = this->atual_spritesize();

  for(size_t i {}; i < 40 && sprites_count < 10; ++i){
    int16_t y = static_cast<int16_t>(this->bus->memoria[OAM_INICIO + i*4]) - 16;

    if(ly >= y && ly < y + sprite_sz){
      this->sprites_sel[sprites_count++] = Sprite{bus->memoria[OAM_INICIO + i*4], bus->memoria[OAM_INICIO + i*4 + 1],
          bus->memoria[OAM_INICIO + i*4 + 2], bus->memoria[OAM_INICIO + i*4 + 3]};
    }
  }
}

void PPU::draw_step(void){
  this->fetcher.step(this);

  if(this->fetcher.size <= 0) return;

  if(this->fetcher.drop_pixels > 0){
    this->fetcher.pop();
    --this->fetcher.drop_pixels;
    return;
  }

  if(this->fetcher.x_pos >= 160) return;

  uint8_t ly = this->bus->memoria[0xFF44];
  tile_pixel px = this->fetcher.pop();
  this->framebuffer[ly*160 + this->fetcher.x_pos] = this->decide_bg_color(px);

  if(this->is_sprite_enabled()){
    
  }

  ++this->fetcher.x_pos;
}

}

