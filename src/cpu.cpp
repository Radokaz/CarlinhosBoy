#include "actions.h"

namespace GB{

void roda_cpu(CPU *atual, Timer& timer){
  if(atual->check_joypad() || (atual->get_if() & 0x1F)){
    atual->stepping = true;
  }
  if(!atual->stepping) return;
  atual->check();
  atual->step();
  timer.step(atual->last_ticks, atual->bus);
  for(size_t i {}; i < atual->last_ticks; i+=4)
      atual->bus.dma->step(atual->bus.memoria.data());
}

void CPU::check(void){
    uint8_t Ie = this->get_ie();
    uint8_t If = this->get_if();
    if(Ie & If)
      this->halted = false;

    if(this->ime){
      if(Ie & If & BIT_VBLANK){
        this->jump_vblank();
      }
      else if(Ie & If & BIT_LCDSTAT){
        this->jump_lcdstat();
      }
      else if(Ie & If & BIT_TIMER){
        this->jump_timer();
      }
      else if(Ie & If & BIT_SERIAL){
        this->jump_serial();
      }
      else if(Ie & If & BIT_JOYPAD){
        this->jump_joypad();
      }
    }
}

bool CPU::check_joypad(void){
  if((this->bus.memoria[0xFF00] & 0x30) == 0x30) return false;
  uint8_t prev = this->bus.pad->controles_prev;
  uint8_t curr = this->bus.pad->controles;

  if(prev & ~curr & 0x0F){
    this->get_if() |= BIT_JOYPAD;
    return true;
  }
  
  return false;
}


void CPU::step(){
  if(this->halted){
    this->last_ticks = 4;
    return;
  };

  bool set_ime {false};
  if(this->ime_ie){
    set_ime = true;
    this->ime_ie = false;
  }

  uint8_t inst_byte = this->bus.read_byte(this->pc);

  try{
    auto current_act = le_byte(inst_byte, this);
    current_act.execute(current_act, this);
    std::cout << "Last inst: " << std::hex << static_cast<int>(inst_byte) << std::dec << "\n";
    if(current_act.execute == &GBInstruct::DI)
      set_ime = false;

    if(!this->jp_flag){
      if(!this->haltbug)
        this->pc+=current_act.tamanho;
      this->haltbug = false;
    }
  }
  catch(std::exception& ex){
    std::cerr << "Erro: " << ex.what() << "\n";
    this->pc++;
  }

  this->jp_flag = false;
  if(set_ime)
    this->ime = true;
}

void CPU::jump_vblank(void){
  this->push(this->pc);
  this->pc = 0x0040;
  this->ime = 0;
  this->get_if() &= ~BIT_VBLANK;
}

void CPU::jump_serial(void){
  this->push(this->pc);
  this->pc = 0x0058;
  this->ime = 0;
  this->get_if() &= ~BIT_SERIAL;
}

void CPU::jump_timer(void){
  this->push(this->pc);
  this->pc = 0x0050;
  this->ime = 0;
  this->get_if() &= ~BIT_TIMER;
}

void CPU::jump_lcdstat(void){
  this->push(this->pc);
  this->pc = 0x0048;
  this->ime = 0;
  this->get_if() &= ~BIT_LCDSTAT;
}

void CPU::jump_joypad(void){
  this->push(this->pc);
  this->pc = 0x0060;
  this->ime = 0;
  this->get_if() &= ~BIT_JOYPAD;
}

uint8_t& CPU::get_target(reg_target alvo){
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
    case HL:
      return this->bus.read_byte(this->registradores.get_duplo(HL));
    case BC:
      return this->bus.read_byte(this->registradores.get_duplo(BC));
    case DE:
      return this->bus.read_byte(this->registradores.get_duplo(DE));
    case HLI:{
      uint16_t temp = this->registradores.get_duplo(HL);
      this->registradores.set_duplo(HL, temp + 1);
      return this->bus.read_byte(temp);
    }
    case HLD:{
      uint16_t temp = this->registradores.get_duplo(HL);
      this->registradores.set_duplo(HL, temp - 1);
      return this->bus.read_byte(temp);
    }
    case A8:
      return this->bus.read_byte(0xFF00 + static_cast<uint16_t>(this->bus.read_byte(this->pc + 1)));
    case A16:{
      uint16_t lower = static_cast<uint16_t>(this->bus.read_byte(this->pc + 1));
      uint16_t upper = (static_cast<uint16_t>(this->bus.read_byte(this->pc + 2)) << 8);
      return this->bus.read_byte(lower | upper);
    }
    case n:
      return this->bus.read_byte(this->pc + 1);
    case CPTR:
      return this->bus.read_byte(0xFF00 + static_cast<uint16_t>(this->registradores.c));
    default:
      throw std::runtime_error("Registrador invalido.\n");
  }
}

uint16_t CPU::get_target_duplo(reg_target alvo){
    if(alvo == reg_target::SP)
      return this->sp;
    if(alvo == reg_target::n){
      uint16_t lower = static_cast<uint16_t>(this->bus.read_byte(this->pc + 1));
      uint16_t upper = (static_cast<uint16_t>(this->bus.read_byte(this->pc + 2)) << 8);
      return (lower | upper);
    }
    return this->registradores.get_duplo(alvo);
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
    case HL:
      return ((this->bus.read_byte(this->registradores.get_duplo(HL)) & (1 << bit)) > 0) ? 1 : 0;
    default:
      throw std::runtime_error("Registrador inválido.\n");
  }
}

}

