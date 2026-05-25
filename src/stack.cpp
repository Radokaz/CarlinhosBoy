#include "cpu.h"

namespace GB{

void CPU::push(reg_target alvo){
  uint16_t valor = this->get_target_duplo(alvo);
  --this->sp;
  this->bus.write_byte(this->sp, static_cast<uint8_t>(((valor & 0xFF00) >> 8)));
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  --this->sp;
  this->bus.write_byte(this->sp, static_cast<uint8_t>((valor & 0xFF)));
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
}

void CPU::pop(reg_target alvo){
  uint16_t lower = static_cast<uint16_t>(this->bus.read_byte(this->sp));
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  ++this->sp;
  uint16_t upper = (static_cast<uint16_t>(this->bus.read_byte(this->sp)) << 8);
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  ++this->sp;
  uint16_t valor = (lower | upper);

  if(alvo == reg_target::AF)
    valor &= 0xFFF0;

  this->registradores.set_duplo(alvo, valor);
}

void CPU::push(uint16_t valor){
  --this->sp;
  this->bus.write_byte(this->sp, static_cast<uint8_t>(((valor & 0xFF00) >> 8)));
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  --this->sp;
  this->bus.write_byte(this->sp, static_cast<uint8_t>((valor & 0xFF)));
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
}

uint16_t CPU::pop(void){
  uint16_t lower = static_cast<uint16_t>(this->bus.read_byte(this->sp));
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  ++this->sp;
  uint16_t upper = (static_cast<uint16_t>(this->bus.read_byte(this->sp)) << 8);
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
  ++this->sp;
  uint16_t valor = (lower | upper);
  return valor;
}

void CPU::call(uint16_t endereco){
  ++this->pc;
  this->push(this->pc);
  this->pc = endereco;
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
}

void CPU::ret(void){
  this->pc = this->pop();
  roda_perifericos(this, this->bus.timer, this->bus.ppu);
}

}
