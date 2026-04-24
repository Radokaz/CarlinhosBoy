#ifndef INIT_H
#define INIT_H

#include "actions.h"
#include <string>
#include <fstream>
#include <map>

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

inline Header *init_rom(CPU *cpu, std::string_view src){
  std::fstream arquivo(src.data(), arquivo.binary | arquivo.in);
  if(!arquivo){
    std::cerr << "Não foi possível abrir a ROM.\n";
    exit(1);
  }

  arquivo.read(reinterpret_cast<char*>(cpu->bus.memoria.data()), 0x8000);
  cpu->bus.memoria[0xFF04] = 0xAB;
  cpu->bus.memoria[0xFF05] = 0x00;
  cpu->bus.memoria[0xFF06] = 0x00;
  cpu->bus.memoria[0xFF07] = 0xF8;
  cpu->bus.memoria[0xFF10] = 0x80;
  cpu->bus.memoria[0xFF11] = 0xBF;
  cpu->bus.memoria[0xFF12] = 0xF3;
  cpu->bus.memoria[0xFF14] = 0xBF;
  cpu->bus.memoria[0xFF16] = 0x3F;
  cpu->bus.memoria[0xFF0F] = 0xE1;
  cpu->bus.memoria[0xFFFF] = 0x00;
  cpu->bus.memoria[0xFF00] = 0xFF;
  cpu->bus.memoria[0xFF40] = 0x91; // LCDC
  cpu->bus.memoria[0xFF41] = 0x85; // STAT
  cpu->bus.memoria[0xFF44] = 0x00; // LY
  cpu->bus.memoria[0xFF47] = 0xFC; // BGP

  return reinterpret_cast<Header*>(&cpu->bus.memoria[0x100]);
}

inline void checa_validade(Header *header, CPU *cpu){

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
  std::cout << "\t ROM Size : " << std::dec << static_cast<int>(header->rom_tam) << "KB\n";
  std::cout << "\t RAM Size : " << std::hex << static_cast<int>(header->ram_tam) << "\n";
  std::cout << "\t LIC Code : " << static_cast<int>(header->lic_code) << " (" << ((header->new_lic_code <= 0xA4) ? LIC_CODE[header->lic_code] : "DESCONHECIDO") << ")\n";
  std::cout << "\t ROM Vers : " << static_cast<int>(header->versao) << "\n";

  size_t aux {};
  for(size_t i {0x0134}; i < 0x014D; ++i){
    aux = aux - cpu->bus.memoria[i] - 1;
  }

  std::cout << "\t Checksum : " << static_cast<int>(header->checksum) << " (" << ((aux & 0xFF) ? "SUCESSO)\n" : "FALHA)\n") << std::dec; 
  if(!(aux & 0xFF)){
    std::cerr << "Erro ao ler a ROM.\n";
    exit(1);
  }
}

inline void init_game(CPU *cpu, Timer& timer, char **argv){
  Header *header = init_rom(cpu, argv[1]);
  checa_validade(header, cpu);
}


}

#endif
