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
  std::filesystem::path image_path;
  size_t save_states[MAX_SAVE_SLOTS];
  size_t frame_states[MAX_SAVE_SLOTS];
  bool *save_liberado {};

  Game_State(Texture2D *texture, GB_State *estado, std::string_view states, std::string_view rom_path): 
    pad{reinterpret_cast<const void*>(estado->controles.data()), reinterpret_cast<const void*>(estado->controles_but.data())}, 
    timer{}, ppu{texture}, cpu{&timer, &pad, &ppu}, apu{cpu.bus.memoria.data()} {
      timer.apu = &apu;
      cpu.bus.restaura_rom = &restaura_rom;
      save_liberado = &cpu.bus.sv_state_liberado;
      
      save_path = std::filesystem::path(states) / std::filesystem::path(rom_path).filename();
      save_path.replace_extension(".db");

      image_path = std::filesystem::path(states) / std::filesystem::path("Frames");
      std::filesystem::create_directories(image_path);
      image_path /= std::filesystem::path(rom_path).filename();
      image_path.replace_extension(".bin");

      std::fstream saves(save_path, saves.in | saves.binary);
      if(!saves){
        for(size_t i {}; i < MAX_SAVE_SLOTS; ++i){
          save_states[i] = 0;
          frame_states[i] = 0;
        }

        return;
      }
      
      for(size_t i {}; i < MAX_SAVE_SLOTS; ++i){
        size_t offset {};
        saves.read(reinterpret_cast<char*>(&offset), sizeof(size_t));
        save_states[i] = offset;
      }
      saves.read(reinterpret_cast<char*>(&estado->save_slot), sizeof(size_t));

      saves.close();
      saves.clear();
      saves.open(image_path, saves.in | saves.binary);
      if(!saves){
        for(size_t i {}; i < MAX_SAVE_SLOTS; ++i){
          frame_states[i] = 0;
        }

        return;
      }

      for(size_t i {}; i < MAX_SAVE_SLOTS; ++i){
        size_t offset {};
        saves.read(reinterpret_cast<char*>(&offset), sizeof(size_t));
        frame_states[i] = offset;
      }
  }

  void save_framebuffer(size_t slot){
    Image frame = LoadImageFromTexture(*(ppu.raylib_texture));

    std::fstream imagens(image_path, imagens.in | imagens.out | imagens.binary);
    if(!imagens){
      imagens.close();

      std::ofstream novo(image_path, novo.out | novo.binary);

      for(size_t i {}; i < MAX_SAVE_SLOTS; ++i){
        if(i + 1 == slot){
          frame_states[i] = PAGE_SIZE;
        }
        
        novo.write(reinterpret_cast<char*>(&frame_states[i]), sizeof(size_t));
      }

      novo.close();
      imagens.clear();
      imagens.open(image_path, imagens.in | imagens.out | imagens.binary);
    }

    constexpr size_t tamanho = sizeof(uint32_t)*160*144;

    if(!frame_states[slot - 1]){
      size_t excluidos {};
      imagens.seekg(10*sizeof(size_t), std::ios::beg);
      imagens.read(reinterpret_cast<char*>(&excluidos), sizeof(size_t));

      if(excluidos){
        size_t offset {};
        imagens.seekg((11 + (excluidos - 1))*sizeof(size_t), std::ios::beg);
        imagens.read(reinterpret_cast<char*>(&offset), sizeof(size_t));
        frame_states[slot - 1] = offset;
        --excluidos;
        imagens.seekp((slot - 1)*sizeof(size_t), std::ios::beg);
        imagens.write(reinterpret_cast<char*>(&frame_states[slot - 1]), sizeof(size_t));
        imagens.seekp(10*sizeof(size_t), std::ios::beg);
        imagens.write(reinterpret_cast<char*>(&excluidos), sizeof(size_t));
      }
      else{
        size_t num_frames {};
        for(size_t i {}; i < MAX_SAVE_SLOTS; ++i){
          if(frame_states[i])
            ++num_frames;
        }

        frame_states[slot - 1] = PAGE_SIZE + num_frames*(sizeof(frame) + tamanho);
        imagens.seekp((slot - 1)*sizeof(size_t), std::ios::beg);
        imagens.write(reinterpret_cast<char*>(&frame_states[slot - 1]), sizeof(size_t));
      }
    }

    imagens.seekp(frame_states[slot - 1], std::ios::beg);
    imagens.write(reinterpret_cast<char*>(&frame), sizeof(frame));
    imagens.write(reinterpret_cast<char*>(frame.data), tamanho);

    UnloadImage(frame);
  }

  std::unique_ptr<Texture2D> load_image(size_t slot){
    if(!frame_states[slot - 1]) return nullptr;

    std::fstream imagens(image_path, imagens.in | imagens.binary);
    if(!imagens)
      return nullptr;

    Image imagem{};
    constexpr size_t tamanho = sizeof(uint32_t)*160*144;

    imagens.seekg(frame_states[slot - 1], std::ios::beg);
    imagens.read(reinterpret_cast<char*>(&imagem), sizeof(imagem));

    imagem.data = reinterpret_cast<void*>(new uint32_t[160*144]);
    imagens.read(reinterpret_cast<char*>(imagem.data), tamanho);
    auto frame = std::make_unique<Texture2D>(LoadTextureFromImage(imagem));

    uint32_t *aux = reinterpret_cast<uint32_t*>(imagem.data);
    imagem.data = nullptr;
    delete[] aux;
    UnloadImage(imagem);

    return frame;
  }

  std::unique_ptr<std::unique_ptr<Texture2D>[]> load_framebuffer(void){
    auto frames = std::make_unique<std::unique_ptr<Texture2D>[]>(MAX_SAVE_SLOTS);

    std::fstream imagens(image_path, imagens.in | imagens.binary);
    if(!imagens){
      return frames;
    }

    constexpr size_t tamanho = sizeof(uint32_t)*160*144;
    for(size_t i {}; i < MAX_SAVE_SLOTS; ++i){
      Image imagem{};

      if(!frame_states[i]){
        UnloadImage(imagem);
        continue;
      }

      imagens.seekg(frame_states[i], std::ios::beg);
      imagens.read(reinterpret_cast<char*>(&imagem), sizeof(imagem));

      imagem.data = reinterpret_cast<void*>(new uint32_t[160*144]);
      imagens.read(reinterpret_cast<char*>(imagem.data), tamanho);
      frames[i] = std::make_unique<Texture2D>(LoadTextureFromImage(imagem));

      uint32_t *aux = reinterpret_cast<uint32_t*>(imagem.data);
      imagem.data = nullptr;
      delete[] aux;
      UnloadImage(imagem);
    }

    return frames;
  }

  size_t get_excluidos(std::fstream *save){
    save->seekg(11*sizeof(size_t), std::ios::beg);
    size_t valor {};
    save->read(reinterpret_cast<char*>(&valor), sizeof(size_t));
    return valor;
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
      size_t deletados = this->get_excluidos(&save);
      if(deletados){
        size_t offset {};
        save.seekg((12 + (deletados - 1))*sizeof(size_t), std::ios::beg);
        save.read(reinterpret_cast<char*>(&offset), sizeof(size_t));
        save_states[slot - 1] = offset;
        --deletados;
        save.seekp((slot - 1)*sizeof(size_t), std::ios::beg);
        save.write(reinterpret_cast<char*>(&save_states[slot - 1]), sizeof(size_t));
        save.seekp(11*sizeof(size_t), std::ios::beg);
        save.write(reinterpret_cast<char*>(&deletados), sizeof(size_t));
      }
      else{
        size_t save_index {};
        for(size_t i {}; i < MAX_SAVE_SLOTS; ++i){
          if(save_states[i])
            ++save_index;
          }

        save_states[slot - 1] = PAGE_SIZE + save_index*PAGE_SIZE*80;
        save.seekp((slot - 1)*sizeof(size_t), std::ios::beg);
        save.write(reinterpret_cast<char*>(&save_states[slot - 1]), sizeof(size_t));
      }
    }

    save.seekp(save_states[slot - 1], std::ios::beg);

    this->save_framebuffer(slot);
    std::unique_ptr<uint8_t[]> vbank = std::move(ppu.vram_bank1);
    std::unique_ptr<uint8_t[]> bg_palette = std::move(ppu.bg_palette_ram);
    std::unique_ptr<uint8_t[]> obj_palette = std::move(ppu.obj_palette_ram);
    std::unique_ptr<uint8_t[]> wram = std::move(cpu.bus.cgb_wram);
    std::unique_ptr<MBC> mbc = std::move(cpu.bus.mbc);
    auto synths = std::move(apu.synths);

    save.write(reinterpret_cast<char*>(&pad), sizeof(Joypad));
    save.write(reinterpret_cast<char*>(&timer), sizeof(Timer));
    save.write(reinterpret_cast<char*>(&ppu), sizeof(PPU));
    save.write(reinterpret_cast<char*>(&cpu), sizeof(CPU));
    save.write(reinterpret_cast<char*>(&apu), sizeof(APU));

    if(mbc)
      mbc->save_state(&save);
    if(vbank)
      save.write(reinterpret_cast<char*>(vbank.get()), 8*1024);
    if(wram)
      save.write(reinterpret_cast<char*>(wram.get()), 32*1024);
    if(bg_palette && cpu.modo > 0)
      save.write(reinterpret_cast<char*>(bg_palette.get()), 64);
    if(obj_palette && cpu.modo > 0)
      save.write(reinterpret_cast<char*>(obj_palette.get()), 64);
    
    cpu.bus.mbc = std::move(mbc);
    ppu.vram_bank1 = std::move(vbank);
    ppu.bg_palette_ram = std::move(bg_palette);
    ppu.obj_palette_ram = std::move(obj_palette);
    cpu.bus.cgb_wram = std::move(wram);
    apu.synths = std::move(synths);

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
    auto synths = std::move(apu.synths);
    const void *tec = pad.teclas;
    const void *but = pad.botoes;
    Texture2D *frame = ppu.raylib_texture;
    uint8_t modo_cpu = cpu.modo;
    bool paleta_cgb = ppu.paleta_cgb;

    save.read(reinterpret_cast<char*>(&pad), sizeof(Joypad));
    save.read(reinterpret_cast<char*>(&timer), sizeof(Timer));
    save.read(reinterpret_cast<char*>(&ppu), sizeof(PPU));
    save.read(reinterpret_cast<char*>(&cpu), sizeof(CPU));
    save.read(reinterpret_cast<char*>(&apu), sizeof(APU));

    if(mbc)
      mbc->load_state(&save);
    if(vbank)
      save.read(reinterpret_cast<char*>(vbank.get()), 8*1024);
    if(wram)
      save.read(reinterpret_cast<char*>(wram.get()), 32*1024);
    if(bg_palette && modo_cpu > 0)
      save.read(reinterpret_cast<char*>(bg_palette.get()), 64);
    if(obj_palette && modo_cpu > 0)
      save.read(reinterpret_cast<char*>(obj_palette.get()), 64);
    
    cpu.modo = modo_cpu;
    ppu.modo_cpu = modo_cpu;
    ppu.paleta_cgb = paleta_cgb;
    apu.seta_modo(paleta_cgb);

    cpu.bus.mbc = std::move(mbc);
    ppu.vram_bank1 = std::move(vbank);
    ppu.bg_palette_ram = std::move(bg_palette);
    ppu.obj_palette_ram = std::move(obj_palette);
    cpu.bus.cgb_wram = std::move(wram);
    apu.synths = std::move(synths);

    cpu.bus.restaura_rom = &restaura_rom;
    cpu.bus.timer = &timer;
    timer.apu = &apu;
    cpu.bus.pad = &pad;
    cpu.bus.ppu = &ppu;
    cpu.bus.dma.mbc = cpu.bus.mbc.get();
    cpu.bus.dma.wram = cpu.bus.cgb_wram.get();
    apu.memoria = cpu.bus.memoria.data();
    apu.ch1.memoria = cpu.bus.memoria.data();
    apu.ch1.synth = &apu.synths[0];
    apu.ch2.memoria = cpu.bus.memoria.data();
    apu.ch2.synth = &apu.synths[1];
    apu.ch3.memoria = cpu.bus.memoria.data();
    apu.ch3.synth = &apu.synths[2];
    apu.ch4.memoria = cpu.bus.memoria.data();
    apu.ch4.synth = &apu.synths[3];
    pad.teclas = tec;
    pad.botoes = but;
    pad.p1 = &cpu.bus.memoria[0xFF00];
    ppu.memoria = cpu.bus.memoria.data();
    ppu.raylib_texture = frame;
    ppu.hdma_hblank = &cpu.bus.hdma.modo_hblank;
    ppu.hdma_ativo = &cpu.bus.hdma.ativo;
    limpa_samples();

    std::cout << "Save carregado no slot " << slot << ".\n";
  }

  void delete_state(size_t slot){
    if(!save_states[slot - 1]){
      std::cout << "Nenhum save encontrado no slot selecionado.\n";
      return;
    }

    size_t vazios {};
    size_t copia {save_states[slot - 1]};
    save_states[slot - 1] = 0;
    for(size_t i {}; i < MAX_SAVE_SLOTS; ++i){
      if(!save_states[i])
        ++vazios;
    }

    if(vazios == MAX_SAVE_SLOTS){
      std::error_code err;
      if(std::filesystem::remove(save_path, err)){
        frame_states[slot - 1] = 0;
        std::filesystem::remove(image_path, err);
        return;
      }

      TraceLog(LOG_WARNING, "Não foi possível apagar o arquivo. '%s'", err.message().c_str());
    }

    std::fstream save(save_path, save.in | save.out | save.binary);
    size_t deletados = this->get_excluidos(&save);
    ++deletados;
    
    save.seekp(11*sizeof(size_t), std::ios::beg);
    save.write(reinterpret_cast<char*>(&deletados), sizeof(size_t));
    save.seekp((12 + (deletados - 1))*sizeof(size_t), std::ios::beg);
    save.write(reinterpret_cast<char*>(&copia), sizeof(size_t));
    save.seekp((slot - 1)*sizeof(size_t), std::ios::beg);
    save.write(reinterpret_cast<char*>(&save_states[slot - 1]), sizeof(size_t));

    save.close();
    save.clear();

    if(!frame_states[slot - 1]) return;

    save.open(image_path, save.in | save.out | save.binary);
    if(!save) return;

    save.seekg(10*sizeof(size_t), std::ios::beg);
    save.read(reinterpret_cast<char*>(&deletados), sizeof(size_t));
    ++deletados;
    save.seekp(10*sizeof(size_t), std::ios::beg);
    save.write(reinterpret_cast<char*>(&deletados), sizeof(size_t));
    save.seekp((11 + (deletados - 1))*sizeof(size_t), std::ios::beg);
    save.write(reinterpret_cast<char*>(&frame_states[slot - 1]), sizeof(size_t));

    frame_states[slot - 1] = 0;
    save.seekp((slot - 1)*sizeof(size_t), std::ios::beg);
    save.write(reinterpret_cast<char*>(&frame_states[slot - 1]), sizeof(size_t));
  }

  ~Game_State(){
    limpa_samples();
  }
 
};

void le_input(Game_State *estado, size_t save_slot, bool& pausado, bool& is_120, bool& janela_alterada);
void le_input_controle(Game_State *estado, size_t save_slot, bool& pausado, bool& is_120, bool& janela_alterada, uint8_t& controles, int gamepad);
void display_saves(Game_State *game, GB_State *estado);
bool pausa_jogo(Game_State *game, GB_State *estado, bool& pausado, bool& resumido);

}

#endif
