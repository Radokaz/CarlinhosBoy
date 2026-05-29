#ifndef LCD_H
#define LCD_H

#include <utility>

//registrador LCDC (0xFF40)
#define LCDC_ENABLE (1 << 7) //se a tela está ligada ou não
#define LCDC_WIN_MAP (1 << 6) //0x9800 ou 0x9C00 para mapear a memória da janela
#define LCDC_WIN_ENABLE (1 << 5)
#define LCDC_TILE_DATA (1 << 4)//0x8800 ou 0x8000
#define LCDC_BG_MAP (1 << 3)//0x9800 ou 0x9C00
#define LCDC_OBJ_SIZE (1 << 2)//8x8 ou 8x16
#define LCDC_OBJ_ENABLE (1 << 1)
#define LCDC_BG_ENABLE (1 << 0)

//registrador STAT (0xFF41)
#define LYC_ENABLE (1 << 6)
#define OAM_ENABLE (1 << 5)
#define VBLANK_ENABLE (1 << 4)
#define HBLANK_ENABLE (1 << 3) 
#define LYC_Comparison_Signal (1 << 2)

namespace GB{

enum class screen_mode: uint8_t{
  HBLANK = 0,
  VBLANK = 1,
  SOAMRAM = 2,
  DRAWING = 3,
};

}

#endif
