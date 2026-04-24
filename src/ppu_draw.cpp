#include "memorybus.h"

uint32_t tile_to_color(GB::tile_pixel px, GB::Memorybus *bus){

    uint8_t cor_final{};
    uint8_t bgp {bus->read_byte(0xFF47)};
    switch(px){
      using namespace GB;
      using enum tile_pixel;

      case PX_WHITE: 
        cor_final = (bgp >> 0) & 0x03;
        break;
      case PX_LGRAY:
        cor_final = (bgp >> 2) & 0x03;
        break;
      case PX_DGRAY:
        cor_final = (bgp >> 4) & 0x03;
        break;
      case PX_BLACK:
        cor_final = (bgp >> 6) & 0x03;;
        break;
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

namespace GB{

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

void PPU::merge_sprites(std::array<tile_pixel, 160>& pixels){

  uint8_t ly = this->bus->memoria[0xFF44];
  uint8_t sprite_altura = this->atual_spritesize();
  auto screenx = [](Sprite s) -> int16_t{ return static_cast<int16_t>(s.x) - 8; };
  auto screeny = [](Sprite s) -> int16_t{ return static_cast<int16_t>(s.y) - 16; };
  //TODO

}

void PPU::draw_line(void){

  std::array<tile_pixel, 160> px_prontos;
  px_prontos.fill(tile_pixel::PX_WHITE);

  uint8_t ly = this->bus->memoria[0xFF44];
  uint8_t scy = this->get_scrolly();
  uint8_t scx = this->get_scrollx();
  uint16_t tiledata = this->atual_bgtiledata();
  uint16_t tilemap = this->atual_bgtilemap();

  //TODO

  if(this->is_sprite_enabled())
    this->merge_sprites(px_prontos);
  this->ppu_draw(px_prontos);
}

void PPU::ppu_draw(const std::array<tile_pixel, 160>& pixels){
  uint8_t ly = this->bus->memoria[0xFF44];
  for(size_t i {}; i < 160; ++i){
    this->framebuffer[160*ly + i] = tile_to_color(pixels[i], this->bus);
  }
}

}

