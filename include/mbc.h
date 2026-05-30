#ifndef MBC_H
#define MBC_H

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <filesystem>

namespace GB{

struct MBC{
  std::vector<uint8_t> rom;
  std::vector<uint8_t> ram;
  std::string fonte;
  uint8_t *pega_rom(void){
    return rom.data();
  };

  virtual uint8_t& read(uint16_t endereco)=0;
  virtual void write(uint16_t endereco, uint8_t valor)=0;

  void save(void);
  void load(void);
  virtual ~MBC() = default;
};

//cada bank de rom tem 16kb de memória, os de ram tem só 8kb
struct MBC1 : public MBC{
  uint16_t total_banks{};
  uint8_t rom_bank {1}; //1 a 127, o bank 0 fica sempre no range 0x0000-0x3FFF
  uint8_t ram_bank {}; //0 a 3
  bool ram_banking {false};
  bool ram_ativa {false};
  uint8_t ram_hack {0xFF};

  MBC1(std::string_view rom_src, size_t rom_tam, size_t ram_tam){
    std::fstream arquivo(rom_src.data(), arquivo.in | arquivo.binary);
    if(!arquivo){
      std::cerr << "Rom não encontrada.\n";
      std::terminate();
    }

    fonte = rom_src;
    rom.resize(rom_tam);
    if(ram_tam)
      ram.resize(ram_tam);
    total_banks = rom.size()/(16*1024);
    arquivo.read(reinterpret_cast<char*>(rom.data()), rom.size());
    std::cout << "MBC3 inicializado. RAM alocada: " << ram_tam/1024 << "KB\n";
    std::cout << "ROM alocada: " << rom.size()/1024 << "KB\n";
  }

  uint8_t& read(uint16_t endereco) override;
  void write(uint16_t endereco, uint8_t valor) override;   
};

struct MBC2: public MBC{
  uint16_t total_banks{};
  uint8_t rom_bank {1};
  bool ram_ativa {false};
  uint8_t ram_hack {0xFF};

  MBC2(std::string_view rom_src, size_t rom_tam){
    std::fstream arquivo(rom_src.data(), arquivo.in | arquivo.binary);
    if(!arquivo){
      std::cerr << "Rom não encontrada.\n";
      std::terminate();
    }

    fonte = rom_src;
    rom.resize(rom_tam);
    ram.resize(512);
    arquivo.read(reinterpret_cast<char*>(rom.data()), rom.size());
    total_banks = rom.size()/(16*1024);
    std::cout << "MBC2 inicializado. RAM alocada: 512 bytes\n";
    std::cout << "ROM alocada: " << rom.size()/1024 << "KB\n";
  }

  uint8_t& read(uint16_t endereco) override;
  void write(uint16_t endereco, uint8_t valor) override;   
};

struct MBC3 : public MBC{
  uint16_t total_banks{};
  uint8_t rom_bank {1};
  uint8_t ram_bank {};
  bool ram_ativa {false};
  uint8_t ram_hack {0xFF};
  uint8_t rtc_registrador {};
  bool rtc_selected {false};
  bool latch_clock {false};

  struct RTC{
    uint8_t s {}; //segundos
    uint8_t m {}; //minutos
    uint8_t h {}; //horas
    uint8_t d {}; //dias
    uint8_t f {}; //flags e dias superiores
  };

  RTC rtc;
  RTC rtc_latch;

  MBC3(std::string_view rom_src, size_t rom_tam, size_t ram_tam){
    std::fstream arquivo(rom_src.data(), arquivo.in | arquivo.binary);
    if(!arquivo){
      std::cerr << "Rom não encontrada.\n";
      std::terminate();
    }

    fonte = rom_src;
    arquivo.seekg(0, std::ios::end);
    size_t tamanho = arquivo.tellg();
    arquivo.seekg(0);
    rom.resize(rom_tam);
    if(ram_tam)
      ram.resize(ram_tam);
    total_banks = rom.size()/(16*1024);
    arquivo.read(reinterpret_cast<char*>(rom.data()), rom.size());
    std::cout << "MBC3 inicializado. RAM alocada: " << ram_tam/1024 << "KB\n";
    std::cout << "ROM alocada: " << rom.size()/1024 << "KB\n";
  }

  uint8_t& read(uint16_t endereco) override;
  void write(uint16_t endereco, uint8_t valor) override;
};

struct MBC5 : public MBC{
  uint16_t total_banks{};
  uint16_t rom_bank {}; //bank 0 disponível
  uint16_t ram_bank {};
  bool ram_ativa {false};
  uint8_t ram_hack {0xFF};

  MBC5(std::string_view rom_src, size_t rom_tam, size_t ram_tam){
    std::fstream arquivo(rom_src.data(), arquivo.in | arquivo.binary);
    if(!arquivo){
      std::cerr << "Rom não encontrada.\n";
      std::terminate();
    }

    fonte = rom_src;
    rom.resize(rom_tam);
    if(ram_tam)
      ram.resize(ram_tam);
    total_banks = rom.size()/(16*1024);
    arquivo.read(reinterpret_cast<char*>(rom.data()), rom.size());
    std::cout << "MBC5 inicializado. RAM alocada: " << ram_tam/1024 << "KB\n";
    std::cout << "ROM alocada: " << rom.size()/1024 << "KB\n";
  }

  uint8_t& read(uint16_t endereco) override;
  void write(uint16_t endereco, uint8_t valor) override;
};

}

#endif
