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
  atual = fetcher_estado::READ_ID;
  ciclos = 0;
  x_pos = 0;
  tiles_buscados = 0;
  size = 0;
  ultimo = 0;
  prim = 0;
  finalizado = false;
  flush_cycles = 0;
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

bool PPU::is_lcd_enabled(void){
  return static_cast<bool>((this->bus->read_byte(0xFF40) & LCDC_ENABLE) != 0);
}

bool PPU::is_win_enabled(void){
  return static_cast<bool>((this->bus->read_byte(0xFF40) & LCDC_WIN_ENABLE) != 0);
}

bool PPU::is_bg_enabled(void){
  return static_cast<bool>((this->bus->read_byte(0xFF40) & LCDC_BG_ENABLE) != 0);
}

bool PPU::is_sprite_enabled(void){
  return static_cast<bool>((this->bus->read_byte(0xFF40) & LCDC_OBJ_ENABLE) != 0);
}

uint8_t& PPU::get_scrolly(void) { return bus->memoria[0xFF42]; }
uint8_t& PPU::get_scrollx(void) { return bus->memoria[0xFF43]; }
uint8_t& PPU::get_winy(void) { return bus->memoria[0xFF4A]; }
uint8_t& PPU::get_winx(void) {return bus->memoria[0xFF4B]; }

void PPU::set_mode(screen_mode modo){
    uint8_t& stat = bus->memoria[0xFF41];
    stat = (stat & 0b11111100) | std::to_underlying<screen_mode>(modo);
    this->modo_atual = modo;
    this->check_stat_interruption();
}

bool PPU::check_stat(void){
    if(!this->is_lcd_enabled())
      return false;

    uint8_t ly = bus->memoria[0xFF44];
    uint8_t lyc = bus->memoria[0xFF45];
    uint8_t& stat = bus->memoria[0xFF41];
  
    return (((ly == lyc) && (stat & LYC_ENABLE)) ||
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
}

void PPU::step(void){
  if(!this->is_lcd_enabled()){
    this->modo_atual = screen_mode::HBLANK;
    this->ciclos = 0;
    this->bus->memoria[0xFF44] = 0;
    uint8_t& stat = bus->memoria[0xFF41];
    stat = (stat & 0b11111100);
    return;
  }

  for(size_t i {}; i < 4; ++i){
    ++this->ciclos;

    switch(this->modo_atual){
      using enum screen_mode;

      case SOAMRAM:{
        if(this->ciclos >= 80){
          this->ciclos = 0;
          this->scan_oam();
          this->fetcher.clear();
          this->draw_ciclos = 0;
          this->fetcher.drop_pixels = this->get_scrollx() % 8;
          this->fetcher.window_ativa = false;
          this->set_mode(screen_mode::DRAWING);
        }
        break;
      }
      case DRAWING:{
        if(!this->fetcher.finalizado){
          this->draw_step();
          ++this->draw_ciclos;
        }
        else{
          this->ciclos = 0;
          this->hblank_ciclos = 456 - this->draw_ciclos - 80;
          if(this->fetcher.window_ativa)
            ++this->fetcher.win_line;
          this->set_mode(screen_mode::HBLANK);
        }
        break;
      }
      case HBLANK:{
        if(this->ciclos >= this->hblank_ciclos){
          this->ciclos = 0;
          this->hblank_ciclos = 0;
          this->avanca_ly();
          if(this->bus->memoria[0xFF44] == 144){
            this->bus->memoria[0xFF0F] |= BIT_VBLANK;
            this->set_mode(screen_mode::VBLANK);
            UpdateTexture(*(this->raylib_texture), this->framebuffer.data());
          }
          else{
            this->set_mode(screen_mode::SOAMRAM);
          }
        }
        break;
      }
      case VBLANK:{
        if(this->ciclos >= 456){
          this->ciclos = 0;
          this->avanca_ly();
          if(this->bus->memoria[0xFF44] == 0x00){
            this->fetcher.win_line = 0;
            this->frame_pronto = true;
            this->set_mode(screen_mode::SOAMRAM);
          }
        }
        break;
      }
    }
  }
}

void PPU_fetcher::step(PPU *ppu){
  ++this->ciclos;

  if(this->ciclos < 2) return;
  this->ciclos = 0;

  switch(this->atual){
    using enum fetcher_estado;

    case READ_ID:{
      uint8_t ly = ppu->bus->memoria[0xFF44];
      if(!this->window_ativa){
        uint8_t scx = ppu->get_scrollx();
        uint8_t scy = ppu->get_scrolly();
        uint16_t tilemap = ppu->atual_bgtilemap();

        uint8_t tilex = ((scx/8) + this->tiles_buscados) & 31;
        uint8_t tiley = (((scy + ly) % 256)/8) & 31;

        this->tile_id = ppu->bus->memoria[tilemap + tiley*32 + tilex];
      }
      else{
        uint8_t winy = this->win_line;
        uint16_t tilemap = ppu->atual_wintilemap();

        uint8_t tilex = this->tiles_buscados & 31;
        uint8_t tiley = (((winy + ly) % 256)/8) & 31;

        this->tile_id = ppu->bus->memoria[tilemap + tiley*32 + tilex];
      }
      this->atual = fetcher_estado::READ_LOW;
      break;
    }
    case READ_LOW:{
      uint8_t ly = ppu->bus->memoria[0xFF44];

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
      
      this->tile_low = ppu->bus->memoria[tile_addr + linha*2];
      this->atual = fetcher_estado::READ_HIGH;
      break;
    }
    case READ_HIGH:{
      uint8_t ly = ppu->bus->memoria[0xFF44];

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

      this->tile_high = ppu->bus->memoria[tile_addr + linha*2 + 1];
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
      ++this->flush_cycles;
      if(this->flush_cycles >= 3){
        this->flush_cycles = 0;
        this->finalizado = true;
      }
    }
  }
}

}
