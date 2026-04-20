#include "actions.h"

namespace GB{

void roda_cpu(CPU *atual){
  atual->step();
  atual->check();
}

void CPU::check(void){
  uint8_t Ie = this->get_ie();
  uint8_t If = this->get_if();

  if(this->ime){
    this->push(this->pc);
    this->ime = false;
    //TODO
  }
}

void CPU::step(void){
  if(this->halted || this->stopped) return;

  bool set_ime {false};
  if(this->ime_ie){
    set_ime = true;
    this->ime_ie = false;
  }

  uint8_t inst_byte = this->bus.read_byte(this->pc);

  try{
  auto current_act = le_byte(inst_byte, this);
  current_act.execute(current_act, this);

  if(!this->jp_flag)
    this->pc+=current_act.tamanho;
  }
  catch(std::exception& ex){
    std::cerr << "Erro: " << ex.what() << "\n";
    this->pc++;
  }
  this->jp_flag = false;
  if(set_ime)
    this->ime = true;

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

uint16_t CPU::get_target_duplo(reg_target alvo) const{
    if(alvo == reg_target::SP)
      return this->sp;
    if(alvo == reg_target::n){
      uint16_t lower = static_cast<uint16_t>(this->bus.read_byte_const(this->pc + 1));
      uint16_t upper = (static_cast<uint16_t>(this->bus.read_byte_const(this->pc + 2)) << 8);
      return (lower | upper);
    }
    return this->registradores.get_duplo(alvo);
}

uint8_t CPU::get_bit(reg_target alvo, uint8_t bit) const{
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
    default:
      throw std::runtime_error("Registrador inválido.\n");
  }
}

}

