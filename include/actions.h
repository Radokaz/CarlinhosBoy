#ifndef ACTIONS_H
#define ACTIONS_H

#include "cpu.h"

namespace GBInstruct{
    using namespace GB;  

    inline void NOP(const Action& atual, CPU *cpu){
      cpu->last_ticks = 4;
      return;
    }

    inline void STOP(const Action& atual, CPU *cpu){
      cpu->last_ticks = 4;
      cpu->stepping = false;
      cpu->jp_flag = true;
      cpu->pc+=2;
    }

    inline void HALT(const Action& atual, CPU *cpu){
      if(cpu->get_ie() & cpu->get_if() & 0x1F)
        cpu->haltbug = true;
      else
        cpu->halted = true;

      cpu->last_ticks = 4;
      cpu->jp_flag = true;
      ++cpu->pc;
    }

    inline void JPALWAYS(const Action& atual, CPU *cpu) {
      if(atual.alvo == reg_target::HL){
        cpu->pc = cpu->registradores.get_duplo(reg_target::HL);
        cpu->last_ticks = 4;
      }
      else{
        cpu->pc = (static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc + 1)) | (static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc + 2)) << 8));
        cpu->last_ticks = 16;
      }

      cpu->jp_flag = true;
    }

    inline void JPZERO(const Action& atual, CPU *cpu) {
      if(cpu->registradores.f & BIT_ZERO){
        cpu->pc = (static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc + 1)) | (static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc + 2)) << 8));
        cpu->jp_flag = true;
        cpu->last_ticks = 16;
      }
      else
        cpu->last_ticks = 12;
    }

    inline void JPCARRY(const Action& atual, CPU *cpu){
      if(cpu->registradores.f & BIT_CARRY){
        cpu->pc = (static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc + 1)) | (static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc + 2)) << 8));
        cpu->jp_flag = true;
        cpu->last_ticks = 16;
      }
      else
        cpu->last_ticks = 12;
    }
    
    inline void JPNZERO(const Action& atual, CPU *cpu){
      if(!(cpu->registradores.f & BIT_ZERO)){
        cpu->pc = (static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc + 1)) | (static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc + 2)) << 8));
        cpu->jp_flag = true;
        cpu->last_ticks = 16;
      }
      else
        cpu->last_ticks = 12;
    }

    inline void JPNCARRY(const Action& atual, CPU *cpu){
      if(!(cpu->registradores.f & BIT_CARRY)){
        cpu->pc = (static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc + 1)) | (static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc + 2)) << 8));
        cpu->jp_flag = true;
        cpu->last_ticks = 16;
      }
      else
        cpu->last_ticks = 12;
    }

    inline void JRALWAYS(const Action& atual, CPU *cpu) {
      int8_t add = static_cast<int8_t>(cpu->bus.read_byte(cpu->pc + 1));
      cpu->pc = static_cast<uint16_t>(cpu->pc + static_cast<int16_t>(add) + atual.tamanho);
      cpu->jp_flag = true;
      cpu->last_ticks = 12;
    }

    inline void JRZERO(const Action& atual, CPU *cpu){
      if(cpu->registradores.f & BIT_ZERO){
        int8_t add = static_cast<int8_t>(cpu->bus.read_byte(cpu->pc + 1));
        cpu->pc = static_cast<uint16_t>(cpu->pc + static_cast<int16_t>(add) + atual.tamanho);
        cpu->jp_flag = true;
        cpu->last_ticks = 12;
      }
      else
        cpu->last_ticks = 8;
    }

    inline void JRCARRY(const Action& atual, CPU *cpu){
      if(cpu->registradores.f & BIT_CARRY){
        int8_t add = static_cast<int8_t>(cpu->bus.read_byte(cpu->pc + 1));
        cpu->pc = static_cast<uint16_t>(cpu->pc + static_cast<int16_t>(add) + atual.tamanho);
        cpu->jp_flag = true;
        cpu->last_ticks = 12;
      }
      else
        cpu->last_ticks = 8;
    }

    inline void JRNZERO(const Action& atual, CPU *cpu){
      if(!(cpu->registradores.f & BIT_ZERO)){
        int8_t add = static_cast<int8_t>(cpu->bus.read_byte(cpu->pc + 1));
        cpu->pc = static_cast<uint16_t>(cpu->pc + static_cast<int16_t>(add) + atual.tamanho);
        cpu->jp_flag = true;
        cpu->last_ticks = 12;
      }
      else
        cpu->last_ticks = 8;
    }

    inline void JRNCARRY(const Action& atual, CPU *cpu){
      if(!(cpu->registradores.f & BIT_CARRY)){
        int8_t add = static_cast<int8_t>(cpu->bus.read_byte(cpu->pc + 1));
        cpu->pc = static_cast<uint16_t>(cpu->pc + static_cast<int16_t>(add) + atual.tamanho);
        cpu->jp_flag = true;
        cpu->last_ticks = 12;
      }
      else
        cpu->last_ticks = 8;
    }

    inline void LD(const Action& atual, CPU *cpu){
      if(atual.N == 0xFEA0){
        uint8_t& memoria = cpu->get_target(atual.alvo);
        uint8_t valor = cpu->get_target(atual.ld_alvo);
        memoria = valor;
      }
      else{
        uint8_t valor = cpu->get_target(atual.ld_alvo);
        if(atual.alvo == reg_target::HLI)
          cpu->registradores.set_duplo(reg_target::HL, atual.N + 1);
        else if(atual.alvo == reg_target::HLD)
          cpu->registradores.set_duplo(reg_target::HL, atual.N - 1);

        cpu->bus.write_byte(atual.N, valor);
      }
      
      cpu->last_ticks = atual.bit_index;
    }

    inline void LDDUP(const Action& atual, CPU *cpu){
      if(atual.alvo == reg_target::SP){
        uint16_t valor = cpu->get_target_duplo(atual.ld_alvo);
        cpu->sp = valor;
        cpu->last_ticks = (atual.ld_alvo == reg_target::n) ? 12 : 8;
      }
      else{
        uint16_t valor = cpu->get_target_duplo(atual.ld_alvo);
        cpu->registradores.set_duplo(atual.alvo, valor);
        cpu->last_ticks = 12;
      }
    }

    inline void LDHL(const Action& atual, CPU *cpu){
      int8_t add = static_cast<int8_t>(cpu->bus.read_byte(cpu->pc + 1));
      uint16_t result = static_cast<uint16_t>(cpu->sp + static_cast<int16_t>(add));
      cpu->registradores.set_duplo(reg_target::HL, result);
      cpu->registradores.f = 0;
      if((cpu->sp & 0x0F) + (add & 0x0F) > 0x0F)
        cpu->registradores.f |= BIT_HALFCARRY;
      if((cpu->sp & 0xFF) + (add & 0xFF) > 0xFF)
        cpu->registradores.f |= BIT_CARRY;

      cpu->last_ticks = 12;
    }

    inline void LDSP(const Action& atual, CPU *cpu){
      uint16_t lower = static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc + 1)); 
      uint16_t upper = (static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc + 2)) << 8); 
      uint16_t resultado = lower | upper;

      cpu->bus.write_byte(resultado, static_cast<uint8_t>(cpu->sp & 0xFF));
      cpu->bus.write_byte(resultado + 1, static_cast<uint8_t>((cpu->sp >> 8) & 0xFF));
      cpu->last_ticks = 20;
    }

    inline void PUSH(const Action& atual, CPU *cpu){
      cpu->push(atual.alvo);
      cpu->last_ticks = 16;
    }

    inline void POP(const Action& atual, CPU *cpu){
      cpu->pop(atual.alvo);
      cpu->last_ticks = 12;
    }

    inline void CALLALWAYS(const Action& atual, CPU *cpu){
      cpu->call(atual.N);
      cpu->jp_flag = true;
      cpu->last_ticks = 24;
    }

    inline void CALLZERO(const Action& atual, CPU *cpu){
      if(cpu->registradores.f & BIT_ZERO){
        cpu->call(atual.N);
        cpu->jp_flag = true;
        cpu->last_ticks = 24;
      }
      else
        cpu->last_ticks = 12;
    }

    inline void CALLNZERO(const Action& atual, CPU *cpu){
      if(!(cpu->registradores.f & BIT_ZERO)){
        cpu->call(atual.N);
        cpu->jp_flag = true;
        cpu->last_ticks = 24;
      }
      else
        cpu->last_ticks = 12;
    }

    inline void CALLCARRY(const Action& atual, CPU *cpu){
      if(cpu->registradores.f & BIT_CARRY){
        cpu->call(atual.N);
        cpu->jp_flag = true;
        cpu->last_ticks = 24;
      }
      else
        cpu->last_ticks = 12;
    }

    inline void CALLNCARRY(const Action& atual, CPU *cpu){
      if(!(cpu->registradores.f & BIT_CARRY)){
        cpu->call(atual.N);
        cpu->jp_flag = true;
        cpu->last_ticks = 24;
      }
      else
        cpu->last_ticks = 12;
    }

    inline void RETALWAYS(const Action& atual, CPU *cpu){
      cpu->ret();
      cpu->jp_flag = true;
      cpu->last_ticks = 16;
    }

    inline void RETZERO(const Action& atual, CPU *cpu){
      if(cpu->registradores.f & BIT_ZERO){
        cpu->ret();
        cpu->jp_flag = true;
        cpu->last_ticks = 20;
      }
      else
        cpu->last_ticks = 8;
    }

    inline void RETNZERO(const Action& atual, CPU *cpu){
      if(!(cpu->registradores.f & BIT_ZERO)){
        cpu->ret();
        cpu->jp_flag = true;
        cpu->last_ticks = 20;
      }
      else
        cpu->last_ticks = 8;
    }

    inline void RETCARRY(const Action& atual, CPU *cpu){
      if(cpu->registradores.f & BIT_CARRY){
        cpu->ret();
        cpu->jp_flag = true;
        cpu->last_ticks = 20;
      }
      else
        cpu->last_ticks = 8;
    }

    inline void RETNCARRY(const Action& atual, CPU *cpu){
      if(!(cpu->registradores.f & BIT_CARRY)){
        cpu->ret();
        cpu->jp_flag = true;
        cpu->last_ticks = 20;
      }
      else
        cpu->last_ticks = 8;
    }

    inline void ADD(const Action& atual, CPU *cpu){
      uint8_t valor = (atual.alvo == reg_target::n) ? static_cast<uint8_t>(atual.N) : cpu->get_target(atual.alvo);
      cpu->registradores.f = 0;
      uint32_t result = static_cast<uint32_t>(cpu->registradores.a) + static_cast<uint32_t>(valor);

      if(result > 0xFF)
        cpu->registradores.f |= BIT_CARRY;
      if((result & 0xFF) == 0)
        cpu->registradores.f |= BIT_ZERO;
      if((cpu->registradores.a & 0x0F) + (valor & 0x0F) > 0x0F)
        cpu->registradores.f |= BIT_HALFCARRY;
      
      cpu->registradores.a = static_cast<uint8_t>(result & 0xFF);
      cpu->last_ticks = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 8 : 4;
    }

    inline void ADDHL(const Action& atual, CPU *cpu){
      uint16_t valor = cpu->get_target_duplo(atual.alvo);
      uint32_t result = static_cast<uint32_t>(valor) + static_cast<uint32_t>(cpu->registradores.get_duplo(reg_target::HL));

      cpu->registradores.f &= ~(BIT_HALFCARRY | BIT_CARRY | BIT_SUBTRACT);
      if((cpu->registradores.get_duplo(reg_target::HL) & 0x0FFF) + (valor & 0x0FFF) > 0x0FFF)
        cpu->registradores.f |= BIT_HALFCARRY;
      if(result > 0xFFFF)
        cpu->registradores.f |= BIT_CARRY;

      cpu->registradores.set_duplo(reg_target::HL, static_cast<uint16_t>(result & 0xFFFF));
      cpu->last_ticks = 8;
    }

    inline void ADDSP(const Action& atual, CPU *cpu){
      int8_t valor = static_cast<int8_t>(cpu->bus.read_byte(cpu->pc + 1));
      uint32_t result = static_cast<uint32_t>(cpu->sp) + static_cast<uint32_t>(static_cast<int16_t>(valor));

      cpu->registradores.f = 0;
      if((cpu->sp & 0xFF) + (valor & 0xFF) > 0xFF)
        cpu->registradores.f |= BIT_CARRY;
      if((cpu->sp & 0x0F) + (valor & 0x0F) > 0x0F)
        cpu->registradores.f |= BIT_HALFCARRY;

      cpu->sp = static_cast<uint16_t>(result & 0xFFFF);
      cpu->last_ticks = 16;
    }

    inline void ADC(const Action& atual, CPU *cpu){
      uint8_t valor = (atual.alvo == reg_target::n) ? static_cast<uint8_t>(atual.N) : cpu->get_target(atual.alvo);
      uint32_t carry = ((cpu->registradores.f & BIT_CARRY) > 0) ? 1 : 0;

      uint32_t result = static_cast<uint32_t>(cpu->registradores.a) + static_cast<uint32_t>(valor) + carry;
      cpu->registradores.f = 0;

      if(result > 0xFF)
        cpu->registradores.f |= BIT_CARRY;
      if(((cpu->registradores.a + valor + carry) & 0xFF) == 0)
        cpu->registradores.f |= BIT_ZERO;
      if(((cpu->registradores.a & 0x0F) + (valor & 0x0F) + carry) > 0x0F)
        cpu->registradores.f |= BIT_HALFCARRY;
      
      cpu->registradores.a = static_cast<uint8_t>(result & 0xFF);
      cpu->last_ticks = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 8 : 4;
    }

    inline void SUB(const Action& atual, CPU *cpu){
      uint8_t valor = (atual.alvo == reg_target::n) ? static_cast<uint8_t>(atual.N) : cpu->get_target(atual.alvo);
      uint32_t result = static_cast<uint32_t>(cpu->registradores.a) - static_cast<uint32_t>(valor);
      
      cpu->registradores.f = BIT_SUBTRACT;

      if((result & 0xFF) == 0)
        cpu->registradores.f |= BIT_ZERO;
      if((cpu->registradores.a & 0x0F) < (valor & 0x0F))
        cpu->registradores.f |= BIT_HALFCARRY;
      if((cpu->registradores.a & 0xFF) < (valor & 0xFF))
        cpu->registradores.f |= BIT_CARRY;

      cpu->registradores.a = static_cast<uint8_t>(result & 0xFF);
      cpu->last_ticks = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 8 : 4;
    }

    inline void SBC(const Action& atual, CPU *cpu){
      uint8_t valor = (atual.alvo == reg_target::n) ? static_cast<uint8_t>(atual.N) : cpu->get_target(atual.alvo);
      uint32_t carry = ((cpu->registradores.f & BIT_CARRY) > 0) ? 1 : 0;
      uint32_t result = static_cast<uint32_t>(cpu->registradores.a) - static_cast<uint32_t>(valor) - carry;
      
      cpu->registradores.f = BIT_SUBTRACT;

      if((result & 0xFF) == 0)
        cpu->registradores.f |= BIT_ZERO;
      if((cpu->registradores.a & 0x0F) < ((valor & 0x0F) + carry))
        cpu->registradores.f |= BIT_HALFCARRY;
      if((cpu->registradores.a & 0xFF) < ((valor & 0xFF) + carry))
        cpu->registradores.f |= BIT_CARRY;

      cpu->registradores.a = static_cast<uint8_t>(result & 0xFF);
      cpu->last_ticks = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 8 : 4;
    }

    inline void AND(const Action& atual, CPU *cpu){
      uint8_t valor = (atual.alvo == reg_target::n) ? static_cast<uint8_t>(atual.N) : cpu->get_target(atual.alvo);
      uint8_t result = (cpu->registradores.a & valor);
      cpu->registradores.f = BIT_HALFCARRY;

      if(result == 0)
        cpu->registradores.f |= BIT_ZERO;
      
      cpu->registradores.a = result;
      cpu->last_ticks = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 8 : 4;
    }

    inline void OR(const Action& atual, CPU *cpu){
      uint8_t valor = (atual.alvo == reg_target::n) ? static_cast<uint8_t>(atual.N) : cpu->get_target(atual.alvo);
      uint8_t result = (cpu->registradores.a | valor);
      cpu->registradores.f = 0;

      if(result == 0)
        cpu->registradores.f |= BIT_ZERO;
      
      cpu->registradores.a = result;
      cpu->last_ticks = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 8 : 4;
    }

    inline void XOR(const Action& atual, CPU *cpu){
      uint8_t valor = (atual.alvo == reg_target::n) ? static_cast<uint8_t>(atual.N) : cpu->get_target(atual.alvo);
      uint8_t result = (cpu->registradores.a ^ valor);
      cpu->registradores.f = 0;

      if(result == 0)
        cpu->registradores.f |= BIT_ZERO;
      
      cpu->registradores.a = result;
      cpu->last_ticks = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 8 : 4;
    }

    inline void CP(const Action& atual, CPU *cpu){
      uint8_t valor = (atual.alvo == reg_target::n) ? static_cast<uint8_t>(atual.N) : cpu->get_target(atual.alvo);
      uint32_t result = static_cast<uint32_t>(cpu->registradores.a) - static_cast<uint32_t>(valor);
      
      cpu->registradores.f = BIT_SUBTRACT;

      if((result & 0xFF) == 0)
        cpu->registradores.f |= BIT_ZERO;
      if((cpu->registradores.a & 0x0F) < (valor & 0x0F))
        cpu->registradores.f |= BIT_HALFCARRY;
      if((cpu->registradores.a & 0xFF) < (valor & 0xFF))
        cpu->registradores.f |= BIT_CARRY;

      cpu->last_ticks = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 8 : 4;
    }

    inline void INC(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target(atual.alvo);
      uint32_t result = static_cast<uint32_t>(reg) + 1;

      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if((result & 0xFF) == 0)
        cpu->registradores.f |= BIT_ZERO;
      if((reg & 0x0F) + 0x01 > 0x0F)
        cpu->registradores.f |= BIT_HALFCARRY;
      
      ++reg;
      cpu->last_ticks = (atual.alvo == reg_target::HL) ? 12 : 4;
    }

    inline void INCDUP(const Action& atual, CPU *cpu){
      uint16_t reg = cpu->get_target_duplo(atual.alvo);
            
      if(atual.alvo != reg_target::SP)
        cpu->registradores.set_duplo(atual.alvo, reg + 1);
      else
        cpu->sp++;

      cpu->last_ticks = 8;
    }

    inline void DEC(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target(atual.alvo);
      uint32_t result = static_cast<uint32_t>(reg) - 1;

      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY);
      cpu->registradores.f |= BIT_SUBTRACT;
      if((result & 0xFF) == 0)
        cpu->registradores.f |= BIT_ZERO;
      if((reg & 0x0F) == 0)
        cpu->registradores.f |= BIT_HALFCARRY;
      
      --reg;
      cpu->last_ticks = (atual.alvo == reg_target::HL) ? 12 : 4;
    }

    inline void DECDUP(const Action& atual, CPU *cpu){
      uint16_t reg = cpu->get_target_duplo(atual.alvo);
            
      if(atual.alvo != reg_target::SP)
        cpu->registradores.set_duplo(atual.alvo, reg - 1);
      else
        cpu->sp--;

      cpu->last_ticks = 8;
    }

    inline void CCF(const Action& atual, CPU *cpu){
      cpu->registradores.f ^= BIT_CARRY;
      cpu->registradores.f &= ~(BIT_SUBTRACT | BIT_HALFCARRY);
      cpu->last_ticks = 4;
    }

    inline void SCF(const Action& atual, CPU *cpu){
      cpu->registradores.f |= BIT_CARRY;
      cpu->registradores.f &= ~(BIT_SUBTRACT | BIT_HALFCARRY);
      cpu->last_ticks = 4;
    }

    inline void RRA(const Action& atual, CPU *cpu){
      uint8_t bit0 = (cpu->registradores.a & 0x01);
      uint8_t carry = ((cpu->registradores.f & BIT_CARRY) > 0) ? 1 : 0;

      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit0)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      cpu->registradores.a = ((cpu->registradores.a >> 1) | (carry << 7));
      cpu->last_ticks = 4;
    }

    inline void RLA(const Action& atual, CPU *cpu){
      uint8_t bit7 = (cpu->registradores.a & (1 << 7));
      uint8_t carry = ((cpu->registradores.f & BIT_CARRY) > 0) ? 1 : 0;

      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit7)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      cpu->registradores.a = ((cpu->registradores.a << 1) | carry);
      cpu->last_ticks = 4;
    }

    inline void RRCA(const Action& atual, CPU *cpu){
      uint8_t bit0 = (cpu->registradores.a & 0x01);
      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit0)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      cpu->registradores.a = ((cpu->registradores.a >> 1) | (bit0 << 7));
      cpu->last_ticks = 4;
    } 

    inline void RLCA(const Action& atual, CPU *cpu){
      uint8_t bit7 = ((cpu->registradores.a & (1 << 7)) > 0) ? 1 : 0;
      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit7)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      cpu->registradores.a = ((cpu->registradores.a << 1) | bit7);
      cpu->last_ticks = 4;
    } 

    inline void CPL(const Action& atual, CPU *cpu){
      cpu->registradores.a = ~cpu->registradores.a;
      cpu->registradores.f |= (BIT_SUBTRACT | BIT_HALFCARRY);
      cpu->last_ticks = 4;
    }

    inline void DI(const Action& atual, CPU *cpu){
      cpu->ime = false;
      cpu->last_ticks = 4;
    }

    inline void EI(const Action& atual, CPU *cpu){
      cpu->ime_ie = true;
      cpu->last_ticks = 4;
    }

    inline void RETI(const Action& atual, CPU *cpu){
      cpu->pc = cpu->pop();
      cpu->ime = true;
      cpu->jp_flag = true;
      cpu->last_ticks = 16;
    }

    inline void RST(const Action& atual, CPU *cpu){
      cpu->push(cpu->pc + 1);
      cpu->pc = atual.N;
      cpu->jp_flag = true;
      cpu->last_ticks = 16;
    }

    inline void BIT(const Action& atual, CPU *cpu){
      uint8_t bit = cpu->get_bit(atual.alvo, atual.bit_index);
      cpu->registradores.f &= ~(BIT_ZERO | BIT_SUBTRACT);
      cpu->registradores.f |= BIT_HALFCARRY;
      if(!bit)
        cpu->registradores.f |= BIT_ZERO;
      
      cpu->last_ticks = (atual.alvo == reg_target::HL) ? 12 : 8;
    }

    inline void RESET(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target(atual.alvo);
      reg &= ~(1 << atual.bit_index);
      cpu->last_ticks = (atual.alvo == reg_target::HL) ? 16 : 8;
    }

    inline void SET(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target(atual.alvo);
      reg |= (1 << atual.bit_index);
      cpu->last_ticks = (atual.alvo == reg_target::HL) ? 16 : 8;
    }

    inline void SRL(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target(atual.alvo);
      uint8_t bit0 = (reg & 0x01);
      reg = (reg >> 1);
      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);

      if(bit0)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

      cpu->last_ticks = (atual.alvo == reg_target::HL) ? 16 : 8;
    }

    inline void RR(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target(atual.alvo);
      uint8_t bit0 = (reg & 0x01);
      uint8_t carry = ((cpu->registradores.f & BIT_CARRY) > 0) ? 1 : 0;

      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit0)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      reg = ((reg >> 1) | (carry << 7));
      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

      cpu->last_ticks = (atual.alvo == reg_target::HL) ? 16 : 8;
    }

    inline void RL(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target(atual.alvo);
      uint8_t bit7 = (reg & (1 << 7));
      uint8_t carry = ((cpu->registradores.f & BIT_CARRY) > 0) ? 1 : 0;

      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit7)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      reg = ((reg << 1) | carry);
      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

      cpu->last_ticks = (atual.alvo == reg_target::HL) ? 16 : 8;
    }

    inline void RRC(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target(atual.alvo);
      uint8_t bit0 = (reg & 0x01);
      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit0)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      reg = ((reg >> 1) | (bit0 << 7));
      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

      cpu->last_ticks = (atual.alvo == reg_target::HL) ? 16 : 8;
    }

    inline void RLC(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target(atual.alvo);
      uint8_t bit7 = ((reg & (1 << 7)) > 0) ? 1 : 0;
      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit7)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      reg = ((reg << 1) | bit7);
      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

      cpu->last_ticks = (atual.alvo == reg_target::HL) ? 16 : 8;
    }

    inline void SRA(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target(atual.alvo);
      uint8_t bit7 = ((reg & (1 << 7)) > 0) ? 1 : 0;
      uint8_t carry = (reg & 0x01);
      cpu->registradores.f = 0;

      if(carry)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      reg = ((reg >> 1) | (bit7 << 7));
      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

      cpu->last_ticks = (atual.alvo == reg_target::HL) ? 16 : 8;
    }

    inline void SLA(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target(atual.alvo);
      uint8_t carry = (reg & (1 << 7));
      cpu->registradores.f = 0;

      if(carry)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      reg = (reg << 1);
      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

      cpu->last_ticks = (atual.alvo == reg_target::HL) ? 16 : 8;
    }

    inline void SWAP(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target(atual.alvo);
      uint8_t lower = (reg << 4);
      uint8_t upper = (reg >> 4);

      reg = (lower | upper);
      cpu->registradores.f = 0;
      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

      cpu->last_ticks = (atual.alvo == reg_target::HL) ? 16 : 8;
    }

    inline void DAA(const Action& atual, CPU *cpu){
      uint8_t a = cpu->registradores.a;

      if(cpu->registradores.f & BIT_SUBTRACT){
        if(cpu->registradores.f & BIT_HALFCARRY)
          cpu->registradores.a -= 0x06;
        if((cpu->registradores.f & BIT_CARRY))
          cpu->registradores.a -= 0x60;
      }
      else{
        if((cpu->registradores.f & BIT_HALFCARRY) || (a & 0x0F) > 0x09)
          cpu->registradores.a += 0x06;
        if((cpu->registradores.f & BIT_CARRY) || a > 0x99){
          cpu->registradores.a += 0x60;
          cpu->registradores.f |= BIT_CARRY;
        }
      }

      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY);
      if(!cpu->registradores.a)
        cpu->registradores.f |= BIT_ZERO;

      cpu->last_ticks = 4;
    }








}

#endif
