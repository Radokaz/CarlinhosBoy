#include "cpu.h"

void CPU::step(void){
  uint8_t inst_byte = this->bus.read_byte(this->pc);
  auto current_act = le_byte(inst_byte, this);
  
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
    default:
      throw std::runtime_error("Registrador invalido.\n");
  }
}

uint16_t CPU::get_target_duplo(reg_target alvo) const{
    if(alvo == reg_target::SP)
      return this->sp;
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

void CPU::execute(const Action& atual){
  
  switch(atual.instrucao){
    using enum Instrucoes;

    case ADD: {
      uint8_t valor = (atual.alvo == reg_target::n) ? atual.N : this->get_target(atual.alvo);
      this->registradores.f = 0;
      uint32_t result = static_cast<uint32_t>(this->registradores.a) + static_cast<uint32_t>(valor);

      if(result > 0xFF)
        this->registradores.f |= BIT_CARRY;
      if((result & 0xFF) == 0)
        this->registradores.f |= BIT_ZERO;
      if((this->registradores.a & 0x0F) + (valor & 0x0F) > 0x0F)
        this->registradores.f |= BIT_HALFCARRY;
      
      this->registradores.a = static_cast<uint8_t>(result & 0xFF);
      break;
    }
    case ADDHL: {
      uint16_t valor = this->get_target_duplo(atual.alvo);
      uint32_t result = static_cast<uint32_t>(valor) + static_cast<uint32_t>(this->registradores.get_duplo(reg_target::HL));

      this->registradores.f &= ~(BIT_HALFCARRY | BIT_CARRY | BIT_SUBTRACT);
      if((this->registradores.get_duplo(reg_target::HL) & 0x0FFF) + (valor & 0x0FFF) > 0x0FFF)
        this->registradores.f |= BIT_HALFCARRY;
      if(result > 0xFFFF)
        this->registradores.f |= BIT_CARRY;

      this->registradores.set_duplo(reg_target::HL, static_cast<uint16_t>(result & 0xFFFF));
      break;
    }
    case ADC: {
      uint8_t valor = (atual.alvo == reg_target::n) ? atual.N : this->get_target(atual.alvo);
      uint32_t carry = ((this->registradores.f & BIT_CARRY) > 0) ? 1 : 0;

      uint32_t result = static_cast<uint32_t>(this->registradores.a) + static_cast<uint32_t>(valor) + carry;
      this->registradores.f = 0;

      if(result > 0xFF)
        this->registradores.f |= BIT_CARRY;
      if((result & 0xFF) == 0)
        this->registradores.f |= BIT_ZERO;
      if((this->registradores.a & 0x0F) + (valor & 0x0F) + (carry & 0x0F) > 0x0F)
        this->registradores.f |= BIT_HALFCARRY;
      
      this->registradores.a = static_cast<uint8_t>(result & 0xFF);
      break;
    }
    case SUB: {
      uint8_t valor = (atual.alvo == reg_target::n) ? atual.N : this->get_target(atual.alvo);
      uint32_t result = static_cast<uint32_t>(this->registradores.a) - static_cast<uint32_t>(valor);
      
      this->registradores.f = BIT_SUBTRACT;

      if((result & 0xFF) == 0)
        this->registradores.f |= BIT_ZERO;
      if((this->registradores.a & 0x0F) < (valor & 0x0F))
        this->registradores.f |= BIT_HALFCARRY;
      if((this->registradores.a & 0xFF) < (valor & 0xFF))
        this->registradores.f |= BIT_CARRY;

      this->registradores.a = static_cast<uint8_t>(result & 0xFF);
      break;
    }
    case SBC: {
      uint8_t valor = (atual.alvo == reg_target::n) ? atual.N : this->get_target(atual.alvo);
      uint32_t carry = ((this->registradores.f & BIT_CARRY) > 0) ? 1 : 0;
      uint32_t result = static_cast<uint32_t>(this->registradores.a) - static_cast<uint32_t>(valor) - carry;
      
      this->registradores.f = BIT_SUBTRACT;

      if((result & 0xFF) == 0)
        this->registradores.f |= BIT_ZERO;
      if((this->registradores.a & 0x0F) < ((valor & 0x0F) + carry))
        this->registradores.f |= BIT_HALFCARRY;
      if((this->registradores.a & 0xFF) < ((valor & 0xFF) + carry))
        this->registradores.f |= BIT_CARRY;

      this->registradores.a = static_cast<uint8_t>(result & 0xFF);
      break;
    }
    case AND: {
      uint8_t valor = (atual.alvo == reg_target::n) ? atual.N : this->get_target(atual.alvo);
      uint8_t result = (this->registradores.a & valor);
      this->registradores.f = BIT_HALFCARRY;

      if(result == 0)
        this->registradores.f |= BIT_ZERO;
      
      this->registradores.a = result;
      break;
    }
    case OR: {
      uint8_t valor = (atual.alvo == reg_target::n) ? atual.N : this->get_target(atual.alvo);
      uint8_t result = (this->registradores.a | valor);
      this->registradores.f = 0;

      if(result == 0)
        this->registradores.f |= BIT_ZERO;
      
      this->registradores.a = result;
      break;
    }
    case XOR: {
      uint8_t valor = (atual.alvo == reg_target::n) ? atual.N : this->get_target(atual.alvo);
      uint8_t result = (this->registradores.a ^ valor);
      this->registradores.f = 0;

      if(result == 0)
        this->registradores.f |= BIT_ZERO;
      
      this->registradores.a = result;
      break;
    }
    case CP: {
      uint8_t valor = (atual.alvo == reg_target::n) ? atual.N : this->get_target(atual.alvo);
      uint32_t result = static_cast<uint32_t>(this->registradores.a) - static_cast<uint32_t>(valor);
      
      this->registradores.f = BIT_SUBTRACT;

      if((result & 0xFF) == 0)
        this->registradores.f |= BIT_ZERO;
      if((this->registradores.a & 0x0F) < (valor & 0x0F))
        this->registradores.f |= BIT_HALFCARRY;
      if((this->registradores.a & 0xFF) < (valor & 0xFF))
        this->registradores.f |= BIT_CARRY;

      break;
    }
    case INC: {
      uint8_t& reg = this->get_target(atual.alvo);
      uint32_t result = static_cast<uint32_t>(reg) + 1;

      this->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if((result & 0xFF) == 0)
        this->registradores.f |= BIT_ZERO;
      if((reg & 0x0F) + 0x01 > 0x0F)
        this->registradores.f |= BIT_HALFCARRY;
      
      ++reg;
      break;
    }
    case INCDUP: {
      uint16_t reg = this->get_target_duplo(atual.alvo);
            
      if(atual.alvo != reg_target::SP)
        this->registradores.set_duplo(atual.alvo, reg + 1);
      else
        this->sp++;
      break;
    }
    case DEC: {
      uint8_t& reg = this->get_target(atual.alvo);
      uint32_t result = static_cast<uint32_t>(reg) - 1;

      this->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY);
      this->registradores.f |= BIT_SUBTRACT;
      if((result & 0xFF) == 0)
        this->registradores.f |= BIT_ZERO;
      if((reg & 0x0F) == 0)
        this->registradores.f |= BIT_HALFCARRY;
      
      --reg;
      break;
    }
    case DECDUP: {
      uint16_t reg = this->get_target_duplo(atual.alvo);
            
      if(atual.alvo != reg_target::SP)
        this->registradores.set_duplo(atual.alvo, reg - 1);
      else
        this->sp--;

      break;
    }
    case CCF: {
      this->registradores.f ^= BIT_CARRY;
      this->registradores.f &= ~(BIT_SUBTRACT | BIT_HALFCARRY);
      break;
    }
    case SCF: {
      this->registradores.f |= BIT_CARRY;
      this->registradores.f &= ~(BIT_SUBTRACT | BIT_HALFCARRY);
      break;
    }
    case RRA: {
      uint8_t bit0 = (this->registradores.a & 0x01);
      uint8_t carry = ((this->registradores.f & BIT_CARRY) > 0) ? 1 : 0;

      this->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit0)
        this->registradores.f |= BIT_CARRY;
      else
        this->registradores.f &= ~BIT_CARRY;

      this->registradores.a = ((this->registradores.a >> 1) | (carry << 7));
      break;
    }
    case RLA: {
      uint8_t bit7 = (this->registradores.a & (1 << 7));
      uint8_t carry = ((this->registradores.f & BIT_CARRY) > 0) ? 1 : 0;

      this->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit7)
        this->registradores.f |= BIT_CARRY;
      else
        this->registradores.f &= ~BIT_CARRY;

      this->registradores.a = ((this->registradores.a << 1) | carry);
      break;
    }
    case RRCA: {
      uint8_t bit0 = (this->registradores.a & 0x01);
      this->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit0)
        this->registradores.f |= BIT_CARRY;
      else
        this->registradores.f &= ~BIT_CARRY;

      this->registradores.a = ((this->registradores.a >> 1) | (bit0 << 7));
      break;
    } 
    case RLCA: {
      uint8_t bit7 = ((this->registradores.a & (1 << 7)) > 0) ? 1 : 0;
      this->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit7)
        this->registradores.f |= BIT_CARRY;
      else
        this->registradores.f &= ~BIT_CARRY;

      this->registradores.a = ((this->registradores.a << 1) | bit7);
      break;
    } 
    case CPL: {
      this->registradores.a = ~this->registradores.a;
      this->registradores.f |= (BIT_SUBTRACT | BIT_HALFCARRY);
      break;
    }
    case BIT: {
      uint8_t bit = this->get_bit(atual.alvo, atual.bit_index);
      this->registradores.f &= ~(BIT_ZERO | BIT_SUBTRACT);
      this->registradores.f |= BIT_HALFCARRY;
      if(!bit)
        this->registradores.f |= BIT_ZERO;
      
      break;
    }
    case RESET: {
      uint8_t& reg = this->get_target(atual.alvo);
      reg &= ~(1 << atual.bit_index);

      break;
    }
    case SET: {
      uint8_t& reg = this->get_target(atual.alvo);
      reg |= (1 << atual.bit_index);

      break;
    }
    case SRL: {
      uint8_t& reg = this->get_target(atual.alvo);
      uint8_t bit0 = (reg & 0x01);
      reg = (reg >> 1);
      this->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);

      if(bit0)
        this->registradores.f |= BIT_CARRY;
      else
        this->registradores.f &= ~BIT_CARRY;

      if(!reg)
        this->registradores.f |= BIT_ZERO;

      break;
    }
    case RR: {
      uint8_t& reg = this->get_target(atual.alvo);
      uint8_t bit0 = (reg & 0x01);
      uint8_t carry = ((this->registradores.f & BIT_CARRY) > 0) ? 1 : 0;

      this->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit0)
        this->registradores.f |= BIT_CARRY;
      else
        this->registradores.f &= ~BIT_CARRY;

      reg = ((reg >> 1) | (carry << 7));
      if(!reg)
        this->registradores.f |= BIT_ZERO;

      break;
    }
    case RL: {
      uint8_t& reg = this->get_target(atual.alvo);
      uint8_t bit7 = (reg & (1 << 7));
      uint8_t carry = ((this->registradores.f & BIT_CARRY) > 0) ? 1 : 0;

      this->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit7)
        this->registradores.f |= BIT_CARRY;
      else
        this->registradores.f &= ~BIT_CARRY;

      reg = ((reg << 1) | carry);
      if(!reg)
        this->registradores.f |= BIT_ZERO;

      break;
    }
    case RRC: {
      uint8_t& reg = this->get_target(atual.alvo);
      uint8_t bit0 = (reg & 0x01);
      this->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit0)
        this->registradores.f |= BIT_CARRY;
      else
        this->registradores.f &= ~BIT_CARRY;

      reg = ((reg >> 1) | (bit0 << 7));
      if(!reg)
        this->registradores.f |= BIT_ZERO;

      break;
    }
    case RLC: {
      uint8_t& reg = this->get_target(atual.alvo);
      uint8_t bit7 = ((reg & (1 << 7)) > 0) ? 1 : 0;
      this->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit7)
        this->registradores.f |= BIT_CARRY;
      else
        this->registradores.f &= ~BIT_CARRY;

      reg = ((reg << 1) | bit7);
      if(!reg)
        this->registradores.f |= BIT_ZERO;

      break;
    }
    case SRA: {
      uint8_t& reg = this->get_target(atual.alvo);
      uint8_t bit7 = ((reg & (1 << 7)) > 0) ? 1 : 0;
      uint8_t carry = (reg & 0x01);
      this->registradores.f = 0;

      if(carry)
        this->registradores.f |= BIT_CARRY;
      else
        this->registradores.f &= ~BIT_CARRY;

      reg = ((reg >> 1) | (bit7 << 7));
      if(!reg)
        this->registradores.f |= BIT_ZERO;

      break;
    }
    case SLA: {
      uint8_t& reg = this->get_target(atual.alvo);
      uint8_t carry = (reg & (1 << 7));
      this->registradores.f = 0;

      if(carry)
        this->registradores.f |= BIT_CARRY;
      else
        this->registradores.f &= ~BIT_CARRY;

      reg = (reg << 1);
      if(!reg)
        this->registradores.f |= BIT_ZERO;

      break;
    }
    case SWAP: {
      uint8_t& reg = this->get_target(atual.alvo);
      uint8_t lower = (reg << 4);
      uint8_t upper = (reg >> 4);

      reg = (lower | upper);
      this->registradores.f = 0;
      if(!reg)
        this->registradores.f |= BIT_ZERO;

      break;
    }
    default:
      throw std::runtime_error("Operação inválida.\n");
  }
}


