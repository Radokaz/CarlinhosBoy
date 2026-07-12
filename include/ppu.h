#ifndef PPU_H
#define PPU_H

#define VRAM_INICIO 0x8000
#define VRAM_FINAL  0xA000

#define WRAM_INICIO 0xC000
#define WRAM_FINAL 0xD000

#define FIRST_TILE1 0x8000
#define SECOND_TILE1 0x8800
#define SECOND_TILE2 0x9000
#define TILE_END 0x9800

#define OAM_INICIO 0xFE00
#define OAM_FIM 0xFEA0

#include <cstddef>
#include <cstdint>
#include <array>
#include <memory>
#include <algorithm>
#include <raylib.h>
#include "lcd.h"

namespace GB{

enum class tile_pixel: uint8_t{
  INDEX_ZERO = 0,
  INDEX_ONE,
  INDEX_TWO,
  INDEX_THREE,
  INDEX_NULO
};

enum class fetcher_estado: uint8_t{
  INICIO,
  READ_ID,
  READ_LOW,
  READ_HIGH,
  PUSH,
  FLUSH
};

enum class oam_corruption{
  READ,
  WRITE,
  READ_WRITE
};

//cada tile possui 64 pixels em que cada pixel usa 2 bits para representar sua cor,
//dando 64*2 = 128 bits = 16 bytes em cada tile

struct Sprite{
  uint8_t y;
  uint8_t x;
  uint8_t tile_index;
  uint8_t flags;
};

struct Pixel{
  tile_pixel px;
  uint8_t atributo;
};

struct PPU;

struct PPU_fetcher{
  std::array<Pixel, 16> fila;
  uint8_t ultimo{};
  uint8_t prim {};
  uint8_t size {};

  uint8_t tile_id {};
  uint8_t tile_low {};
  uint8_t tile_high {};
  uint8_t tile_attribute {};

  uint8_t ciclos {};
  uint8_t x_pos {};
  uint8_t tiles_buscados {};
  uint8_t drop_pixels {};
  uint8_t win_line {};
  fetcher_estado atual {fetcher_estado::READ_ID};
  bool window_ativa {false};
  uint8_t delay {};
  uint8_t sprite_penality {};
  bool finalizado {false};

  PPU_fetcher(){
    fila.fill(Pixel{tile_pixel::INDEX_NULO, 0});
  }

  void push(Pixel alvo);
  Pixel pop(void);
  void clear(void);

  void step(PPU *ppu);
};


struct PPU{
  std::array<uint32_t, 160*144> framebuffer;
  PPU_fetcher fetcher{};
  std::array<Sprite, 10> sprites_sel{};
  std::array<uint8_t, 32> tiles_lidos{};
  std::array<uint8_t, 10> sprites_buscados{};
  std::unique_ptr<uint8_t[]> vram_bank1 {};
  std::unique_ptr<uint8_t[]> bg_palette_ram {};
  std::unique_ptr<uint8_t[]> obj_palette_ram {};
  uint8_t *memoria {};
  Texture2D *raylib_texture;
  bool *hdma_hblank {nullptr};
  bool *hdma_ativo {nullptr};
  uint16_t ciclos {};
  uint16_t draw_ciclos {};
  uint16_t hblank_ciclos {};
  screen_mode modo_atual {screen_mode::SOAMRAM};
  uint8_t sprites_count {};
  uint8_t sprites_lidos {};
  uint8_t modo_cpu {}; //0: DMG only, 1: DMG e CGB, 2: CGB only
  uint8_t speed_bug {}; //0: não bugado, 1: bug em hblank/vblank, 2: bug em soamram
  bool lcd_start {false};
  bool lcd_prev {false};
  bool stat_prev {false};
  bool paleta_lcd {true};
  bool paleta_cgb {false};
  bool frame_pronto {false};

  PPU(Texture2D *texture): raylib_texture{texture}{
    framebuffer.fill(0xFFFFFFFF);
    vram_bank1 = nullptr;
    bg_palette_ram = nullptr;
    obj_palette_ram = nullptr;
  }

  uint8_t& read_vram(uint16_t endereco);
  void write_vram(uint16_t endereco, uint8_t valor, bool hdma);
  void step(void);
  void scan_oam(void);
  void verifica_penalidade(const Sprite& sprite);
  void checa_sprites(uint8_t x_atual);
  uint32_t merge_sprites(uint8_t x_atual, tile_pixel bg_cor, uint8_t tile_att);
  void draw_step(void);

  uint16_t atual_wintilemap(void);
  uint16_t atual_bgtiledata(void);
  uint16_t atual_bgtilemap(void);
  uint8_t atual_spritesize(void);
  bool is_lcd_enabled(void);
  bool is_win_enabled(void);
  bool is_bg_enabled(void);
  bool is_sprite_enabled(void);
  uint8_t& get_scrolly(void);
  uint8_t& get_scrollx(void);
  uint8_t& get_winx(void);
  uint8_t& get_winy(void);
  void set_mode(screen_mode modo);
  bool check_stat(void);
  void check_stat_interruption(void);
  //ly é o registrador que marca a linha sendo scaneada no momento
  void avanca_ly(void);
  void reset_sprites(void);
  void oam_write_corruption(uint8_t scan_row);
  void oam_read_corruption(uint8_t scan_row);
  void oam_readwrite_corruption(uint8_t scan_row);
  void check_oam(uint16_t registrador, oam_corruption corruption);
  uint32_t rgb555_to_rgb888(uint16_t cor);
  uint16_t pega_cor_bg(uint8_t paleta, uint8_t indice);
  uint16_t pega_cor_obj(uint8_t paleta, uint8_t indice);
  uint32_t decide_bg_color_dmg(tile_pixel px);
  uint32_t decide_bg_color_cgb(tile_pixel px, uint8_t atributos);
  uint32_t decide_obj_color_dmg(const Sprite& sprite, tile_pixel pos);
  uint32_t decide_obj_color_cgb(const Sprite& sprite, tile_pixel px);
  uint32_t esverdear(uint32_t px);
};



}

#endif
