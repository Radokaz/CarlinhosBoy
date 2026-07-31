#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "init.h"
#define MAX_SAVE_SLOTS 10
#define PAGE_SIZE 4096

namespace GB{

struct Game_State{
  Joypad pad;
  Timer timer;
  PPU ppu;
  CPU cpu;
  APU apu;
  std::function<void()> restaura_rom;
  std::filesystem::path save_path;
  size_t save_states[MAX_SAVE_SLOTS];
  bool *save_liberado {};

  Game_State(Texture2D *texture, GB_State *estado, std::string_view states, std::string_view rom_path): 
    pad{reinterpret_cast<const void*>(estado->controles.data())}, timer{}, ppu{texture}, cpu{&timer, &pad, &ppu}, apu{cpu.bus.memoria.data()} {
      timer.apu = &apu;
      cpu.bus.restaura_rom = &restaura_rom;
      save_liberado = &cpu.bus.sv_state_liberado;
      
      save_path = std::filesystem::path(states) / std::filesystem::path(rom_path).filename();
      save_path.replace_extension(".db");

      std::fstream saves(save_path, saves.in | saves.binary);
      if(!saves){
        for(size_t i {}; i < MAX_SAVE_SLOTS; ++i){
          save_states[i] = 0;
        }

        return;
      }
      
      for(size_t i {}; i < MAX_SAVE_SLOTS; ++i){
        size_t offset {};
        saves.read(reinterpret_cast<char*>(&offset), sizeof(size_t));
        save_states[i] = offset;
      }
      saves.read(reinterpret_cast<char*>(&estado->save_slot), sizeof(size_t));
  }
  
  void save_state(size_t slot){
    if(!*save_liberado){
      std::cout << "Não é possível salvar durante o bootrom.\n";
      return;
    }

    std::fstream save(save_path, save.in | save.out | save.binary);
    if(!save){
      save.close();
      std::ofstream novo(save_path, save.out | save.binary);
      
      for(size_t i {}; i < MAX_SAVE_SLOTS; ++i){
        if(i + 1 == slot){
          save_states[i] = PAGE_SIZE;
        }
        novo.write(reinterpret_cast<char*>(&save_states[i]), sizeof(size_t));
      }

      novo.write(reinterpret_cast<char*>(&slot), sizeof(size_t));
      novo.close();
      save.clear();
      save.open(save_path, save.in | save.out | save.binary);
    }
    
    if(!save_states[slot - 1]){
      size_t save_index {};
      for(size_t i {}; i < MAX_SAVE_SLOTS; ++i){
        if(save_states[i])
          ++save_index;
      }

      save_states[slot - 1] = PAGE_SIZE + save_index*PAGE_SIZE*80;
      save.seekg((slot - 1)*sizeof(size_t), std::ios::beg);
      save.write(reinterpret_cast<char*>(&save_states[slot - 1]), sizeof(size_t));
    }

    save.seekg(save_states[slot - 1], std::ios::beg);

    std::unique_ptr<uint8_t[]> vbank = std::move(ppu.vram_bank1);
    std::unique_ptr<uint8_t[]> bg_palette = std::move(ppu.bg_palette_ram);
    std::unique_ptr<uint8_t[]> obj_palette = std::move(ppu.obj_palette_ram);
    std::unique_ptr<uint8_t[]> wram = std::move(cpu.bus.cgb_wram);
    std::unique_ptr<MBC> mbc = std::move(cpu.bus.mbc);

    save.write(reinterpret_cast<char*>(&pad), sizeof(Joypad));
    save.write(reinterpret_cast<char*>(&timer), sizeof(Timer));
    save.write(reinterpret_cast<char*>(&ppu), sizeof(PPU));
    save.write(reinterpret_cast<char*>(&cpu), sizeof(CPU));
    save.write(reinterpret_cast<char*>(&apu), sizeof(APU));

    if(vbank)
      save.write(reinterpret_cast<char*>(vbank.get()), 8*1024);
    if(bg_palette)
      save.write(reinterpret_cast<char*>(bg_palette.get()), 64);
    if(obj_palette)
      save.write(reinterpret_cast<char*>(obj_palette.get()), 64);
    if(wram)
      save.write(reinterpret_cast<char*>(wram.get()), 32*1024);

    if(mbc){
      mbc->save_state(&save);
    }

    cpu.bus.mbc = std::move(mbc);
    ppu.vram_bank1 = std::move(vbank);
    ppu.bg_palette_ram = std::move(bg_palette);
    ppu.obj_palette_ram = std::move(obj_palette);
    cpu.bus.cgb_wram = std::move(wram);

    std::cout << "Jogo salvo no slot " << slot << ".\n";
  }

