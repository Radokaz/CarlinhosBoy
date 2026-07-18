#include "actions.h"

namespace GB{

//static uint16_t debug_cycles;
//static uint16_t debug_tamanho;

void roda_cpu(CPU *atual){
  if(atual->check_joypad() || (atual->get_if() & 0x1F)){
    atual->stepping = true;
  }
  if(!atual->stepping) 
    return;

  if(atual->pausa_ciclos > 0){
    atual->bus.ppu->step();
    atual->bus.timer->apu->step(atual->modo);
    atual->pausa_ciclos-=atual->pausa_offset;

    if(atual->pausa_ciclos <= 0){
      atual->pausa_ciclos = 0;
      atual->pausa_offset = 0;
      atual->bus.ppu->speed_bug = 0;
      atual->bus.memoria[0xFF4D] &= ~0x01;
      atual->bus.memoria[0xFF4D] ^= 0x80;
    }

    return;
  }

  if(atual->bus.hdma.ativo && !atual->halted){
    atual->bus.hdma.step(&atual->bus);
    roda_perifericos(atual, atual->bus.timer, atual->bus.ppu);
    return;
  }

  atual->check();
  atual->step();
}

void roda_perifericos(CPU *atual, Timer *timer, PPU *ppu){
  if(atual->bus.serial_count){
    --atual->bus.serial_count;
    if(!atual->bus.serial_count){
      atual->bus.memoria[0xFF01] = 0xFF;
      atual->bus.memoria[0xFF02] &= ~0x80;
      atual->get_if() |= BIT_SERIAL;
    }
  }
  timer->step(atual->bus);
  atual->bus.dma.step(&atual->bus);

  if(ppu->stat_bug){
    --ppu->stat_bug;
    if(!ppu->stat_bug){
      atual->bus.memoria[0xFF41] = (ppu->stat_cache & 0x78) | (atual->bus.memoria[0xFF41] & 0x07);
      ppu->stat_cache = 0;
    }
  }
  ppu->step();
  timer->apu->step(atual->modo);
  //++debug_cycles;
}

void CPU::check(void){
    uint8_t Ie = this->get_ie();
    uint8_t If = this->get_if();
    if(Ie & If & 0x1F)
      this->halted = false;
    else
      return;

    if(this->ime && !this->bus.hdma.ativo){
      this->jump_interruption();
    }
}

void CPU::jump_interruption(void){
  this->ime = false;
  uint8_t Ie = this->get_ie();
  uint8_t If = this->get_if();

  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  uint16_t end_inicial {0x70};
  uint16_t end_final {0x70};
  uint8_t bit {};

  if(Ie & If & BIT_VBLANK){
    this->get_if() &= ~BIT_VBLANK;
    end_inicial = 0x40;
  }
  else if(Ie & If & BIT_LCDSTAT){
    this->get_if() &= ~BIT_LCDSTAT;
    end_inicial = 0x48;
  }
  else if(Ie & If & BIT_TIMER){
    this->get_if() &= ~BIT_TIMER;
    end_inicial = 0x50;
  }
  else if(Ie & If & BIT_SERIAL){
    this->get_if() &= ~BIT_SERIAL;
    end_inicial = 0x58;
  }
  else if(Ie & If & BIT_JOYPAD){
    this->get_if() &= ~BIT_JOYPAD;
    end_inicial = 0x60;
  }

  this->push(this->pc);

  If = this->get_if();
  Ie = this->get_ie();

  if(Ie & If & BIT_VBLANK){
    bit = BIT_VBLANK;
    end_final = 0x40;
  }
  else if(Ie & If & BIT_LCDSTAT){
    bit = BIT_LCDSTAT;
    end_final = 0x48;
  }
  else if(Ie & If & BIT_TIMER){
    bit = BIT_TIMER;
    end_final = 0x50;
  }
  else if(Ie & If & BIT_SERIAL){
    bit = BIT_SERIAL;
    end_final = 0x58;
  }
  else if(Ie & If & BIT_JOYPAD){
    bit = BIT_JOYPAD;
    end_final = 0x60;
  }

  if(end_inicial < end_final){
    this->pc = end_inicial;
  }
  else{
    this->pc = end_final;
    this->get_if() &= ~bit;
  }

  roda_perifericos(this, this->bus.timer, this->bus.ppu);
}

bool CPU::check_joypad(void){
  uint8_t prev = this->bus.pad->controles_prev;
  uint8_t curr = this->bus.pad->controles;

  if(~prev & curr){
    this->bus.pad->controles_prev = curr;
    this->get_if() |= BIT_JOYPAD;
    return true;
  }
  
  return false;
}


void CPU::step(){
  //debug_cycles = 0;
  if(this->bus.hdma.ativo && !this->halted) return;
  if(this->halted){
    roda_perifericos(this, this->bus.timer, this->bus.ppu);
    return;
  };

  if(this->ime_count > 0){
    this->ime_count = 0;
    this->ime = true;
  }

  //debug_tamanho = 1;
  uint8_t inst_byte = this->bus.read_byte(this->pc);
  if(inst_byte != 0xCB){
    this->last_instruct = inst_byte;
  }
  this->bus.ppu->check_oam(this->pc, oam_corruption::READ);
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  
  try{
    auto current_act = le_byte(inst_byte, this);
    current_act.execute(current_act, this);

    /*if(debug_tamanho != current_act.tamanho){
      std::cerr << std::dec << "Tamanho esperado: " << static_cast<int>(current_act.tamanho) << ", Tamanho obtido: " << debug_tamanho << "\n";
      std::cerr << "Instrução: " << std::hex << static_cast<int>(this->last_instruct) << "\n"; 
      std::terminate();
    }
    if(debug_cycles != this->ciclos_esperados){
      std::cerr << std::dec << "Ciclos esperados: " << static_cast<int>(ciclos_esperados) << ", Ciclos obtidos: " << debug_cycles << "\n";
      std::cerr << "Instrução: " << std::hex << static_cast<int>(this->last_instruct) << "\n"; 
      std::terminate();
    }*/
    if(!skipa_fetch){
      this->incrementa_pc();
    }

    //std::cout << "Ultima instrução: " << std::hex << static_cast<int>(this->last_instruct) << std::dec << "\n";
  }
  catch(std::exception& ex){
    std::cerr << "Erro: " << ex.what();
    std::cerr << "Instrução: " << std::hex << static_cast<int>(this->last_instruct) << "\n";
    std::terminate();
  }

  this->skipa_fetch = false;
}

uint8_t& CPU::get_target_ref(reg_target alvo){
  switch(alvo){
    using enum reg_target;

    case A:
      return this->registradores.a;
    case B:
      return this->registradores.b;
    case C:
      return this->registradores.c;
    case D:
      return this->registradores.d;
    case E:
      return this->registradores.e;
    case F:
      return this->registradores.f;
    case H:
      return this->registradores.h;
    case L:
      return this->registradores.l;
    case HL:{
      this->hack = this->bus.read_byte(this->registradores.get_duplo(HL));
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return this->hack;
    }
    default:
      throw std::runtime_error("Registrador invalido. Reg_target_ref\n");
  }
}

uint8_t CPU::get_target_value(reg_target alvo){
  switch(alvo){
    using enum reg_target;

    case A:
      return this->registradores.a;
    case B:
      return this->registradores.b;
    case C:
      return this->registradores.c;
    case D:
      return this->registradores.d;
    case E:
      return this->registradores.e;
    case F:
      return this->registradores.f;
    case H:
      return this->registradores.h;
    case L:
      return this->registradores.l;
    case HL:{
      uint8_t result = this->bus.read_byte(this->registradores.get_duplo(HL));
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return result;
    }
    case BC:{
      uint8_t result = this->bus.read_byte(this->registradores.get_duplo(BC));
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return result;
    }
    case DE:{
      uint8_t result = this->bus.read_byte(this->registradores.get_duplo(DE));
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return result;
    }
    case HLI:{
      uint16_t temp = this->registradores.get_duplo(HL);
      uint8_t result = this->bus.read_byte(temp);
      this->registradores.set_duplo(HL, temp + 1);
      this->bus.ppu->check_oam(temp, oam_corruption::READ_WRITE);
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return result;
    }
    case HLD:{
      uint16_t temp = this->registradores.get_duplo(HL);
      uint8_t result = this->bus.read_byte(temp);
      this->registradores.set_duplo(HL, temp - 1);
      this->bus.ppu->check_oam(temp, oam_corruption::READ_WRITE);
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return result;
    }
    case A8:{
      this->incrementa_pc();
      uint16_t byte = static_cast<uint16_t>(this->bus.read_byte(this->pc));
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      uint8_t result = this->bus.read_byte(0xFF00 + byte);
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return result;
    }
    case A16:{
      this->incrementa_pc();
      uint16_t lower = static_cast<uint16_t>(this->bus.read_byte(this->pc));
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      this->incrementa_pc();
      uint16_t upper = (static_cast<uint16_t>(this->bus.read_byte(this->pc)) << 8);
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      uint8_t result = this->bus.read_byte(lower | upper);
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return result;
    }
    case n:{
      this->incrementa_pc();
      uint8_t result = this->bus.read_byte(this->pc);
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return result;
    }
    case CPTR:{
      uint8_t result = this->bus.read_byte(0xFF00 + static_cast<uint16_t>(this->registradores.c));
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return result;
    }
    default:
      throw std::runtime_error("Registrador invalido.\n");
  }
}

uint16_t CPU::get_target_duplo(reg_target alvo){
    if(alvo == reg_target::SP)
      return this->sp;
    if(alvo == reg_target::n){
      this->incrementa_pc();
      uint16_t lower = static_cast<uint16_t>(this->bus.read_byte(this->pc));
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      this->incrementa_pc();
      uint16_t upper = (static_cast<uint16_t>(this->bus.read_byte(this->pc)) << 8);
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return (lower | upper);
    }

    uint16_t result = this->registradores.get_duplo(alvo);
    return result;
}

uint8_t CPU::get_bit(reg_target alvo, uint8_t bit){
  switch(alvo){
    using enum reg_target;
    case A:
      return ((this->registradores.a & (1 << bit)) > 0) ? 1 : 0;
    case B:
      return ((this->registradores.b & (1 << bit)) > 0) ? 1 : 0;
    case C:
      return ((this->registradores.c & (1 << bit)) > 0) ? 1 : 0;
    case D:
      return ((this->registradores.d & (1 << bit)) > 0) ? 1 : 0;
    case E:
      return ((this->registradores.e & (1 << bit)) > 0) ? 1 : 0;
    case F:
      return ((this->registradores.f & (1 << bit)) > 0) ? 1 : 0;
    case H:
      return ((this->registradores.h & (1 << bit)) > 0) ? 1 : 0;
    case L:
      return ((this->registradores.l & (1 << bit)) > 0) ? 1 : 0;
    case HL:{
      uint8_t result = this->bus.read_byte(this->registradores.get_duplo(HL));
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return ((result & (1 << bit)) > 0) ? 1 : 0;
    }
    default:
      throw std::runtime_error("Registrador inválido.\n");
  }
}

void CPU::incrementa_pc(void){
  if(!this->haltbug){
    ++this->pc;
    this->bus.ppu->check_oam(this->pc - 1, oam_corruption::WRITE);
  }
  else
    this->haltbug = false;

  //++debug_tamanho;
}

}

