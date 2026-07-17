#include "ppu.h"

namespace GB{

uint16_t PPU::pega_cor_bg(uint8_t paleta, uint8_t indice){
  uint8_t byte1 = this->bg_palette_ram[paleta*8 + indice*2];
  uint8_t byte2 = this->bg_palette_ram[paleta*8 + indice*2 + 1];
  uint16_t cor = (byte2 << 8) | byte1;
  return cor;
}

uint16_t PPU::pega_cor_obj(uint8_t paleta, uint8_t indice){
  uint8_t byte1 = this->obj_palette_ram[paleta*8 + indice*2];
  uint8_t byte2 = this->obj_palette_ram[paleta*8 + indice*2 + 1];
  uint16_t cor = (byte2 << 8) | byte1;
  return cor;
}

uint32_t PPU::rgb555_to_rgb888(uint16_t cor){
  uint8_t r5 = cor & 0x1F;
  uint8_t g5 = (cor >> 5) & 0x1F;
  uint8_t b5 = (cor >> 10) & 0x1F;

  uint32_t r = r5*26 + g5*4 + b5*2;
  uint32_t g = g5*24 + b5*8;
  uint32_t b = r5*6 + g5*4 + b5*22;

  r = std::min(960u, r) >> 2;
  g = std::min(960u, g) >> 2;
  b = std::min(960u, b) >> 2;

  uint8_t r8 = static_cast<uint8_t>(r);
  uint8_t g8 = static_cast<uint8_t>(g);
  uint8_t b8 = static_cast<uint8_t>(b);

  return 0xFF000000 | ((b8 << 16) | (g8 << 8) | r8);
}

uint32_t PPU::decide_bg_color_dmg(tile_pixel px){
    uint8_t cor_final{};
    uint8_t bgp {this->memoria[0xFF47]};
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

    if(!this->paleta_cgb){
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
    
    uint16_t cor555 = pega_cor_bg(0, cor_final);
    return this->rgb555_to_rgb888(cor555);
}

uint32_t PPU::decide_obj_color_dmg(const Sprite& sprite, tile_pixel pos){
    using enum tile_pixel;
    uint8_t obj_palette = (sprite.flags & (1 << 4)) ? this->memoria[0xFF49] : this->memoria[0xFF48];
    uint8_t cor_final {};

    switch(pos){
      case INDEX_ZERO: 
        return 0x00; //transparente
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

    if(!this->paleta_cgb){
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

    uint8_t paleta = (sprite.flags & (1 << 4)) ? 1 : 0;
    uint16_t cor555 = pega_cor_obj(paleta, cor_final);
    return this->rgb555_to_rgb888(cor555);
}

uint32_t PPU::decide_obj_color_cgb(const Sprite& sprite, tile_pixel px){
  using enum tile_pixel;
  uint8_t obj_palette = sprite.flags & 0x07;
  uint16_t cor_final {};

  switch(px){
    case INDEX_ZERO:
      return 0x00;
    case INDEX_ONE:
      cor_final = pega_cor_obj(obj_palette, 1);
      break;
    case INDEX_TWO:
      cor_final = pega_cor_obj(obj_palette, 2);
      break;
    case INDEX_THREE:
      cor_final = pega_cor_obj(obj_palette, 3);
      break;
    default:
      return 0x00;
  }

  return rgb555_to_rgb888(cor_final);
}

uint32_t PPU::decide_bg_color_cgb(tile_pixel px, uint8_t atributos){
  using enum tile_pixel;
  uint8_t bg_palette = atributos & 0x07;
  uint16_t cor_final;

  switch(px){
    case INDEX_ZERO:
      cor_final = pega_cor_bg(bg_palette, 0);
      break;
    case INDEX_ONE:
      cor_final = pega_cor_bg(bg_palette, 1);
      break;
    case INDEX_TWO:
      cor_final = pega_cor_bg(bg_palette, 2);
      break;
    case INDEX_THREE:
      cor_final = pega_cor_bg(bg_palette, 3);
      break;
    default:
      return 0xFFFF0000;
  }

  return rgb555_to_rgb888(cor_final);
}

uint32_t PPU::esverdear(uint32_t px){
  uint8_t b = (px >> 16) & 0xFF;
  uint8_t g = (px >> 8)  & 0xFF;
  uint8_t r = (px) & 0xFF;
    
  r = (r*0x9B) / 0xFF;
  g = (g*0xBC) / 0xFF;
  b = (b*0x6A) / 0xFF;
    
  return (0xFF << 24) | (b << 16) | (g << 8) | r;
}

//lê um sprite apenas
void PPU::scan_oam(void){
  if(sprites_count >= 10){
    ++sprites_lidos;
    return;
  }

  uint8_t ly = this->memoria[0xFF44];
  uint8_t sprite_sz = this->atual_spritesize();
  int16_t y = static_cast<int16_t>(this->memoria[OAM_INICIO + sprites_lidos*4]) - 16;

  if(!(ly >= y && ly < y + sprite_sz)){
    ++sprites_lidos;
    return;
  }
  
  this->sprites_sel[sprites_count++] = Sprite{memoria[OAM_INICIO + sprites_lidos*4],
    memoria[OAM_INICIO + sprites_lidos*4 + 1], memoria[OAM_INICIO + sprites_lidos*4 + 2], 
    memoria[OAM_INICIO + sprites_lidos*4 + 3]};

  ++sprites_lidos;
}

void PPU::verifica_penalidade(const Sprite& sprite){
  if(!sprite.x){
    this->fetcher.sprite_penality += 11;
    return;
  }

  int32_t esquerdo = sprite.x - 8;
  int32_t fundo {};
  if(this->fetcher.window_ativa){
    fundo = esquerdo - (static_cast<int>(this->get_winx()) - 7);
  }
  else{
    fundo = esquerdo + this->get_scrollx();
  }

  uint8_t coluna_tile = ((fundo/8) & 0x1F);
  if(this->tiles_lidos[coluna_tile]){
    this->fetcher.sprite_penality += 6;
    return;
  }

  this->tiles_lidos[coluna_tile] = 1;
  uint8_t pixels_direita = 7 - (fundo % 8);
  int8_t penalidade = pixels_direita - 2;
  
  this->fetcher.sprite_penality += (6 + ((penalidade <= 0) ? 0 : penalidade)); 
}

uint32_t PPU::merge_sprites(uint8_t x_atual, tile_pixel bg_cor, uint8_t tile_att){
  uint8_t ly = this->memoria[0xFF44];
  uint8_t sprite_sz = this->atual_spritesize();

  for(size_t i {}; i < sprites_count; ++i){
    const Sprite& sprite = sprites_sel[i];

     if(bg_cor != tile_pixel::INDEX_ZERO){
      if(!modo_cpu){
        if(sprite.flags & (1 << 7)) continue; //prioridade do background para o dmg
      }
      else{
        if(this->is_bg_enabled() && ((sprite.flags & (1 << 7)) || (tile_att & (1 << 7)))) continue; //prioridade de bg para o cgb
      }    
    }

    int32_t tela_x = sprite.x - 8;
    int32_t tela_y = sprite.y - 16;

    if(x_atual < tela_x || x_atual >= sprite.x) continue;
    if(ly < tela_y || ly >= tela_y + sprite_sz) continue;
    
    uint16_t pixel_x = x_atual - tela_x;
    uint16_t pixel_y = ly - tela_y;
    uint8_t pixel_tile = sprite.tile_index;

    if(sprite.flags & (1 << 5)){ //flip x
      pixel_x = 7 - pixel_x;
    }
    if(sprite.flags & (1 << 6)){ //flip y
      pixel_y = sprite_sz - 1 - pixel_y;
    }

    if(sprite_sz == 16){
      pixel_tile &= 0xFE;

      if(pixel_y >= 8){
        pixel_tile++;
        pixel_y-=8;
      }
    }
    
    uint8_t byte1 {}, byte2 {};
    if(!modo_cpu || !(sprite.flags & 0x08)){
      byte1 = this->memoria[VRAM_INICIO + pixel_tile*16 + pixel_y*2];
      byte2 = this->memoria[VRAM_INICIO + pixel_tile*16 + pixel_y*2 + 1];
    }
    else{
      byte1 = this->vram_bank1[pixel_tile*16 + pixel_y*2];
      byte2 = this->vram_bank1[pixel_tile*16 + pixel_y*2 + 1];
    }

    uint8_t bit = 7 - pixel_x;
    
    tile_pixel result = tile_pixel::INDEX_NULO;
    switch((((byte2 >> bit) & 0x01) << 1) | ((byte1 >> bit) & 0x01)){
      using enum tile_pixel;

      case 0x00:
        result = INDEX_ZERO;
        break;
      case 0x01:
        result = INDEX_ONE;
        break;
      case 0x02:
        result = INDEX_TWO;
        break;
      case 0x03:
        result = INDEX_THREE;
        break;
    }

    if(result == tile_pixel::INDEX_ZERO || result == tile_pixel::INDEX_NULO) continue; //transparente

    return (!this->modo_cpu) ? this->decide_obj_color_dmg(sprite, result) : this->decide_obj_color_cgb(sprite, result);
  }

  if(!modo_cpu){
    return (this->is_bg_enabled()) ? this->decide_bg_color_dmg(bg_cor) : this->decide_bg_color_dmg(tile_pixel::INDEX_ZERO);
  } 
  
  return this->decide_bg_color_cgb(bg_cor, tile_att);
}

void PPU::checa_sprites(uint8_t x_atual){
  for(size_t i {}; i < sprites_count; ++i){
    if(sprites_buscados[i]) continue;
    int32_t tela_x = sprites_sel[i].x - 8;
    if(tela_x <= static_cast<int32_t>(x_atual)){
      sprites_buscados[i] = 1;
      this->verifica_penalidade(sprites_sel[i]);
    }
  }
}

void PPU::draw_step(void){
  if(!this->fetcher.window_ativa){
    int32_t wx = static_cast<int>(this->get_winx()) - 7;

    if(this->is_win_enabled() && fetcher.window_trigger && static_cast<int32_t>(this->fetcher.x_pos) >= wx){
      fetcher.window_ativa = true;
      this->fetcher.clear();
      this->tiles_lidos.fill(0);
      fetcher.drop_pixels = 0;
    }
  }

  this->fetcher.step(this);
  if(fetcher.atual == fetcher_estado::FLUSH || fetcher.sprite_penality) return;

  if(!this->fetcher.size) return;
  if(this->fetcher.drop_pixels > 0){
    this->fetcher.pop();
    --fetcher.drop_pixels;
    return;
  }

  auto [px, atributo] = this->fetcher.pop();

  uint32_t cor_px {};
  if(this->speed_bug == 1)
    cor_px = 0xFF000000;
  else if(this->is_sprite_enabled() && this->speed_bug != 2){
    this->checa_sprites(this->fetcher.x_pos);
    cor_px = this->merge_sprites(this->fetcher.x_pos, px, atributo);
  }
  else{
    if(!modo_cpu)
      cor_px = (this->is_bg_enabled()) ? this->decide_bg_color_dmg(px) : this->decide_bg_color_dmg(tile_pixel::INDEX_ZERO);
    else
      cor_px = this->decide_bg_color_cgb(px, atributo);
  }
  
  if(this->paleta_lcd && !this->paleta_cgb)
    cor_px = this->esverdear(cor_px);

  uint8_t ly = this->memoria[0xFF44];

  this->framebuffer[ly*160 + this->fetcher.x_pos] = cor_px;
  ++fetcher.x_pos;
  if(this->fetcher.x_pos >= 160){
    fetcher.atual = fetcher_estado::FLUSH;
    fetcher.delay = 3;
  }
}

}

