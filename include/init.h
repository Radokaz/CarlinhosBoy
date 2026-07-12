#ifndef INIT_H
#define INIT_H

#include "interface.h"
#include <map>

#define BOOT_SOURCE_DMG "dmg_boot.bin"
#define BOOT_SOURCE_CGB "cgb_boot.bin"

namespace GB{

struct __attribute__((packed)) Header{
  uint8_t entry[4];
  uint8_t logo[0x30];
  char titulo[16];
  uint16_t new_lic_code;
  uint8_t sgb;
  uint8_t mbc;
  uint8_t rom_tam;
  uint8_t ram_tam;
  uint8_t dest_code;
  uint8_t lic_code;
  uint8_t versao;
  uint8_t checksum;
  uint16_t global_checksum;
};

inline void merge_boot_rom(CPU *cpu, std::string_view src, uint8_t mbc){

  std::filesystem::path boot_src = (cpu->bus.ppu->paleta_cgb) ? getExeDir() / BOOT_SOURCE_CGB : getExeDir() / BOOT_SOURCE_DMG;
  std::fstream bootrom(boot_src.string().c_str(), bootrom.binary | bootrom.in);

  if(!bootrom){
    std::cerr << "BOOTROM INDISPONÍVEL\n";
    if(cpu->modo > 0){
      cpu->registradores.a = 0x11;
      cpu->registradores.b = 0x00;
      cpu->registradores.c = 0x00; 
      cpu->registradores.d = 0xFF;
      cpu->registradores.e = 0x56;
      cpu->registradores.h = 0x00;
      cpu->registradores.l = 0x0D;
      cpu->registradores.f = 0x80;
      cpu->bus.memoria[0xFF4D] = 0x7E;
    }
    
    if(cpu->modo == 3){
      cpu->bus.ppu->paleta_cgb = false;
      cpu->bus.timer->apu->seta_modo(cpu->bus.ppu->paleta_cgb);
    }

    cpu->bus.memoria[0xFF00] = 0xCF; //Joypad
    cpu->bus.memoria[0xFF04] = 0xAB; //DIV
    cpu->bus.memoria[0xFF05] = 0x00; //TIMA
    cpu->bus.memoria[0xFF06] = 0x00; //TMA
    cpu->bus.memoria[0xFF07] = 0xF8; //TAC
    cpu->bus.memoria[0xFF0F] = 0xE1; //IF
    cpu->bus.memoria[0xFF10] = 0x80;
    cpu->bus.memoria[0xFF11] = 0xBF;
    cpu->bus.memoria[0xFF12] = 0xF3;
    cpu->bus.memoria[0xFF14] = 0xBF;
    cpu->bus.memoria[0xFF16] = 0x3F;
    cpu->bus.memoria[0xFF40] = 0x91; // LCDC
    cpu->bus.memoria[0xFF41] = 0x85; // STAT
    cpu->bus.memoria[0xFF42] = 0x00; // SCY
    cpu->bus.memoria[0xFF43] = 0x00; // SCX
    cpu->bus.memoria[0xFF44] = 0x00; // LY
    cpu->bus.memoria[0xFF45] = 0x00; // LYC
    cpu->bus.memoria[0xFF46] = (cpu->bus.ppu->paleta_cgb) ? 0x00 : 0xFF; // DMA
    cpu->bus.memoria[0xFF47] = 0xFC; // BGP
    cpu->bus.memoria[0xFF48] = 0xFF; // OBP0
    cpu->bus.memoria[0xFF49] = 0xFF; // OBP1
    cpu->bus.memoria[0xFF4A] = 0x00; // WY
    cpu->bus.memoria[0xFF4B] = 0x00; // WX
    cpu->bus.memoria[0xFFFF] = 0x00; //IE
    return;
  }

  cpu->pc = 0;
  cpu->bus.timer->div_count = 0;
  switch(mbc){
    case 1:
    case 2:
    case 3:
    case 5:
    case 6:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
      if(cpu->bus.ppu->paleta_cgb){
        bootrom.read(reinterpret_cast<char*>(cpu->bus.mbc->pega_rom()), 0x0100);
        bootrom.seekg(0x0200, std::ios::beg);
        bootrom.read(reinterpret_cast<char*>(cpu->bus.mbc->pega_rom() + 0x0200), 0x0700);
        cpu->bus.restaura_rom = [src, cpu](){
          std::fstream rom(src.data(), rom.binary | rom.in);
          rom.read(reinterpret_cast<char*>(cpu->bus.mbc->pega_rom()), 0x0100);
          rom.seekg(0x0200, std::ios::beg);
          rom.read(reinterpret_cast<char*>(cpu->bus.mbc->pega_rom() + 0x200), 0x0700);
          if(cpu->modo == 3){
            cpu->modo = 0;
            cpu->bus.ppu->modo_cpu = 0;
            cpu->bus.cgb_wram.reset();
            cpu->bus.ppu->vram_bank1.reset();
          }
        };
      }
      else{
        bootrom.read(reinterpret_cast<char*>(cpu->bus.mbc->pega_rom()), 0x0100);
        cpu->bus.restaura_rom = [src, cpu](){
          std::fstream rom(src.data(), rom.binary | rom.in);
          rom.read(reinterpret_cast<char*>(cpu->bus.mbc->pega_rom()), 0x0100);
        };
      }
      break;
    default:
      if(cpu->bus.ppu->paleta_cgb){
        bootrom.read(reinterpret_cast<char*>(cpu->bus.memoria.data()), 0x0100);
        bootrom.seekg(0x0200, std::ios::beg);
        bootrom.read(reinterpret_cast<char*>(cpu->bus.memoria.data() + 0x0200), 0x0700);
        cpu->bus.restaura_rom = [src, cpu](){
          std::fstream rom(src.data(), rom.binary | rom.in);
          rom.read(reinterpret_cast<char*>(cpu->bus.memoria.data()), 0x0100);
          rom.seekg(0x0200, std::ios::beg);
          rom.read(reinterpret_cast<char*>(cpu->bus.memoria.data() + 0x200), 0x0700);
          if(cpu->modo == 3){
            cpu->modo = 0;
            cpu->bus.ppu->modo_cpu = 0;
            cpu->bus.cgb_wram.reset();
            cpu->bus.ppu->vram_bank1.reset();
          }
        };
      }
      else{
        bootrom.read(reinterpret_cast<char*>(cpu->bus.memoria.data()), 0x0100);
        cpu->bus.restaura_rom = [src, cpu](){
          std::fstream rom(src.data(), rom.binary | rom.in);
          rom.read(reinterpret_cast<char*>(cpu->bus.memoria.data()), 0x0100);
        };
      }
      break;
  }
}

inline Header *init_rom(CPU *cpu, std::string_view src){
  std::fstream arquivo(src.data(), arquivo.binary | arquivo.in);
  if(!arquivo){
    std::cerr << "Não foi possível abrir a ROM.\n";
    return nullptr;
  }

  arquivo.read(reinterpret_cast<char*>(cpu->bus.memoria.data()), 0x0150);
  return reinterpret_cast<Header*>(&cpu->bus.memoria[0x100]);
}

inline void checa_modo(CPU *cpu, int paleta){
  uint8_t modo = cpu->bus.memoria[0x143];
  if(modo == 0x80 && paleta){
    cpu->modo = 1;
    cpu->bus.ppu->modo_cpu = 1;
    cpu->bus.ppu->paleta_cgb = true;
    cpu->bus.cgb_wram = std::make_unique<uint8_t[]>(32*1024);
    cpu->bus.ppu->vram_bank1 = std::make_unique<uint8_t[]>(8*1024);
    cpu->bus.ppu->bg_palette_ram = std::make_unique<uint8_t[]>(64);
    cpu->bus.ppu->obj_palette_ram = std::make_unique<uint8_t[]>(64);
    std::cout << "Inicializando DMG em CGB Mode.\n";
  }
  else if(modo == 0xC0){
    cpu->modo = 2;
    cpu->bus.ppu->modo_cpu = 2;
    cpu->bus.ppu->paleta_cgb = true;
    cpu->bus.cgb_wram = std::make_unique<uint8_t[]>(32*1024);
    cpu->bus.ppu->vram_bank1 = std::make_unique<uint8_t[]>(8*1024);
    cpu->bus.ppu->bg_palette_ram = std::make_unique<uint8_t[]>(64);
    cpu->bus.ppu->obj_palette_ram = std::make_unique<uint8_t[]>(64);
    std::cout << "Inicializando em CBG Only.\n";
  }
  else{
    cpu->modo = 0;
    cpu->bus.ppu->modo_cpu = 0;
    cpu->bus.ppu->paleta_cgb = (paleta) ? true : false;
    if(paleta){
      cpu->bus.ppu->bg_palette_ram = std::make_unique<uint8_t[]>(64);
      cpu->bus.ppu->obj_palette_ram = std::make_unique<uint8_t[]>(64);
      cpu->bus.cgb_wram = std::make_unique<uint8_t[]>(32*1024);
      cpu->bus.ppu->vram_bank1 = std::make_unique<uint8_t[]>(8*1024);
      cpu->modo = 3;
      cpu->bus.ppu->modo_cpu = 3;
    }
    std::cout << "Inicializando em DMG Only.\n";
  }

  cpu->bus.timer->apu->seta_modo(cpu->bus.ppu->paleta_cgb);
}

inline size_t checa_tamanho_ram(Header *header){
  switch(header->ram_tam){
    case 0: return 0; //não usa ram externa
    case 1: return 1024*2;
    case 2: return 1024*8;
    case 3: return 1024*32;
    case 4: return 1024*128;
    default: return 1024*64;
  }
}

inline size_t checa_tamanho_rom(Header *header){
  switch(header->rom_tam){
    case 0: return 1024*32;
    case 1: return 1024*64;
    case 2: return 1024*128;
    case 3: return 1024*256;
    case 4: return 1024*512;
    case 5: return 1024*1024;
    case 6: return 1024*2048;
    case 7: return 1024*4096;
    default: return 1024*8192;
  }
}

inline bool checa_validade(Header *header, CPU *cpu, std::string_view src, std::string_view saves, int paleta){

  const char *ROM_TYPES[] = {
    "ROM ONLY",
    "MBC1",
    "MBC1+RAM",
    "MBC1+RAM+BATTERY",
    "0x04 ???",
    "MBC2",
    "MBC2+BATTERY",
    "0x07 ???",
    "ROM+RAM 1",
    "ROM+RAM+BATTERY 1",
    "0x0A ???",
    "MMM01",
    "MMM01+RAM",
    "MMM01+RAM+BATTERY",
    "0x0E ???",
    "MBC3+TIMER+BATTERY",
    "MBC3+TIMER+RAM+BATTERY 2",
    "MBC3",
    "MBC3+RAM 2",
    "MBC3+RAM+BATTERY 2",
    "0x14 ???",
    "0x15 ???",
    "0x16 ???",
    "0x17 ???",
    "0x18 ???",
    "MBC5",
    "MBC5+RAM",
    "MBC5+RAM+BATTERY",
    "MBC5+RUMBLE",
    "MBC5+RUMBLE+RAM",
    "MBC5+RUMBLE+RAM+BATTERY",
    "0x1F ???",
    "MBC6",
    "0x21 ???",
    "MBC7+SENSOR+RUMBLE+RAM+BATTERY",
  };

  std::map<uint8_t, const char *> LIC_CODE = {
    {0x00,"None"},
    {0x01, "Nintendo R&D1"},
    {0x08, "Capcom"},
    {0x13, "Electronic Arts"},
    {0x18, "Hudson Soft"},
    {0x19, "b-ai"},
    {0x20, "kss"},
    {0x22, "pow"},
    {0x24, "PCM Complete"},
    {0x25, "san-x"},
    {0x28, "Kemco Japan"},
    {0x29, "seta"},
    {0x30, "Viacom"},
    {0x31, "Nintendo"},
    {0x32, "Bandai"},
    {0x33, "Ocean/Acclaim"},
    {0x34, "Konami"},
    {0x35, "Hector"},
    {0x37, "Taito"},
    {0x38, "Hudson"},
    {0x39, "Banpresto"},
    {0x41, "Ubi Soft"},
    {0x42, "Atlus"},
    {0x44, "Malibu"},
    {0x46, "angel"},
    {0x47, "Bullet-Proof"},
    {0x49, "irem"},
    {0x50, "Absolute"},
    {0x51, "Acclaim"},
    {0x52, "Activision"},
    {0x53, "American sammy"},
    {0x54, "Konami"},
    {0x55, "Hi tech entertainment"},
    {0x56, "LJN"},
    {0x57, "Matchbox"},
    {0x58, "Mattel"},
    {0x59, "Milton Bradley"},
    {0x60, "Titus"},
    {0x61, "Virgin"},
    {0x64, "LucasArts"},
    {0x67, "Ocean"},
    {0x69, "Electronic Arts"},
    {0x70, "Infogrames"},
    {0x71, "Interplay"},
    {0x72, "Broderbund"},
    {0x73, "sculptured"},
    {0x75, "sci"},
    {0x78, "THQ"},
    {0x79, "Accolade"},
    {0x80, "misawa"},
    {0x83, "lozc"},
    {0x86, "Tokuma Shoten Intermedia"},
    {0x87, "Tsukuda Original"},
    {0x91, "Chunsoft"},
    {0x92, "Video system"},
    {0x93, "Ocean/Acclaim"},
    {0x95, "Varie"},
    {0x96, "Yonezawa/s’pal"},
    {0x97, "Kaneko"},
    {0x99, "Pack in soft"},
    {0xA4, "Konami (Yu-Gi-Oh!)"}
  };

  std::cout << "\t Titulo   : " << header->titulo << "\n";
  std::cout << "\t Tipo     : " << std::hex << static_cast<int>(header->mbc) << " (" << ((header->mbc <= 0x22) ? ROM_TYPES[header->mbc] : "DESCONHECIDO") << ")\n";
  std::cout << "\t ROM Size : " << std::dec << static_cast<int>(header->rom_tam) << " banks\n";
  std::cout << "\t RAM Size : " << std::hex << static_cast<int>(header->ram_tam) << " banks\n";
  std::cout << "\t LIC Code : " << static_cast<int>(header->lic_code) << " (" << ((LIC_CODE.count(header->lic_code)) ? LIC_CODE[header->lic_code] : "DESCONHECIDO") << ")\n";
  std::cout << "\t ROM Vers : " << static_cast<int>(header->versao) << "\n";

  size_t aux {};
  for(size_t i {0x0134}; i < 0x014D; ++i){
    aux = aux - cpu->bus.memoria[i] - 1;
  }

  std::cout << "\t Checksum : " << static_cast<int>(header->checksum) << " (" << ((aux & 0xFF) ? "SUCESSO)\n" : "FALHA)\n") << std::dec; 
  if(!(aux & 0xFF)){
    std::cerr << "Erro ao ler a ROM.\n";
    return false;
  }
  
  checa_modo(cpu, paleta);
  size_t ram_sz = checa_tamanho_ram(header);
  size_t rom_sz = checa_tamanho_rom(header);

  switch(header->mbc){
    case 1:
    case 2:
    case 3:{
      cpu->bus.mbc = std::make_unique<MBC1>(saves, src, rom_sz, ram_sz);
      break;
    }
    case 5:
    case 6:{
      cpu->bus.mbc = std::make_unique<MBC2>(saves, src, rom_sz);
      break;
    }
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:{
      cpu->bus.mbc = std::make_unique<MBC3>(saves, src, rom_sz, ram_sz);
      break;
    }
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:{
      cpu->bus.mbc = std::make_unique<MBC5>(saves, src, rom_sz, ram_sz);
      break;
    }
    default:{
      std::fstream rom(src.data(), rom.in | rom.binary);
      rom.read(reinterpret_cast<char*>(cpu->bus.memoria.data()), 0x8000);
    }
  }

  return true;
}


inline void checa_save(CPU *cpu, uint8_t mbc){
  if(!cpu->bus.mbc) return;

  switch(mbc){
    case 3:
    case 6:
    case 9:
    case 15:
    case 16:
    case 18:
    case 19:
    case 27:
    case 30:
      cpu->bus.mbc->tem_save = true;
      break;
    default:
      break;
  }

  if(cpu->bus.mbc->tem_save){
    cpu->bus.mbc->load();
  }
}

inline bool init_game(CPU *cpu, const char *src, const char *saves, int paleta){
  Header *header = init_rom(cpu, src);
  if(!header) return false;

  uint8_t mbc = header->mbc;
  if(!checa_validade(header, cpu, src, saves, paleta))
    return false;

  merge_boot_rom(cpu, src, mbc);
  checa_save(cpu, mbc);

  return true;
}


}

#endif
