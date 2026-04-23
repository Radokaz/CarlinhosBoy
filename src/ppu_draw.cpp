#include "memorybus.h"

void PPU::merge_sprites(std::array<tile_pixel, 160>& pixels){

  uint8_t ly = this->bus->memoria[0xFF44];
  for(size_t i {}; i < this->sprites_count; ++i){
    Sprite& sprite = this->sprites_sel[i];

    uint8_t x {sprite.x - 8};
    uint8_t y {sprite.y - 16};

    uint8_t sprite_line = ly - y;
    uint8_t sprite_size = this->atual_spritesize();

    if(sprite.flags & (1 << 6)){
      sprite_line = (sprite_size - 1) - sprite_line;
    }

    uint8_t tile_index = sprite.tile_index;

    for(int px = 0; px < 8; ++px){

      int screen_x = x + px;

      if(screen_x < 0 || screen_x >= 160) 
        continue;

      int pixel_index = px;

      if(sprite.flags & (1 << 5)){
        pixel_index = 7 - px;
      }

      tile_pixel cor = tile_set[tile_index].pixels[sprite_line * 8 + pixel_index];

      if(cor == tile_pixel::BLACK) 
        continue;

      bool bg_priority = sprite.flags & (1 << 7);

      if(bg_priority && pixels[screen_x] != tile_pixel::BLACK)
        continue;

      pixels[screen_x] = cor;
    }
  }
}

void PPU::draw_line(void){

  uint8_t ly = this->bus->memoria[0xFF44];
  uint8_t scy = this->get_scrolly();
  uint8_t scx = this->get_scrollx();
  uint16_t tiledata = this->atual_bgtiledata();
  uint16_t tilemap = this->atual_bgtilemap();

  uint8_t tilemap_line = (ly + scy) % 256;
  uint8_t tile_line = tilemap_line / 8;
  uint8_t tile_line_index = tilemap_line % 8;
  uint8_t pixel_offset = scx % 8;
  uint8_t first_tile = (tilemap == 0x8800) ? static_cast<int8_t>(this->bus->memoria[tilemap + tile_line*32 + (scx/8) % 32]) + 128 :
    this->bus->memoria[tilemap + tile_line*32 + (scx/8) % 32];

  fetcher.clear();

  for(size_t i {}; i < 8; ++i){
      fetcher.push(tileset[first_tile].pixels[tile_line_index*8 + i]);
  }

  for(size_t i {}; fetcher.size > 0 && i < pixel_offset; ++i){
      fetcher.pop();
  }

  for(size_t i {}; i < 160; ++i){

    uint8_t tile_offset = ((scx + i)/8) % 32;
    uint8_t tile_index = this->bus->memoria[tilemap + tile_line*32 + tile_offset];
    if(tiledata == 0x8800){
      tile_index = static_cast<int8_t>(tile_index) + 128;
    }

    if(this->fetcher.size <= 8){
      for(size_t j {}; j < 8; ++j){
        fetcher.push(this->tileset[tile_index].pixels[tile_line_index*8 + j]);
      }
    }

    if(fetcher.size > 0)
      px_prontos[i] = fetcher.pop();
  }

  this->merge_sprites(px_prontos);
  return px_prontos;
}

void PPU::ppu_draw(const std::array<tile_pixel, 160>& pixels){
  uint8_t ly = this->bus->memoria[0xFF44];
  for(size_t i {}; i < 160; ++i){
    this->framebuffer[160*ly + i] = tile_to_color(pixels[i])
  }
}

}

Color tile_to_color(tile_pixel px){
    switch(px){
      using enum tile_pixel;
        case WHITE:  return {0xFF, 0xFF, 0xFF, 0xFF};
        case LGRAY:  return {0xAA, 0xAA, 0xAA, 0xFF};
        case DGRAY:  return {0x55, 0x55, 0x55, 0xFF};
        case BLACK:  return {0x00, 0x00, 0x00, 0xFF};
        default:                 return {0xFF, 0xFF, 0xFF, 0xFF};
    }
}


