#ifndef LCD_H
#define LCD_H

#include "cpu.h"

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

enum class screen_mode: uint8_t{
  HBLANK = 0,
  VBLANK,
  SOAMRAM,
  DRAWING
};

inline void set_lcd(CPU *cpu, bool ligado){
  if(ligado)
    cpu->bus.read_byte(0xFF40) |= LCDC_ENABLE;
  else
    cpu->bus.read_byte(0xFF40) &= ~LCDC_ENABLE;
}

inline uint16_t atual_tilemap(CPU *cpu){
  return (cpu->bus.read_byte(0xFF40) & LCDC_WIN_MAP) ? 0x9C00 : 0x9800;
}

inline void set_window(CPU *cpu, bool ligado){
  if(ligado)
    cpu->bus.read_byte(0xFF40) |= LCDC_WIN_ENABLE;
  else
    cpu->bus.read_byte(0xFF40) &= ~LCDC_WIN_ENABLE;
}

inline uint16_t atual_bgtiledata(CPU *cpu){
  return (cpu->bus.read_byte(0xFF40) & LCDC_TILE_DATA) ? 0x8000 : 0x8800;
}

inline uint16_t atual_bgtilemap(CPU *cpu){
  return (cpu->bus.read_byte(0xFF40) & LCDC_BG_MAP) ? 0x9C00 : 0x9800;
}

inline uint16_t atual_spritesize(CPU *cpu){
  return (cpu->bus.read_byte(0xFF40) & LCDC_OBJ_SIZE) ? 16 : 8;
}

inline void set_screen(CPU *cpu, screen_mode modo){
  uint8_t& stat = cpu->bus.read_byte(0xFF41);
  stat = (stat & 0b11111100) | std::to_underlying<screen_mode>(modo);
}

inline screen_mode get_mode(CPU *cpu){
    return static_cast<screen_mode>(cpu->bus.read_byte(0xFF41) & 0x03);
}

inline bool check_stat(CPU *cpu){
  if((cpu->bus.read_byte(0xFF40) & LCDC_ENABLE) != LCDC_ENABLE)
    return false;

  uint8_t ly = cpu->bus.read_byte(0xFF44);
  uint8_t lyc = cpu->bus.read_byte(0xFF45);
  uint8_t& stat = cpu->bus.read_byte(0xFF41);
  screen_mode atual = get_mode(cpu);
  
  return (((ly == lyc) && (stat & LYC_Comparison_Signal)) ||
  ((atual == screen_mode::HBLANK) && (stat & HBLANK_ENABLE )) ||
  ((atual == screen_mode::SOAMRAM) && (stat & OAM_ENABLE)) ||
  ((atual == screen_mode::VBLANK) && ((stat & VBLANK_ENABLE) || (stat & OAM_ENABLE))));
}

inline void check_stat_interruption(CPU *cpu, bool& stat_prev){
  bool stat_atual = check_stat(cpu);
  if(stat_atual && !stat_prev)
    cpu->get_if() |= BIT_LCDSTAT;

  stat_prev = stat_atual;
}

inline void aumenta_ly(CPU *cpu){
  uint8_t ly = cpu->bus.read_byte(0xFF44);
  ++ly;
  if(ly > 153)
    ly = 0;

}


#endif
