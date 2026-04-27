#include "cpu.h"

namespace GB{

void CPU::push(reg_target alvo){
  uint16_t valor = this->registradores.get_duplo(alvo);
  --this->sp;
  this->bus.write_byte(this->sp, static_cast<uint8_t>(((valor & 0xFF00) >> 8)));
  --this->sp;
  this->bus.write_byte(this->sp, static_cast<uint8_t>((valor & 0xFF)));
}

void CPU::pop(reg_target alvo){
  uint16_t lower = static_cast<uint16_t>(this->bus.read_byte(this->sp));
  uint16_t upper = (static_cast<uint16_t>(this->bus.read_byte(this->sp + 1)) << 8);
  uint16_t valor = (lower | upper);

  if(alvo == reg_target::AF)
    valor &= 0xFFF0;

  this->registradores.set_duplo(alvo, valor);
  this->sp+=2;
}

void CPU::push(uint16_t valor){
  --this->sp;
  this->bus.write_byte(this->sp, static_cast<uint8_t>(((valor & 0xFF00) >> 8)));
  --this->sp;
  this->bus.write_byte(this->sp, static_cast<uint8_t>((valor & 0xFF)));
}

uint16_t CPU::pop(void){
  uint16_t lower = static_cast<uint16_t>(this->bus.read_byte(this->sp));
  uint16_t upper = (static_cast<uint16_t>(this->bus.read_byte(this->sp + 1)) << 8);
  uint16_t valor = (lower | upper);
  this->sp+=2;
  return valor;
}

void CPU::call(uint16_t endereco){
  uint16_t pc_prox = (!this->haltbug) ? this->pc + 3 : this->pc + 2;
  this->push(pc_prox);
  this->pc = endereco;
}

void CPU::ret(void){
  this->pc = this->pop();
}

}
