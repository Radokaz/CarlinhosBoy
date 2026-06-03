#include "ppu.h"

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

void PPU_fetcher::clear(void){
  atual = fetcher_estado::READ_ID;
  ciclos = 0;
  tiles_buscados = 0;
  size = 0;
  ultimo = 0;
  prim = 0;
  finalizado = false;
  delay = 0;
}

void PPU_fetcher::step(PPU *ppu){
  if(this->sprite_penality){
    --this->sprite_penality;
    return;
  }

  ++this->ciclos;

  if(this->ciclos < 2) return;
  this->ciclos = 0;

  switch(this->atual){
    using enum fetcher_estado;

    case READ_ID:{
      uint8_t ly = ppu->memoria[0xFF44];
      if(!this->window_ativa){
        uint8_t scx = ppu->get_scrollx();
        uint8_t scy = ppu->get_scrolly();
        uint16_t tilemap = ppu->atual_bgtilemap();

        uint8_t tilex = ((scx/8) + this->tiles_buscados) & 31;
        uint8_t tiley = (((scy + ly) % 256)/8) & 31;

        this->tile_id = ppu->memoria[tilemap + tiley*32 + tilex];
      }
      else{
        uint8_t winy = this->win_line;
        uint16_t tilemap = ppu->atual_wintilemap();

        uint8_t tilex = this->tiles_buscados & 31;
        uint8_t tiley = (winy/8) & 31;

        this->tile_id = ppu->memoria[tilemap + tiley*32 + tilex];
      }
      this->atual = fetcher_estado::READ_LOW;
      break;
    }
    case READ_LOW:{
      uint8_t ly = ppu->memoria[0xFF44];

      uint8_t y = (!this->window_ativa) ? ppu->get_scrolly() : this->win_line;
      uint16_t tilemap = (!this->window_ativa) ? ppu->atual_bgtilemap() : ppu->atual_wintilemap();

      uint8_t linha = (!this->window_ativa) ? ((y + ly) % 256) % 8 : y % 8;
      uint16_t tile_addr {};

      if(ppu->atual_bgtiledata() == 0x8000){
        tile_addr = 0x8000 + this->tile_id*16;
      }
      else{
        tile_addr = 0x9000 + static_cast<int8_t>(this->tile_id)*16;
      }
      
      this->tile_low = ppu->memoria[tile_addr + linha*2];
      this->atual = fetcher_estado::READ_HIGH;
      break;
    }
    case READ_HIGH:{
      uint8_t ly = ppu->memoria[0xFF44];

      uint8_t y = (!this->window_ativa) ? ppu->get_scrolly() : this->win_line;
      uint16_t tilemap = (!this->window_ativa) ? ppu->atual_bgtilemap() : ppu->atual_wintilemap();

      uint8_t linha = (!this->window_ativa) ? ((y + ly) % 256) % 8 : y % 8;
      uint16_t tile_addr {};

      if(ppu->atual_bgtiledata() == 0x8000){
        tile_addr = 0x8000 + this->tile_id*16;
      }
      else{
        tile_addr = 0x9000 + static_cast<int8_t>(this->tile_id)*16;
      }

      this->tile_high = ppu->memoria[tile_addr + linha*2 + 1];
      this->atual = fetcher_estado::PUSH;
      break;
    }
    case PUSH:{
      if(!this->size){
        for(size_t i {}; i < 8; ++i){
          uint8_t mask = 1 << (7 - i);
          uint8_t bit1 = ((this->tile_low & mask) >> (7 - i));
          uint8_t bit2 = ((this->tile_high & mask) >> (7 - i));
          switch((bit2 << 1) | bit1){
            case 0x00:
              this->push(tile_pixel::INDEX_ZERO);
              break;
            case 0x01:
              this->push(tile_pixel::INDEX_ONE);
              break;
            case 0x02:
              this->push(tile_pixel::INDEX_TWO);
              break;
            case 0x03:
              this->push(tile_pixel::INDEX_THREE);
              break;
          }
        }
        ++this->tiles_buscados;
        this->atual = fetcher_estado::READ_ID;
      }
      
      break;
    }
    case FLUSH:{
      //t-cicles de penalidade para o encerramento do modo 3
      --this->delay;
      if(!this->delay){
        this->finalizado = true;
      }

      break;
    }
  }
}

}
