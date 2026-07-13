#include <iostream>
#include "mbc.h"

namespace GB{

void MBC::load(void){
    std::filesystem::create_directories(saves);
    std::filesystem::path save_path = saves / std::filesystem::path(fonte).filename();
    save_path.replace_extension(".sav");

    std::fstream arquivo(save_path, arquivo.in | arquivo.binary);
    if(!arquivo) return;

    arquivo.read(reinterpret_cast<char*>(ram.data()), ram.size());
    std::cout << "Save carregado.\n";
}

void MBC::save(void){
    std::filesystem::create_directories(saves);
    std::filesystem::path save_path = saves / std::filesystem::path(fonte).filename();
    save_path.replace_extension(".sav");

    std::ofstream arquivo(save_path, arquivo.binary | arquivo.trunc);
    if(!arquivo) return;

    arquivo.write(reinterpret_cast<char*>(ram.data()), ram.size());
    std::cout << "Jogo salvo.\n";
}

uint8_t& MBC1::read(uint16_t endereco){
    if(endereco < 0x4000){
      return rom[endereco];
    }
    if(endereco < 0x8000){
      uint32_t address = (rom_bank*0x4000) + (endereco - 0x4000);
      return rom[address];
    }

    uint32_t address = (ram_bank*0x2000) + (endereco - 0xA000);
    ram_hack = 0xFF;
    if(address >= ram.size())
        address &= (ram.size() - 1);

    return (ram_ativa && ram.size()) ? ram[address] : ram_hack;
}

void MBC1::write(uint16_t endereco, uint8_t valor){
    if(endereco < 0x2000){
      ram_ativa = ((valor & 0x0F) == 0x0A);
      return;
    }

    if(endereco < 0x4000){
      rom_bank = ((rom_bank & 0x60) | (valor & 0x1F));
      rom_bank %= total_banks;
      if(!(rom_bank & 0x1F))
        ++rom_bank;
      
      return;
    }
    
    if(endereco < 0x6000){
      if(ram_banking)
        ram_bank = valor & 0x03;
      else{
        rom_bank = ((rom_bank & 0x1F) | ((valor & 0x03) << 5));
        rom_bank %= total_banks;

        if(!(rom_bank & 0x1F))
          ++rom_bank;
  
      }

      return;
    }

    if(endereco < 0x8000){
      ram_banking = valor & 0x01;
      return;
    }
    
    if(ram_ativa && ram.size()){
      uint32_t address = (ram_bank*0x2000) + (endereco - 0xA000);
      if(address >= ram.size())
        address &= (ram.size() - 1);

      ram[address] = valor;
    }
  }

uint8_t& MBC2::read(uint16_t endereco){
  if(endereco < 0x4000){
    return rom[endereco];
  }
  if(endereco < 0x8000){
    uint32_t address = (rom_bank*0x4000) + (endereco - 0x4000);
    return rom[address];
  }
  
  if(endereco < 0xA000 || endereco >= 0xA200){
    ram_hack = 0xFF;
    return ram_hack;
  }

  uint32_t address = endereco - 0xA000;
  if(address >= ram.size())
        address &= (ram.size() - 1);

  ram_hack = (ram_ativa) ? (ram[address] | 0xF0) : 0xFF;
  return ram_hack;
}

void MBC2::write(uint16_t endereco, uint8_t valor){
  if(endereco < 0x4000){
    if(endereco & (1 << 8)){;
      rom_bank = (valor & 0x0F);
      rom_bank %= total_banks;
      if(!rom_bank)
        rom_bank = 1;
    }
    else{
      ram_ativa = ((valor & 0x0F) == 0x0A);
    }
    
    return;
  }
  
  if(endereco >= 0xA000 && endereco < 0xA200 && ram_ativa){
    uint32_t address = endereco - 0xA000;
    if(address >= ram.size())
        address &= (ram.size() - 1);

    ram[address] = valor & 0x0F;
  }
}

uint8_t& MBC3::read(uint16_t endereco){
    if(endereco < 0x4000){
      return rom[endereco];
    }
    if(endereco < 0x8000){
      uint32_t address = (rom_bank*0x4000) + (endereco - 0x4000);
      return rom[address];
    }
    if(!ram_ativa || !ram.size()){
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
    if(address >= ram.size())
        address &= (ram.size() - 1);

    return ram[address];
  }

void MBC3::write(uint16_t endereco, uint8_t valor){
    if(endereco < 0x2000){
      ram_ativa = ((valor & 0x0F) == 0x0A);
      return;
    }

    if(endereco < 0x4000){
      rom_bank = (rom.size() > 1024*2048) ? (valor & 0xFF) : (valor & 0x7F);
      rom_bank %= total_banks;
      if(!rom_bank)
        rom_bank = 1;
      
      return;
    }

    if(endereco < 0x6000){
      if(valor <= 3){
        ram_bank = valor ;
        rtc_selected = false;
        return;
      }
      else if(valor >= 8 && valor <= 12){
        rtc_selected = true;
        rtc_registrador = valor;
      }

      return;
    }

    if(endereco < 0x8000){
      if(valor == 0 && !latch_clock){
        latch_clock = true;
      }
      else if(latch_clock && valor == 1){
        rtc_latch = rtc;
        latch_clock = false;
      }
      else{
        latch_clock = false;
      }

      return;
    }

    if(ram_ativa && ram.size()){
      uint32_t address = (ram_bank*0x2000) + (endereco - 0xA000);
      if(address >= ram.size())
        address &= (ram.size() - 1);

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

void MBC3::save(void){
  std::filesystem::create_directories(saves);
  std::filesystem::path save_path = saves / std::filesystem::path(fonte).filename();
  save_path.replace_extension(".sav");

  std::ofstream arquivo(save_path, arquivo.binary | arquivo.trunc);
  if(!arquivo) return;

  arquivo.write(reinterpret_cast<char*>(ram.data()), ram.size());
  arquivo.write(reinterpret_cast<char*>(&rtc), sizeof(RTC));
  arquivo.write(reinterpret_cast<char*>(&rtc_latch), sizeof(RTC));
  arquivo.write(reinterpret_cast<char*>(&rtc_last), sizeof(rtc_last));
  std::cout << "Jogo salvo.\n";
}

void MBC3::load(void){
  std::filesystem::create_directories(saves);
  std::filesystem::path save_path = saves / std::filesystem::path(fonte).filename();
  save_path.replace_extension(".sav");

  std::fstream arquivo(save_path, arquivo.in | arquivo.binary);
  if(!arquivo) return;

  arquivo.read(reinterpret_cast<char*>(ram.data()), ram.size());
  arquivo.read(reinterpret_cast<char*>(&rtc), sizeof(RTC));
  arquivo.read(reinterpret_cast<char*>(&rtc_latch), sizeof(RTC));
  arquivo.read(reinterpret_cast<char*>(&rtc_last), sizeof(rtc_last));
  std::cout << "Save carregado.\n";
}

void MBC3::atualiza_rtc(void){
  if(rtc.f & 0x40) return;

  auto clock_atual = std::chrono::system_clock::now();
  auto seg_passados = std::chrono::duration_cast<std::chrono::seconds>(clock_atual - rtc_last).count();

  if(seg_passados < 1) return;
  rtc_last = clock_atual;

  uint64_t total_seg = rtc.s + seg_passados;
  uint64_t total_min = rtc.m + total_seg/60;
  uint64_t total_hour = rtc.h + total_min/60;
  uint32_t total_days = (((rtc.f & 0x01) << 8) | rtc.d) + total_hour/24;

  rtc.s = total_seg % 60;
  rtc.m = total_min % 60;
  rtc.h = total_hour % 24;
  rtc.d = total_days & 0xFF;
  rtc.f = (rtc.f & 0xC0) | ((total_days >> 8) & 0x01);

  if(total_days > 511)
    rtc.f |= 0x80;
}

uint8_t& MBC5::read(uint16_t endereco){
   if(endereco < 0x4000){
      return rom[endereco];
    }
    if(endereco < 0x8000){
      uint32_t address = (rom_bank*0x4000) + (endereco - 0x4000);
      return rom[address];
    }

    uint32_t address = (ram_bank*0x2000) + (endereco - 0xA000);
    ram_hack = 0xFF;
    if(address >= ram.size())
      address &= (ram.size() - 1);

    return (ram_ativa && ram.size()) ? ram[address] : ram_hack;
}

void MBC5::write(uint16_t endereco, uint8_t valor){
  if(endereco < 0x2000){
    ram_ativa = ((valor & 0x0F) == 0x0A);
    return;
  }
  if(endereco < 0x3000){
    rom_bank = (rom_bank & 0x100) | valor;
    rom_bank %= total_banks;
    return;
  }
  if(endereco < 0x4000){
    rom_bank = ((rom_bank & 0xFF) | ((valor & 0x01) << 8));
    rom_bank %= total_banks;
    return;
  }
  if(endereco < 0x6000){
    ram_bank = valor & 0x0F;
    return;
  }
  if(endereco >= 0xA000 && endereco < 0xC000){
     if(ram_ativa && ram.size()){
      uint32_t address = (ram_bank*0x2000) + (endereco - 0xA000);
      if(address >= ram.size())
        address &= (ram.size() - 1);

      ram[address] = valor;

      return;
    }
  }
}


}
