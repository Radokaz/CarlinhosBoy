#include "memorybus.h"

namespace GB{

void PPU_fetcher::push(tile_pixel alvo){
  fila[ultimo] = alvo;
  ultimo = (ultimo + 1) % std::size(fila);
  ++size;
}

tile_pixel *PPU_fetcher::pop(void){
  tile_pixel result = fila[prim];
  prim = (prim + 1) % std::size(fila);
  --size;
  return result;
}

void PPU::write_vram(uint16_t endereco, uint8_t valor){
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
      switch((bit2 << 1) | bit1){
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

void PPU::scan_oam(void){
  uint8_t sprites_count {};
  uint8_t ly = this->bus->memoria[0xFF44];
  uint8_t sprite_sz = this->atual_spritesize();

  for(size_t i {}; i < 40 && sprites_count < 10; ++i){
    uint8_t y = this->bus->memoria[OAM_INICIO + i*4];

    if(ly >= y - 16 && ly < y - 16 + sprite_sz){
      this->sprites_sel[sprites_count++] = Sprite(y, bus->memoria[OAM_INICIO + i*4 + 1],
          bus->memoria[OAM_INICIO + i*4 + 2], bus->memoria[OAM_INICIO + i*4 + 3]);
    }
  }
}

void PPU::fetch(void){
  uint16_t endereco = this->atual_tilemap();
  uint16_t ly = this->bus->memoria[0xFF44];
  uint8_t& scy = this->get_scrolly();
  uint8_t linha_atual = (ly + scy) % 256;
  uint16_t tile_index = (endereco - FIRST_TILE1)/16;

  if(fetcher.size <= 8){
    for(size_t i {}; i < 8; ++i){
      tile_pixel pixel = tile_set[tile_index].pixels[linha_atual * 8 + i];
      fetcher.push(pixel);
    }
  }

}

}
