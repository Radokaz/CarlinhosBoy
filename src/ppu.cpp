#include "ppu.h"

namespace GB{

uint16_t PPU::atual_wintilemap(void){
    return (memoria[0xFF40] & LCDC_WIN_MAP) ? 0x9C00 : 0x9800;
}

uint16_t PPU::atual_bgtiledata(void){
    return (memoria[0xFF40] & LCDC_TILE_DATA) ? 0x8000 : 0x8800;
}

uint16_t PPU::atual_bgtilemap(void){
    return (memoria[0xFF40] & LCDC_BG_MAP) ? 0x9C00 : 0x9800;
}

uint8_t PPU::atual_spritesize(void){
    return (memoria[0xFF40] & LCDC_OBJ_SIZE) ? 16 : 8;
}

bool PPU::is_lcd_enabled(void){
  return static_cast<bool>((this->memoria[0xFF40] & LCDC_ENABLE) != 0);
}

bool PPU::is_win_enabled(void){
  return static_cast<bool>((this->memoria[0xFF40] & LCDC_WIN_ENABLE) != 0);
}

bool PPU::is_bg_enabled(void){
  return static_cast<bool>((this->memoria[0xFF40] & LCDC_BG_ENABLE) != 0);
}

bool PPU::is_sprite_enabled(void){
  return static_cast<bool>((this->memoria[0xFF40] & LCDC_OBJ_ENABLE) != 0);
}

uint8_t& PPU::get_scrolly(void) { return memoria[0xFF42]; }
uint8_t& PPU::get_scrollx(void) { return memoria[0xFF43]; }
uint8_t& PPU::get_winy(void) { return memoria[0xFF4A]; }
uint8_t& PPU::get_winx(void) {return memoria[0xFF4B]; }

void PPU::set_mode(screen_mode modo){
    uint8_t& stat = memoria[0xFF41];
    stat = (stat & 0b11111100) | std::to_underlying<screen_mode>(modo);
    this->modo_atual = modo;
    this->check_stat_interruption();
}

void PPU::lyc_compare(void){
  uint8_t ly = memoria[0xFF44];
  uint8_t lyc = memoria[0xFF45];
  uint8_t& stat = memoria[0xFF41];
  if(ly == lyc)
    stat |= LYC_Comparison_Signal;
  else
    stat &= ~LYC_Comparison_Signal;
}

bool PPU::check_stat(void){
    if(!this->is_lcd_enabled())
      return false;

    uint8_t stat = memoria[0xFF41];
    this->lyc_compare(); //hack pra compensar a imprecisão nos ciclos

    return (((stat & LYC_Comparison_Signal) && (stat & LYC_ENABLE)) ||
    ((this->modo_atual == screen_mode::HBLANK) && (stat & HBLANK_ENABLE )) ||
    ((this->modo_atual == screen_mode::SOAMRAM) && (stat & OAM_ENABLE)) ||
    ((this->modo_atual == screen_mode::VBLANK) && ((stat & VBLANK_ENABLE) || (stat & OAM_ENABLE))));
}

void PPU::check_stat_interruption(void){
    bool stat_atual = this->check_stat();
    if(stat_atual && !stat_prev)
      memoria[0xFF0F] |= BIT_LCDSTAT;

    stat_prev = stat_atual;
}

void PPU::avanca_ly(void){
    uint8_t& ly = this->memoria[0xFF44];
    ly = (ly + 1) % 154;
    this->check_stat_interruption();
}

uint8_t& PPU::read_vram(uint16_t endereco){
  if(vram_bank1){
    uint8_t bank = memoria[0xFF4F] & 0x01;
    if(bank)
      return vram_bank1[endereco - VRAM_INICIO];
  }
  
  return memoria[endereco];
}

void PPU::write_vram(uint16_t endereco, uint8_t valor){
  if(this->modo_atual == screen_mode::DRAWING) return;
  if(vram_bank1){
    uint8_t bank = memoria[0xFF4F] & 0x01;
    if(bank){
      vram_bank1[endereco - VRAM_INICIO] = valor;
      return;
    }
  }

  memoria[endereco] = valor;
}

void PPU::reset_sprites(void){
  for(size_t i {}; i < sprites_buscados.size(); ++i){
    sprites_buscados[i] = 0;
  }
  for(size_t i; i < tiles_lidos.size(); ++i){
    tiles_lidos[i] = 0;
  }

  this->sprites_lidos = 0;
}

void PPU::soamram_step(void){
  if(this->ciclos % 2) return;

  this->scan_oam();
  if(sprites_lidos >= 40){
    if(this->modo_cpu == 0 || (this->memoria[0xFF6C] & 0x01)){
      std::stable_sort(this->sprites_sel.begin(), this->sprites_sel.begin() + this->sprites_count, 
          [](const Sprite& a, const Sprite& b) -> bool{ return (a.x < b.x); });
    }
    ciclos = 0;
    this->reset_sprites();
    this->fetcher.clear();
    fetcher.x_pos = 0;
    draw_ciclos = 0;
    fetcher.drop_pixels = this->get_scrollx() % 8;
    fetcher.window_ativa = false;

    uint8_t wy = this->get_winy();
    if(memoria[0xFF44] == wy)
      fetcher.window_trigger = true;

    if(fetcher.gbc_window_desativada && memoria[0xFF44] >= wy){
      fetcher.gbc_window_desativada = false;
      fetcher.window_trigger = true;
    }

    this->set_mode(screen_mode::DRAWING);
  }
}

void PPU::mode3_step(void){
  this->draw_step();
  ++draw_ciclos;
  if(this->fetcher.finalizado){
    ciclos = 0;
    hblank_ciclos = 456 - this->draw_ciclos - 80;
    if(lcd_start){
      hblank_ciclos -= 4;
      lcd_start = false;
    }
    if(fetcher.window_ativa)
      ++fetcher.win_line;

    if(modo_cpu > 0 && *hdma_hblank){
      *hdma_ativo = true;
    }

    this->set_mode(screen_mode::HBLANK);
  }
}

void PPU::hblank_step(void){
  if(this->ciclos < this->hblank_ciclos) return;

  ciclos = 0;
  hblank_ciclos = 0;
  this->avanca_ly();
  if(memoria[0xFF44] == 144){
    this->memoria[0xFF0F] |= BIT_VBLANK;
    this->set_mode(screen_mode::VBLANK);
    UpdateTexture(*(this->raylib_texture), this->framebuffer.data());
  }
  else{
    sprites_count = 0;
    sprites_lidos = 0;
    this->set_mode(screen_mode::SOAMRAM);
  }
}

void PPU::vblank_step(void){
  if(memoria[0xFF44] == 153 && this->ciclos == 8){
    this->avanca_ly();
  }
  if(this->ciclos >= 456){
    ciclos = 0;
    if(memoria[0xFF44] != 0){
      this->avanca_ly();
    }
    else{
      fetcher.win_line = 0;
      fetcher.window_trigger = false;
      fetcher.gbc_window_desativada = false;
      this->frame_pronto = true;
      sprites_count = 0;
      sprites_lidos = 0;
      this->set_mode(screen_mode::SOAMRAM);
    }
  }
}

void PPU::step(void){
  if(!this->is_lcd_enabled()){
    if(lcd_prev){
      framebuffer.fill(((!paleta_cgb && paleta_lcd) ? 0xFF6ABC9B : 0xFFFFFFFF));
      UpdateTexture(*(this->raylib_texture), this->framebuffer.data());
      frame_pronto = true;
    }
    lcd_prev = false;
    modo_atual = screen_mode::HBLANK;
    ciclos = 0;
    memoria[0xFF44] = 0;
    uint8_t& stat = memoria[0xFF41];
    stat = (stat & 0b11111100);
    return;
  }
  if(!this->lcd_prev){
    lcd_prev = true;
    ciclos = 0;
    memoria[0xFF44] = 0;
    sprites_lidos = 0;
    sprites_count = 0;
    draw_ciclos = 0;
    hblank_ciclos = 0;
    lcd_start = true;
    fetcher.window_trigger = false;
    fetcher.gbc_window_desativada = false;
    this->set_mode(screen_mode::SOAMRAM);
  }
  
  this->lyc_compare();
  size_t limiar {4};
  if(modo_cpu > 0 && (memoria[0xFF4D] & 0x80))
    limiar = 2;

  for(size_t i {}; i < limiar; ++i){
    ++this->ciclos;
        
    switch(this->modo_atual){
      using enum screen_mode;

      case SOAMRAM:{
        this->soamram_step();
        break;
      }
      case DRAWING:{
        this->mode3_step();
        break;
      }
      case HBLANK:{
        this->hblank_step();
        break;
      }
      case VBLANK:{
        this->vblank_step();
        break;
      }
    }
  }
}

}
