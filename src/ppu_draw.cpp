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

void PPU::discard_first_tile(void){
  uint8_t ly = this->bus->memoria[0xFF44];
  constexpr uint8_t max_tiles = 32;

  uint8_t first_tilex = (this->get_scrollx()/8) % max_tiles;
  uint8_t first_tiley = (this->get_scrolly() + ly) % (max_tiles*8);
  uint16_t first_tile_i = static_cast<uint16_t>(bus->memoria[this->atual_bgtilemap() + (first_tiley/8)*32 + first_tilex]);
  if(this->atual_bgtiledata() == 0x8800)
    first_tile_i = static_cast<uint16_t>(static_cast<int16_t>(first_tile_i) + 256);

  this->fetcher.clear();
  for(size_t i {}; i < 8; ++i){
    this->fetcher.push(tileset[first_tile_i].pixels[first_tiley % 8][i]);
  }

  for(size_t i {}; i < (this->get_scrollx() % 8); ++i){
    this->fetcher.pop();
  }
}

void PPU::merge_sprites(){

  uint8_t ly = this->bus->memoria[0xFF44];
  uint8_t sprite_altura = this->atual_spritesize();

  std::stable_sort(sprites_sel.begin(), sprites_sel.begin() + sprites_count, [](const Sprite& a, const Sprite& b){
      if(a.x == b.x) return false;
      return a.x < b.x;
      });

  for(size_t i {}; i < sprites_count; ++i){
    //TODO
  }
}

void PPU::draw_window(std::array<tile_pixel, 160>& pixels){
  constexpr uint8_t max_tiles = 32;
  constexpr uint8_t max_pixels = 160;

  int16_t tela_x = static_cast<int16_t>(static_cast<int16_t>(this->get_winx()) - 7);
  uint16_t tela_y = static_cast<uint16_t>(this->get_winy());
  uint16_t tiledata = this->atual_bgtiledata(); //se o tile é signed ou unsigned
  uint16_t tilemap = this->atual_wintilemap();
  uint8_t ly = this->bus->memoria[0xFF44];

  for(size_t x {}; x < max_pixels; ++x){
    
  }
}

void PPU::draw_background(std::array<tile_pixel, 160>& pixels){
  constexpr uint8_t max_tiles = 32;
  constexpr uint8_t max_pixels = 160;

  uint16_t tiledata = this->atual_bgtiledata(); //se o tile é signed ou unsigned
  uint16_t tilemap = this->atual_bgtilemap();
  uint16_t tela_x = static_cast<uint16_t>(this->get_scrollx());
  uint16_t tela_y = static_cast<uint16_t>(this->get_scrolly());
  uint8_t ly = this->bus->memoria[0xFF44];

  uint16_t tiles_carregados {};
  this->discard_first_tile();
  ++tiles_carregados;

  for(size_t x {}; x < max_pixels; ++x){
    if(this->fetcher.size <= 8){
      uint8_t tilex = ((tela_x/8) + tiles_carregados) % max_tiles;
      uint8_t tiley = (tela_y + ly) % (max_tiles*8);
      uint16_t tile_index = static_cast<uint16_t>(bus->memoria[tilemap + (tiley/8)*32 + tilex]);
      if(tiledata == 0x8800)
        tile_index = static_cast<uint16_t>(static_cast<int8_t>(tile_index) + 256);

      for(size_t i {}; i < 8; ++i)
        this->fetcher.push(tileset[tile_index].pixels[tiley % 8][i]);

      ++tiles_carregados;
    }

      pixels[x] = this->fetcher.pop();
  }
}

void PPU::draw_line(void){
  
  std::array<tile_pixel, 160> px_prontos;
  px_prontos.fill(tile_pixel::INDEX_NULO);
 
  uint8_t ly = this->bus->memoria[0xFF44];
  
  if(this->is_bg_enabled())
    this->draw_background(px_prontos);
    
  if(this->is_win_enabled() && ly >= this->get_winy())
    this->draw_window(px_prontos);  
  
  this->draw_framebuffer(px_prontos);
  if(this->is_sprite_enabled())
    this->merge_sprites();
}

void PPU::draw_framebuffer(const std::array<tile_pixel, 160>& pixels){
  uint8_t ly = this->bus->memoria[0xFF44];
  for(size_t i {}; i < 160; ++i){
    this->framebuffer[ly][i] = this->decide_bg_color(pixels[i]);
  }
}

}

