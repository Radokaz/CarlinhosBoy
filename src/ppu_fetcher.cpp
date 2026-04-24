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

void PPU_fetcher::clear(void){
  size = 0;
  ultimo = 0;
  prim = 0;
}

uint16_t PPU::atual_wintilemap(void){
    return (bus->memoria[0xFF40] & LCDC_WIN_MAP) ? 0x9C00 : 0x9800;
}

uint16_t PPU::atual_bgtiledata(void){
    return (bus->memoria[0xFF40] & LCDC_TILE_DATA) ? 0x8000 : 0x8800;
}

uint16_t PPU::atual_bgtilemap(void){
    return (bus->memoria[0xFF40] & LCDC_BG_MAP) ? 0x9C00 : 0x9800;
}

uint8_t PPU::atual_spritesize(void){
    return (bus->memoria[0xFF40] & LCDC_OBJ_SIZE) ? 16 : 8;
}

uint8_t& PPU::get_scrolly(void) { return bus->memoria[0xFF42]; }
uint8_t& PPU::get_scrollx(void) { return bus->memoria[0xFF43]; }

void PPU::set_mode(screen_mode modo){
    uint8_t& stat = bus->memoria[0xFF41];
    stat = (stat & 0b11111100) | std::to_underlying<screen_mode>(modo);
    this->modo_atual = modo;
    this->check_stat_interruption();
}

bool PPU::check_stat(void){
    if((bus->memoria[0xFF40] & LCDC_ENABLE) != LCDC_ENABLE)
      return false;

    uint8_t ly = bus->memoria[0xFF44];
    uint8_t lyc = bus->memoria[0xFF45];
    uint8_t& stat = bus->memoria[0xFF41];
  
    return (((ly == lyc) && (stat & LYC_Comparison_Signal)) ||
    ((this->modo_atual == screen_mode::HBLANK) && (stat & HBLANK_ENABLE )) ||
    ((this->modo_atual == screen_mode::SOAMRAM) && (stat & OAM_ENABLE)) ||
    ((this->modo_atual == screen_mode::VBLANK) && ((stat & VBLANK_ENABLE) || (stat & OAM_ENABLE))));
}

void PPU::check_stat_interruption(void){
    bool stat_atual = this->check_stat();
    if(stat_atual && !stat_prev)
      bus->memoria[0xFF0F] |= BIT_LCDSTAT;

    stat_prev = stat_atual;
}

void PPU::avanca_ly(void){
    uint8_t& ly = bus->memoria[0xFF44];
    ly = (ly + 1) % 154;
    this->check_stat_interruption();
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
      switch((bit1 << 1) | bit2){
        case 0x00:
          tileset[tile_index].pixels[linha*8 + i] = tile_pixel::PX_BLACK;
          break;
        case 0x01:
          tileset[tile_index].pixels[linha*8 + i] = tile_pixel::PX_LGRAY;
          break;
        case 0x02:
          tileset[tile_index].pixels[linha*8 + i] = tile_pixel::PX_DGRAY;
          break;
        case 0x03:
          tileset[tile_index].pixels[linha*8 + i] = tile_pixel::PX_WHITE;
          break;
      }
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

void PPU::step(uint8_t cpu_ciclos, Texture2D& texture){
  this->ciclos+=cpu_ciclos;

  switch(this->modo_atual){
    using enum screen_mode;

    case SOAMRAM:{
      if(this->ciclos >= 80){
        this->ciclos -= 80;
        this->scan_oam();
        this->set_mode(screen_mode::DRAWING);
      }
      break;
    }
    case DRAWING:{
      if(this->ciclos >= 172){
        this->ciclos -= 172;
        this->draw_line();
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
        UpdateTexture(texture, this->framebuffer.data());
        if(this->bus->memoria[0xFF44] == 0x00){
          this->set_mode(screen_mode::SOAMRAM);
        }
      }
      break;
    }
  }
}

}
