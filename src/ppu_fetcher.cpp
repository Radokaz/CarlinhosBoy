#include "memorybus.h"

namespace GB{

void PPU_fetcher::push(tile_pixel alvo){
  fila[ultimo] = alvo;
  ultimo = (ultimo + 1) % std::size(fila);
  ++size;
}

tile_pixel PPU_fetcher::pop(void){
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
      this->sprites_sel[sprites_count++] = Sprite{y, bus->memoria[OAM_INICIO + i*4 + 1],
          bus->memoria[OAM_INICIO + i*4 + 2], bus->memoria[OAM_INICIO + i*4 + 3]};
    }
  }
}

void PPU::draw(void){
  while(this->fetcher.contador < 160){
    uint16_t tile_index = (16*i - FIRST_TILE1)/16;
    uint8_t linha = ((16*i) % 16)/2;

    if(this->fetcher.size <= 8){
      for(size_t i {}; i < 8; ++i){
        fetcher.push(this->tileset[tile_index].pixels[linha*8 + i]);
      }
    }

    ++fetcher;
  }

  //TODO
}

void PPU::step(uint8_t cpu_ciclos){
  this->ciclos+=cpu_ciclos;

  switch(this->modo_atual){
    using enum screen_mode;

    case SOAMRAM:{
      if(this->ciclos >= 80){
        this->ciclos -= 80;
        this->set_mode(screen_mode::DRAWING);
      }
      break;
    }
    case DRAWING:{
      if(this->ciclos >= 172){
        this->ciclos -= 172;
        this->set_mode(screen_mode::HBLANK);
      }
      break;
    }
    case HBLANK:{
      if(this->ciclos >= 204){
        this->ciclos -= 204;
        this->avanca_ly();
        if(this->bus->memoria[0xFF44] == 144){
          this->bus->memoria[0xFF0F] |= BIT_VBLANK;
          this->set_mode(screen_mode::VBLANK);
        }
        else{
          this->set_mode(screen_mode::SOAMRAM);
        }
      }
      break;
    }
    case VBLANK:{
      if(this->ciclos >= 456){
        this->ciclos -= 456;
        this->avanca_ly();
        if(this->bus->memoria[0xFF44] == 0x00){
          this->set_mode(screen_mode::SOAMRAM);
        }
      }
      break;
    }
  }
}

}
