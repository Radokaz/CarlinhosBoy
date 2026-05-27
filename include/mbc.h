#ifndef MBC_H
#define MBC_H

#include <cstdint>
#include <fstream>
#include <vector>

struct MBC{
  std::vector<uint8_t> rom;
  std::vector<uint8_t> ram;
  uint8_t *pega_rom(void){
    return rom.data();
  };

  virtual uint8_t& read(uint16_t endereco)=0;
  virtual void write(uint16_t endereco, uint8_t valor)=0;
  virtual ~MBC() = default;
};

//cada bank tem 16kb de memória
struct MBC1 : public MBC{
  uint16_t total_banks{};
  uint8_t rom_bank {1}; //1 a 127, o bank 0 fica sempre no range 0x0000-0x3FFF
  uint8_t ram_bank {}; //0 a 3
  bool ram_banking {false};
  bool ram_ativa {false};
  uint8_t ram_hack {0xFF};

  MBC1(std::string_view rom_src){
    std::fstream arquivo(rom_src.data(), arquivo.in | arquivo.binary);
    if(!arquivo){
      std::cerr << "Rom não encontrada.\n";
      std::terminate();
    }

    arquivo.seekg(0, std::ios::end);
    size_t tamanho = arquivo.tellg();
    arquivo.seekg(0);
    rom.resize(tamanho);
    arquivo.read(reinterpret_cast<char*>(rom.data()), rom.size());
    ram.resize(1024*32);
    total_banks = rom.size()/(16*1024);
    std::cout << "MBC1 inicializado.\n";
  }

  uint8_t& read(uint16_t endereco) override {
    if(endereco < 0x4000){
      return rom[endereco];
    }
    if(endereco < 0x8000){
      uint32_t address = (rom_bank*0x4000) + (endereco - 0x4000);
      return rom[address];
    }

    uint32_t address = (ram_bank*0x2000) + (endereco - 0xA000);
    ram_hack = 0xFF;
    return (ram_ativa) ? ram[address] : ram_hack;
  }

  void write(uint16_t endereco, uint8_t valor) override {
    if(endereco < 0x2000){
      ram_ativa = ((valor & 0x0F) == 0x0A) ? true : false;
      return;
    }

    if(endereco < 0x4000){
      rom_bank = ((rom_bank & 0x60) | (valor & 0x1F));
      if(!(rom_bank & 0x1F))
        ++rom_bank;
      
      rom_bank %= total_banks;

      return;
    }
    
    if(endereco < 0x6000){
      if(ram_banking)
        ram_bank = valor & 0x03;
      else{
        rom_bank = ((rom_bank & 0x1F) | ((valor & 0x03) << 5));

        if(!(rom_bank & 0x1F))
          ++rom_bank;
  
        rom_bank %= total_banks;
      }

      return;
    }

    if(endereco < 0x8000){
      ram_banking = valor & 0x01;
      return;
    }
    
    //Save aqui
    if(ram_ativa){
      uint32_t address = (ram_bank*0x2000) + (endereco - 0xA000);
      ram[address] = valor;
    }
  }

};

struct MBC3 : public MBC{
  uint16_t total_banks{};
  uint8_t rom_bank {1};
  uint8_t ram_bank {};
  bool ram_banking {false};
  bool ram_ativa {false};
  uint8_t ram_hack {0xFF};
  uint8_t rtc_registrador {};
  bool rtc_selected {false};
  bool halt_flag {false};

  struct RTC{
    uint8_t s {}; //segundos
    uint8_t m {}; //minutos
    uint8_t h {}; //horas
    uint8_t d {}; //dias
    uint8_t f {}; //flags e dias superiores
  };

  RTC rtc;
  RTC rtc_latch;

  MBC3(std::string_view rom_src){
    std::fstream arquivo(rom_src.data(), arquivo.in | arquivo.binary);
    if(!arquivo){
      std::cerr << "Rom não encontrada.\n";
      std::terminate();
    }

    arquivo.seekg(0, std::ios::end);
    size_t tamanho = arquivo.tellg();
    arquivo.seekg(0);
    rom.resize(tamanho);
    arquivo.read(reinterpret_cast<char*>(rom.data()), rom.size());
    ram.resize(1024*64);
    total_banks = rom.size()/(16*1024);
    std::cout << "MBC3 inicializado.\n";
  }

  uint8_t& read(uint16_t endereco) override {
    if(endereco < 0x4000){
      return rom[endereco];
    }
    if(endereco < 0x8000){
      uint32_t address = (rom_bank*0x4000) + (endereco - 0x4000);
      return rom[address];
    }
    if(!ram_ativa){
      ram_hack = 0xFF;
      return ram_hack;
    }
    if(rtc_selected){
      switch(rtc_registrador){
        case 0x08: return rtc_latch.s;
        case 0x09: return rtc_latch.m;
        case 0x0A: return rtc_latch.h;
        case 0x0B: return rtc_latch.d;
        case 0x0C: return rtc_latch.f;
      }

      ram_hack = 0xFF;
      return ram_hack;
    }

    uint32_t address = (ram_bank*0x2000) + (endereco - 0xA000);
    return ram[address];
  }

  void write(uint16_t endereco, uint8_t valor) override {
    if(endereco < 0x2000){
      ram_ativa = ((valor & 0x0F) == 0x0A) ? true : false;
      return;
    }

    if(endereco < 0x4000){
      rom_bank = valor & 0x7F;
      if(!rom_bank)
        rom_bank = 1;
      
      rom_bank %= total_banks;
      
      return;
    }

    if(endereco < 0x6000){
      if(valor <= 3){
        ram_bank = valor ;
        rtc_selected = false;
      }
      else if(valor >= 8 && valor <= 12){
        rtc_selected = true;
        rtc_registrador = valor;
      }

      return;
    }

    if(endereco < 0x8000){
      if(valor == 0 && !halt_flag){
        halt_flag = true;
      }
      else if(halt_flag && valor == 1){
        rtc_latch = rtc;
        halt_flag = false;
      }
      else{
        halt_flag = false;
      }

      return;
    }

    if(ram_ativa){
      uint32_t address = (ram_bank*0x2000) + (endereco - 0xA000);
      ram[address] = valor;
      return;
    }
    if(rtc_selected){
      switch(rtc_registrador){
        case 0x08: 
          rtc.s = valor & 0x3F;
          break;
        case 0x09: 
          rtc.m = valor & 0x3F;
          break;
        case 0x0A: 
          rtc.h = valor & 0x1F;
          break;
        case 0x0B: 
          rtc.d = valor;
          break;
        case 0x0C: 
          rtc.f = valor & 0xC1;
          break;
      }
    }
  }

};

struct MBC5 : public MBC{

};

#endif
