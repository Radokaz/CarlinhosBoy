#include "cpu.h"

void CPU::push(reg_target alvo){
  uint16_t valor = this->registradores.get_duplo(alvo);
  --this->sp;
  this->bus.read_byte(this->sp) = static_cast<uint8_t>(((valor & 0xFF00) >> 8));
  --this->sp;
  this->bus.read_byte(this->sp) = static_cast<uint8_t>((valor & 0xFF));
}

void CPU::pop(reg_target alvo){
  uint16_t lower = static_cast<uint16_t>(this->bus.read_byte(this->sp));
  uint16_t upper = (static_cast<uint16_t>(this->bus.read_byte(this->sp + 1)) << 8);
  this->registradores.set_duplo(alvo, (lower | upper));
  this->sp+=2;
}
