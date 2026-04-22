#ifndef LCD_H
#define LCD_H

#include "cpu.h"

#define LCDC_ENABLE (1 << 7)
#define LCDC_WIN_MAP (1 << 6)
#define LCDC_WIN_ENABLE (1 << 5)
#define LCDC_TILE_DATA (1 << 4)
#define LCDC_BG_MAP (1 << 3)
#define LCDC_OBJ_SIZE (1 << 2)
#define LCDC_OBJ_ENABLE (1 << 1)
#define LCDC_BG_ENABLE (1 << 0)

inline void set_lcd(CPU *cpu, bool ligado){
  if(ligado)
    cpu->bus.read_byte(0xFF40) |= LCDC_ENABLE;
  else
    cpu->bus.read_byte(0xFF40) &= ~LCDC_ENABLE;
}

inline uint16_t atual_tilemap(CPU *cpu){
  return (cpu->bus.read_byte(0xFF40) & LCDC_WIN_MAP) ? 0x9C00 : 0x9800;
}

inline void troca_tilemap(CPU *cpu){
  cpu->bus.read_byte(0xFF40) ^= LCDC_WIN_MAP;
}

inline void set_window(CPU *cpu, bool ligado){
  if(ligado)
    cpu->bus.read_byte(0xFF40) |= LCDC_WIN_ENABLE;
  else
    cpu->bus.read_byte(0xFF40) &= ~LCDC_WIN_ENABLE;
}

inline uint16_t atual_gbtiledata(CPU *cpu){
  return (cpu->bus.read_byte(0xFF40) & LCDC_TILE_DATA) ? 0x8000 : 0x8800;
}

inline void troca_tiledata(CPU *cpu){
  cpu->bus.read_byte(0xFF40) ^= LCDC_TILE_DATA;
}

inline uint16_t atual_bgtilemap(CPU *cpu){
  return (cpu->bus.read_byte(0xFF40) & LCDC_BG_MAP) ? 0x9C00 : 0x9800;
}

inline void troca_bgtilemap(CPU *cpu){
  cpu->bus.read_byte(0xFF40) ^= LCDC_BG_MAP;
}

inline uint16_t atual_spritesize(CPU *cpu){
  return (cpu->bus.read_byte(0xFF40) & LCDC_OBJ_SIZE) ? 16 : 8;
}

inline void troca_spritesize(CPU *cpu){
  cpu->bus.read_byte(0xFF40) ^= LCDC_OBJ_SIZE;
}

inline void set_sprites(CPU *cpu, bool ligado){
  if(ligado)
    cpu->bus.read_byte(0xFF40) |= LCDC_OBJ_ENABLE;
  else
    cpu->bus.read_byte(0xFF40) &= ~LCDC_OBJ_ENABLE;
}

inline void set_bg(CPU *cpu, bool ligado){
  if(ligado)
    cpu->bus.read_byte(0xFF40) |= LCDC_BG_ENABLE;
  else
    cpu->bus.read_byte(0xFF40) &= ~LCDC_BG_ENABLE;
}


#endif