  void load_state(size_t slot){
    if(!save_states[slot - 1]){
      std::cout << "Nenhum save encontrado no slot selecionado.\n";
      return;
    }
    if(!*save_liberado){
      std::cout << "Não é possível dar load durante o bootrom.\n";
      return;
    }

    std::fstream save(save_path, save.in | save.out | save.binary);
    save.seekg(save_states[slot - 1], std::ios::beg);

    std::unique_ptr<uint8_t[]> vbank = std::move(ppu.vram_bank1);
    std::unique_ptr<uint8_t[]> bg_palette = std::move(ppu.bg_palette_ram);
    std::unique_ptr<uint8_t[]> obj_palette = std::move(ppu.obj_palette_ram);
    std::unique_ptr<uint8_t[]> wram = std::move(cpu.bus.cgb_wram);
    std::unique_ptr<MBC> mbc = std::move(cpu.bus.mbc);
    const void *tec = pad.teclas;
    Texture2D *frame = ppu.raylib_texture;

    save.read(reinterpret_cast<char*>(&pad), sizeof(Joypad));
    save.read(reinterpret_cast<char*>(&timer), sizeof(Timer));
    save.read(reinterpret_cast<char*>(&ppu), sizeof(PPU));
    save.read(reinterpret_cast<char*>(&cpu), sizeof(CPU));
    save.read(reinterpret_cast<char*>(&apu), sizeof(APU));

    if(vbank)
      save.read(reinterpret_cast<char*>(vbank.get()), 8*1024);
    if(bg_palette)
      save.read(reinterpret_cast<char*>(bg_palette.get()), 64);
    if(obj_palette)
      save.read(reinterpret_cast<char*>(obj_palette.get()), 64);
    if(wram)
      save.read(reinterpret_cast<char*>(wram.get()), 32*1024);

    if(mbc){
      mbc->load_state(&save);
    }

    cpu.bus.mbc = std::move(mbc);
    ppu.vram_bank1 = std::move(vbank);
    ppu.bg_palette_ram = std::move(bg_palette);
    ppu.obj_palette_ram = std::move(obj_palette);
    cpu.bus.cgb_wram = std::move(wram);
    cpu.bus.restaura_rom = &restaura_rom;
    cpu.bus.timer = &timer;
    timer.apu = &apu;
    cpu.bus.pad = &pad;
    cpu.bus.ppu = &ppu;
    cpu.bus.dma.mbc = cpu.bus.mbc.get();
    cpu.bus.dma.wram = cpu.bus.cgb_wram.get();
    apu.memoria = cpu.bus.memoria.data();
    apu.ch1.memoria = cpu.bus.memoria.data();
    apu.ch2.memoria = cpu.bus.memoria.data();
    apu.ch3.memoria = cpu.bus.memoria.data();
    apu.ch4.memoria = cpu.bus.memoria.data();
    pad.teclas = tec;
    pad.p1 = &cpu.bus.memoria[0xFF00];
    ppu.memoria = cpu.bus.memoria.data();
    ppu.raylib_texture = frame;
    ppu.hdma_hblank = &cpu.bus.hdma.modo_hblank;
    ppu.hdma_ativo = &cpu.bus.hdma.ativo;
    limpa_samples(&apu);

    std::cout << "Save carregado no slot " << slot << ".\n";
  }

  ~Game_State(){
    limpa_samples(&apu);
  }
 
};

void le_input(Game_State *estado, size_t save_slot, bool& pausado, bool& is_120, bool& janela_alterada);
void display_saves(Game_State *game, GB_State *estado);
bool pausa_jogo(Game_State *game, GB_State *estado, bool& pausado, bool& resumido);

}

#endif
