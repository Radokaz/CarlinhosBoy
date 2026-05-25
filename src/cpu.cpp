#include "actions.h"

namespace GB{

static uint16_t debug_cycles;
static uint16_t debug_tamanho;

void roda_cpu(CPU *atual, Timer *timer, PPU *ppu){
  if(atual->check_joypad() || (atual->get_if() & 0x1F)){
    atual->stepping = true;
  }
  if(!atual->stepping) 
    atual->stepping = true; //não dá pra emular essa porra
  atual->check();
  atual->step();
  /*std::cout << "Número de ciclos: " << debug_cycles << "\nInstrução: " << std::hex << static_cast<int>(atual->last_instruct) <<
    std::dec << "\n";*/
}

void roda_perifericos(CPU *atual, Timer *timer, PPU *ppu){
  timer->step(atual->bus);
  atual->bus.dma->step(atual->bus.memoria.data());
  ppu->step();
  ++debug_cycles;
}

void CPU::check(void){
    uint8_t Ie = this->get_ie();
    uint8_t If = this->get_if();
    if(Ie & If & 0x1F)
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

  if(prev & ~curr){
    this->get_if() |= BIT_JOYPAD;
    return true;
  }
  
  return false;
}


void CPU::step(){
  debug_cycles = 0;
  if(this->halted){
    roda_perifericos(this, this->bus.timer, this->bus.ppu);
    return;
  };

  if(this->ime_count > 0){
    this->ime_count = 0;
    this->ime = true;
  }

  debug_tamanho = 1;
  uint8_t inst_byte = this->bus.read_byte(this->pc);
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  if(inst_byte != 0xCB)
    this->last_instruct = inst_byte;

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
      std::terminate();*/
    }
    if(!skipa_fetch){
      this->incrementa_pc();
    }
  }
  catch(std::exception& ex){
    std::cerr << "Erro: " << ex.what();
    std::cerr << "Instrução: " << static_cast<int>(this->last_instruct) << "\n";
    std::terminate();
  }

  this->skipa_fetch = false;
}

void CPU::jump_vblank(void){
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  this->push(this->pc);
  this->pc = 0x0040;
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  this->ime = false;
  this->get_if() &= ~BIT_VBLANK;
  std::cout << "Interrupção: VBLANK\n"; 
}

void CPU::jump_serial(void){
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  this->push(this->pc);
  this->pc = 0x0058;
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  this->ime = false;
  this->get_if() &= ~BIT_SERIAL;
  std::cout << "Interrupção: SERIAL\n"; 
}

void CPU::jump_timer(void){
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  this->push(this->pc);
  this->pc = 0x0050;
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  this->ime = false;
  this->get_if() &= ~BIT_TIMER;
  std::cout << "Interrupção: TIMER\n"; 
}

void CPU::jump_lcdstat(void){
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  this->push(this->pc);
  this->pc = 0x0048;
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  this->ime = false;
  this->get_if() &= ~BIT_LCDSTAT;
  std::cout << "Interrupção: STAT\n"; 
}

void CPU::jump_joypad(void){
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  this->push(this->pc);
  this->pc = 0x0060;
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  this->ime = false;
  this->get_if() &= ~BIT_JOYPAD;
  std::cout << "Interrupção: JOYPAD\n"; 
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
      uint8_t& result = this->bus.read_byte(this->registradores.get_duplo(HL));
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return result;
    }
    case BC:{
      uint8_t& result = this->bus.read_byte(this->registradores.get_duplo(BC));
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return result;
    }
    case DE:{
      uint8_t& result = this->bus.read_byte(this->registradores.get_duplo(DE));
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return result;
    }
    case HLI:{
      uint16_t temp = this->registradores.get_duplo(HL);
      uint8_t& result = this->bus.read_byte(temp);
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      this->registradores.set_duplo(HL, temp + 1);
      return result;
    }
    case HLD:{
      uint16_t temp = this->registradores.get_duplo(HL);
      uint8_t& result = this->bus.read_byte(temp);
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      this->registradores.set_duplo(HL, temp - 1);
      return result;
    }
    case A8:{
      this->incrementa_pc();
      uint16_t byte = static_cast<uint16_t>(this->bus.read_byte(this->pc));
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      uint8_t& result = this->bus.read_byte(0xFF00 + byte);
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
      uint8_t& result = this->bus.read_byte(lower | upper);
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return result;
    }
    case n:{
      this->incrementa_pc();
      uint8_t& result = this->bus.read_byte(this->pc);
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return result;
    }
    case CPTR:{
      uint8_t& result = this->bus.read_byte(0xFF00 + static_cast<uint16_t>(this->registradores.c));
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      return result;
    }
    default:
      throw std::runtime_error("Registrador invalido.\n");
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
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      this->registradores.set_duplo(HL, temp + 1);
      return result;
    }
    case HLD:{
      uint16_t temp = this->registradores.get_duplo(HL);
      uint8_t result = this->bus.read_byte(temp);
      roda_perifericos(this, this->bus.timer, this->bus.ppu);
      this->registradores.set_duplo(HL, temp - 1);
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
  }
  else
    this->haltbug = false;

  ++debug_tamanho;
}

}

