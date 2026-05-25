#ifndef ACTIONS_H
#define ACTIONS_H

#include "cpu.h"

namespace GBInstruct{
    using namespace GB;  

    inline void NOP(const Action& atual, CPU *cpu){
      cpu->ciclos_esperados = 1;
      return;
    }

    inline void STOP(const Action& atual, CPU *cpu){
      cpu->stepping = false;
      cpu->incrementa_pc();
      cpu->ciclos_esperados = 1;
    }

    inline void HALT(const Action& atual, CPU *cpu){
      if((cpu->get_ie() & cpu->get_if() & 0x1F) && !cpu->ime){
        cpu->haltbug = true;
      }
      else
        cpu->halted = true;

      ++cpu->pc;
      cpu->skipa_fetch = true;
      cpu->ciclos_esperados = 1;
    }

    inline void JPALWAYS(const Action& atual, CPU *cpu) {
      if(atual.alvo == reg_target::HL){
        cpu->pc = cpu->registradores.get_duplo(reg_target::HL);
        cpu->ciclos_esperados = 1;
      }
      else{
        cpu->incrementa_pc();
        uint16_t byte1 = static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->incrementa_pc();
        uint16_t byte2 = static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        uint16_t result = (byte1 | (byte2 << 8));
        cpu->pc = result;
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 4;
      }

      cpu->skipa_fetch = true;
    }

    inline void JPZERO(const Action& atual, CPU *cpu) {
      if(cpu->registradores.f & BIT_ZERO){
        cpu->incrementa_pc();
        uint16_t byte1 = static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->incrementa_pc();
        uint16_t byte2 = static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        uint16_t result = (byte1 | (byte2 << 8));
        cpu->pc = result;
        cpu->skipa_fetch = true;
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 4;
      }
      else{
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->incrementa_pc();
        cpu->incrementa_pc();
        cpu->ciclos_esperados = 3;
      }
    }

    inline void JPCARRY(const Action& atual, CPU *cpu){
      if(cpu->registradores.f & BIT_CARRY){
        cpu->incrementa_pc();
        uint16_t byte1 = static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->incrementa_pc();
        uint16_t byte2 = static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        uint16_t result = (byte1 | (byte2 << 8));
        cpu->pc = result;
        cpu->skipa_fetch = true;
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 4;
      }
      else{
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->incrementa_pc();
        cpu->incrementa_pc();
        cpu->ciclos_esperados = 3;
      }
    }
    
    inline void JPNZERO(const Action& atual, CPU *cpu){
      if(!(cpu->registradores.f & BIT_ZERO)){
        cpu->incrementa_pc();
        uint16_t byte1 = static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->incrementa_pc();
        uint16_t byte2 = static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        uint16_t result = (byte1 | (byte2 << 8));
        cpu->pc = result;
        cpu->skipa_fetch = true;
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 4;
      }
      else{
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->incrementa_pc();
        cpu->incrementa_pc();
        cpu->ciclos_esperados = 3;
      }
    }

    inline void JPNCARRY(const Action& atual, CPU *cpu){
      if(!(cpu->registradores.f & BIT_CARRY)){
        cpu->incrementa_pc();
        uint16_t byte1 = static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->incrementa_pc();
        uint16_t byte2 = static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        uint16_t result = (byte1 | (byte2 << 8));
        cpu->pc = result;
        cpu->skipa_fetch = true;
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 4;
      }
      else{
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->incrementa_pc();
        cpu->incrementa_pc();
        cpu->ciclos_esperados = 3;
      }
    }

    inline void JRALWAYS(const Action& atual, CPU *cpu) {
      cpu->incrementa_pc();
      int8_t add = static_cast<int8_t>(cpu->bus.read_byte(cpu->pc));
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      cpu->pc = static_cast<uint16_t>(cpu->pc + static_cast<int16_t>(add) + 1);
      cpu->skipa_fetch = true;
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      cpu->ciclos_esperados = 3;
    }

    inline void JRZERO(const Action& atual, CPU *cpu){
      cpu->incrementa_pc();
      if(cpu->registradores.f & BIT_ZERO){
        int8_t add = static_cast<int8_t>(cpu->bus.read_byte(cpu->pc));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->pc = static_cast<uint16_t>(cpu->pc + static_cast<int16_t>(add) + 1);
        cpu->skipa_fetch = true;
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 3;
      }
      else{
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 2;
      }
    }

    inline void JRCARRY(const Action& atual, CPU *cpu){
      cpu->incrementa_pc();
      if(cpu->registradores.f & BIT_CARRY){
        int8_t add = static_cast<int8_t>(cpu->bus.read_byte(cpu->pc));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->pc = static_cast<uint16_t>(cpu->pc + static_cast<int16_t>(add) + 1);
        cpu->skipa_fetch = true;
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 3;
      }
      else{
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 2;
      }
    }

    inline void JRNZERO(const Action& atual, CPU *cpu){
      cpu->incrementa_pc();
      if(!(cpu->registradores.f & BIT_ZERO)){
        int8_t add = static_cast<int8_t>(cpu->bus.read_byte(cpu->pc));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->pc = static_cast<uint16_t>(cpu->pc + static_cast<int16_t>(add) + 1);
        cpu->skipa_fetch = true;
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 3;
      }
      else{
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 2;
      }
    }

    inline void JRNCARRY(const Action& atual, CPU *cpu){
      cpu->incrementa_pc();
      if(!(cpu->registradores.f & BIT_CARRY)){
        int8_t add = static_cast<int8_t>(cpu->bus.read_byte(cpu->pc));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->pc = static_cast<uint16_t>(cpu->pc + static_cast<int16_t>(add) + 1);
        cpu->skipa_fetch = true;
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 3;
      }
      else{
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 2;
      }
    }

    inline void LD(const Action& atual, CPU *cpu){
      if(atual.N == 0xFEA0){ //endereço de 8 bits
        uint8_t valor = cpu->get_target_value(atual.ld_alvo);
        uint8_t& memoria = cpu->get_target_ref(atual.alvo);
        memoria = valor;
      }
      else{
        switch(atual.alvo){
          using enum reg_target;
          case HLI:{
            cpu->registradores.set_duplo(reg_target::HL, atual.N + 1);
            break;
          }
          case HLD: {
            cpu->registradores.set_duplo(reg_target::HL, atual.N - 1);
            break;
          }
        }

        uint8_t valor = cpu->get_target_value(atual.ld_alvo);

        cpu->bus.write_byte(atual.N, valor);
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      }

      cpu->ciclos_esperados = atual.bit_index/4;
    }

    inline void LDDUP(const Action& atual, CPU *cpu){
      if(atual.alvo == reg_target::SP){
        uint16_t valor = cpu->get_target_duplo(atual.ld_alvo);
        cpu->sp = valor;
        cpu->ciclos_esperados = 3;
        if(atual.ld_alvo == reg_target::HL){
          roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
          cpu->ciclos_esperados = 2;
        }
      }
      else{
        uint16_t valor = cpu->get_target_duplo(atual.ld_alvo);
        cpu->registradores.set_duplo(atual.alvo, valor);
        cpu->ciclos_esperados = 3;
      }
    }

    inline void LDHL(const Action& atual, CPU *cpu){
      cpu->incrementa_pc();
      int8_t add = static_cast<int8_t>(cpu->bus.read_byte(cpu->pc));
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      uint16_t result = static_cast<uint16_t>(cpu->sp + static_cast<int16_t>(add));
      cpu->registradores.set_duplo(reg_target::HL, result);
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);

      cpu->registradores.f = 0;
      if((cpu->sp & 0x0F) + (add & 0x0F) > 0x0F)
        cpu->registradores.f |= BIT_HALFCARRY;
      if((cpu->sp & 0xFF) + (add & 0xFF) > 0xFF)
        cpu->registradores.f |= BIT_CARRY;

      cpu->ciclos_esperados = 3;
    }

    inline void LDSP(const Action& atual, CPU *cpu){
      cpu->incrementa_pc();
      uint16_t lower = static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc)); 
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      cpu->incrementa_pc();
      uint16_t upper = (static_cast<uint16_t>(cpu->bus.read_byte(cpu->pc)) << 8); 
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      uint16_t resultado = lower | upper;

      cpu->bus.write_byte(resultado, static_cast<uint8_t>(cpu->sp & 0xFF));
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);

      cpu->bus.write_byte(resultado + 1, static_cast<uint8_t>((cpu->sp >> 8) & 0xFF));
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      cpu->ciclos_esperados = 5;
    }

    inline void PUSH(const Action& atual, CPU *cpu){
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      cpu->push(atual.alvo);
      cpu->ciclos_esperados = 4;
    }

    inline void POP(const Action& atual, CPU *cpu){
      cpu->pop(atual.alvo);
      cpu->ciclos_esperados = 3;
    }

    inline void CALLALWAYS(const Action& atual, CPU *cpu){
      uint16_t valor = cpu->get_target_duplo(reg_target::n);
      cpu->call(valor);
      cpu->skipa_fetch = true;
      cpu->ciclos_esperados = 6;
    }

    inline void CALLZERO(const Action& atual, CPU *cpu){
      uint16_t valor = cpu->get_target_duplo(reg_target::n);
      cpu->ciclos_esperados = 3;
      if(cpu->registradores.f & BIT_ZERO){
        cpu->call(valor);
        cpu->skipa_fetch = true;
        cpu->ciclos_esperados = 6;
      }
    }

    inline void CALLNZERO(const Action& atual, CPU *cpu){
      uint16_t valor = cpu->get_target_duplo(reg_target::n);
      cpu->ciclos_esperados = 3;
      if(!(cpu->registradores.f & BIT_ZERO)){
        cpu->call(valor);
        cpu->skipa_fetch = true;
        cpu->ciclos_esperados = 6;
      }
    }

    inline void CALLCARRY(const Action& atual, CPU *cpu){
      uint16_t valor = cpu->get_target_duplo(reg_target::n);
      cpu->ciclos_esperados = 3;
      if(cpu->registradores.f & BIT_CARRY){
        cpu->call(valor);
        cpu->skipa_fetch = true;
        cpu->ciclos_esperados = 6;
      }
    }

    inline void CALLNCARRY(const Action& atual, CPU *cpu){
      uint16_t valor = cpu->get_target_duplo(reg_target::n);
      cpu->ciclos_esperados = 3;
      if(!(cpu->registradores.f & BIT_CARRY)){
        cpu->call(valor);
        cpu->skipa_fetch = true;
        cpu->ciclos_esperados = 6;
      }
    }

    inline void RETALWAYS(const Action& atual, CPU *cpu){
      cpu->ret();
      cpu->skipa_fetch = true;
      cpu->ciclos_esperados = 4;
    }

    inline void RETZERO(const Action& atual, CPU *cpu){
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      cpu->ciclos_esperados = 2;
      if(cpu->registradores.f & BIT_ZERO){
        cpu->ret();
        cpu->skipa_fetch = true;
        cpu->ciclos_esperados = 5;
      }
    }

    inline void RETNZERO(const Action& atual, CPU *cpu){
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      cpu->ciclos_esperados = 2;
      if(!(cpu->registradores.f & BIT_ZERO)){
        cpu->ret();
        cpu->skipa_fetch = true;
        cpu->ciclos_esperados = 5;
      }
    }

    inline void RETCARRY(const Action& atual, CPU *cpu){
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      cpu->ciclos_esperados = 2;
      if(cpu->registradores.f & BIT_CARRY){
        cpu->ret();
        cpu->skipa_fetch = true;
        cpu->ciclos_esperados = 5;
      }
    }

    inline void RETNCARRY(const Action& atual, CPU *cpu){
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      cpu->ciclos_esperados = 2;
      if(!(cpu->registradores.f & BIT_CARRY)){
        cpu->ret();
        cpu->skipa_fetch = true;
        cpu->ciclos_esperados = 5;
      }
    }

    inline void ADD(const Action& atual, CPU *cpu){
      uint8_t valor = cpu->get_target_value(atual.alvo);
      cpu->registradores.f = 0;
      uint32_t result = static_cast<uint32_t>(cpu->registradores.a) + static_cast<uint32_t>(valor);

      if(result > 0xFF)
        cpu->registradores.f |= BIT_CARRY;
      if((result & 0xFF) == 0)
        cpu->registradores.f |= BIT_ZERO;
      if((cpu->registradores.a & 0x0F) + (valor & 0x0F) > 0x0F)
        cpu->registradores.f |= BIT_HALFCARRY;
      
      cpu->registradores.a = static_cast<uint8_t>(result & 0xFF);
      cpu->ciclos_esperados = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 2 : 1;
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
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      cpu->ciclos_esperados = 2;
    }

    inline void ADDSP(const Action& atual, CPU *cpu){
      cpu->incrementa_pc();
      int8_t valor = static_cast<int8_t>(cpu->bus.read_byte(cpu->pc));
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      uint32_t result = static_cast<uint32_t>(cpu->sp) + static_cast<uint32_t>(static_cast<int16_t>(valor));

      cpu->registradores.f = 0;
      if((cpu->sp & 0xFF) + (static_cast<uint8_t>(valor) & 0xFF) > 0xFF)
        cpu->registradores.f |= BIT_CARRY;
      if((cpu->sp & 0x0F) + (static_cast<uint8_t>(valor) & 0x0F) > 0x0F)
        cpu->registradores.f |= BIT_HALFCARRY;

      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      cpu->sp = static_cast<uint16_t>(result & 0xFFFF);
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      cpu->ciclos_esperados = 4;
    }

    inline void ADC(const Action& atual, CPU *cpu){
      uint8_t valor = cpu->get_target_value(atual.alvo);
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
      cpu->ciclos_esperados = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 2 : 1;
    }

    inline void SUB(const Action& atual, CPU *cpu){
      uint8_t valor = cpu->get_target_value(atual.alvo);
      uint32_t result = static_cast<uint32_t>(cpu->registradores.a) - static_cast<uint32_t>(valor);
      
      cpu->registradores.f = BIT_SUBTRACT;

      if((result & 0xFF) == 0)
        cpu->registradores.f |= BIT_ZERO;
      if((cpu->registradores.a & 0x0F) < (valor & 0x0F))
        cpu->registradores.f |= BIT_HALFCARRY;
      if((cpu->registradores.a & 0xFF) < (valor & 0xFF))
        cpu->registradores.f |= BIT_CARRY;

      cpu->registradores.a = static_cast<uint8_t>(result & 0xFF);
      cpu->ciclos_esperados = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 2 : 1;
    }

    inline void SBC(const Action& atual, CPU *cpu){
      uint8_t valor = cpu->get_target_value(atual.alvo);
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
      cpu->ciclos_esperados = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 2 : 1;
    }

    inline void AND(const Action& atual, CPU *cpu){
      uint8_t valor = cpu->get_target_value(atual.alvo);
      uint8_t result = (cpu->registradores.a & valor);
      cpu->registradores.f = BIT_HALFCARRY;

      if(result == 0)
        cpu->registradores.f |= BIT_ZERO;
      
      cpu->registradores.a = result;
      cpu->ciclos_esperados = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 2 : 1;
    }

    inline void OR(const Action& atual, CPU *cpu){
      uint8_t valor = cpu->get_target_value(atual.alvo);
      uint8_t result = (cpu->registradores.a | valor);
      cpu->registradores.f = 0;

      if(result == 0)
        cpu->registradores.f |= BIT_ZERO;
      
      cpu->registradores.a = result;
      cpu->ciclos_esperados = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 2 : 1;
    }

    inline void XOR(const Action& atual, CPU *cpu){
      uint8_t valor = cpu->get_target_value(atual.alvo);
      uint8_t result = (cpu->registradores.a ^ valor);
      cpu->registradores.f = 0;

      if(result == 0)
        cpu->registradores.f |= BIT_ZERO;
      
      cpu->registradores.a = result;
      cpu->ciclos_esperados = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 2 : 1;
    }

    inline void CP(const Action& atual, CPU *cpu){
      uint8_t valor = cpu->get_target_value(atual.alvo);
      uint32_t result = static_cast<uint32_t>(cpu->registradores.a) - static_cast<uint32_t>(valor);
      
      cpu->registradores.f = BIT_SUBTRACT;

      if((result & 0xFF) == 0)
        cpu->registradores.f |= BIT_ZERO;
      if((cpu->registradores.a & 0x0F) < (valor & 0x0F))
        cpu->registradores.f |= BIT_HALFCARRY;
      if((cpu->registradores.a & 0xFF) < (valor & 0xFF))
        cpu->registradores.f |= BIT_CARRY;

      cpu->ciclos_esperados = (atual.alvo == reg_target::HL || atual.alvo == reg_target::n) ? 2 : 1;
    }

    inline void INC(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target_ref(atual.alvo);
      uint32_t result = static_cast<uint32_t>(reg) + 1;

      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if((result & 0xFF) == 0)
        cpu->registradores.f |= BIT_ZERO;
      if((reg & 0x0F) + 0x01 > 0x0F)
        cpu->registradores.f |= BIT_HALFCARRY;
     
      if(atual.alvo == reg_target::HL){
        uint16_t address = cpu->registradores.get_duplo(reg_target::HL);
        cpu->bus.write_byte(address, reg + 1);
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      }
      else{
        ++reg;
      }

      cpu->ciclos_esperados = (atual.alvo == reg_target::HL) ? 3 : 1;
    }

    inline void INCDUP(const Action& atual, CPU *cpu){
      uint16_t reg = cpu->get_target_duplo(atual.alvo);
            
      if(atual.alvo != reg_target::SP)
        cpu->registradores.set_duplo(atual.alvo, reg + 1);
      else{
        cpu->sp++;
      }
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);

      cpu->ciclos_esperados = 2;
    }

    inline void DEC(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target_ref(atual.alvo);
      uint32_t result = static_cast<uint32_t>(reg) - 1;

      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY);
      cpu->registradores.f |= BIT_SUBTRACT;
      if((result & 0xFF) == 0)
        cpu->registradores.f |= BIT_ZERO;
      if((reg & 0x0F) == 0)
        cpu->registradores.f |= BIT_HALFCARRY;
      
      if(atual.alvo == reg_target::HL){
        uint16_t address = cpu->registradores.get_duplo(reg_target::HL);
        cpu->bus.write_byte(address, reg - 1);
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      }
      else{
        --reg;
      }

      cpu->ciclos_esperados = (atual.alvo == reg_target::HL) ? 3 : 1;
    }

    inline void DECDUP(const Action& atual, CPU *cpu){
      uint16_t reg = cpu->get_target_duplo(atual.alvo);
            
      if(atual.alvo != reg_target::SP)
        cpu->registradores.set_duplo(atual.alvo, reg - 1);
      else{
        cpu->sp--;
      }

      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      cpu->ciclos_esperados = 2;
    }

    inline void CCF(const Action& atual, CPU *cpu){
      cpu->registradores.f ^= BIT_CARRY;
      cpu->registradores.f &= ~(BIT_SUBTRACT | BIT_HALFCARRY);
      cpu->ciclos_esperados = 1;
    }

    inline void SCF(const Action& atual, CPU *cpu){
      cpu->registradores.f |= BIT_CARRY;
      cpu->registradores.f &= ~(BIT_SUBTRACT | BIT_HALFCARRY);
      cpu->ciclos_esperados = 1;
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
      cpu->ciclos_esperados = 1;
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
      cpu->ciclos_esperados = 1;
    }

    inline void RRCA(const Action& atual, CPU *cpu){
      uint8_t bit0 = (cpu->registradores.a & 0x01);
      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit0)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      cpu->registradores.a = ((cpu->registradores.a >> 1) | (bit0 << 7));
      cpu->ciclos_esperados = 1;
    } 

    inline void RLCA(const Action& atual, CPU *cpu){
      uint8_t bit7 = ((cpu->registradores.a & (1 << 7)) > 0) ? 1 : 0;
      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit7)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      cpu->registradores.a = ((cpu->registradores.a << 1) | bit7);
      cpu->ciclos_esperados = 1;
    } 

    inline void CPL(const Action& atual, CPU *cpu){
      cpu->registradores.a = ~cpu->registradores.a;
      cpu->registradores.f |= (BIT_SUBTRACT | BIT_HALFCARRY);
      cpu->ciclos_esperados = 1;
    }

    inline void DI(const Action& atual, CPU *cpu){
      cpu->ime = false;
      cpu->ime_count = 0;
      cpu->ciclos_esperados = 1;
    }

    inline void EI(const Action& atual, CPU *cpu){
      cpu->ime_count = 1;
      cpu->ciclos_esperados = 1;
    }

    inline void RETI(const Action& atual, CPU *cpu){
      cpu->ret();
      cpu->ime = true;
      cpu->skipa_fetch = true;
      cpu->ciclos_esperados = 4;
    }

    inline void RST(const Action& atual, CPU *cpu){
      ++cpu->pc;
      cpu->push(cpu->pc);
      
      cpu->pc = atual.N;
      cpu->skipa_fetch = true;
      roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
      cpu->ciclos_esperados = 4;
    }

    inline void BIT(const Action& atual, CPU *cpu){
      uint8_t bit = cpu->get_bit(atual.alvo, atual.bit_index);
      cpu->registradores.f &= ~(BIT_ZERO | BIT_SUBTRACT);
      cpu->registradores.f |= BIT_HALFCARRY;
      if(!bit)
        cpu->registradores.f |= BIT_ZERO;
      
      cpu->ciclos_esperados = (atual.alvo == reg_target::HL) ? 3 : 2;
    }

    inline void RESET(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target_ref(atual.alvo);
      if(atual.alvo != reg_target::HL){
        reg &= ~(1 << atual.bit_index);
        cpu->ciclos_esperados = 2;
      }
      else{
        uint16_t address = cpu->registradores.get_duplo(reg_target::HL);
        cpu->bus.write_byte(address, (reg & ~(1 << atual.bit_index)));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 4;
      }

    }

    inline void SET(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target_ref(atual.alvo);
      if(atual.alvo != reg_target::HL){
        reg |= (1 << atual.bit_index);
        cpu->ciclos_esperados = 2;
      }
      else{
        uint16_t address = cpu->registradores.get_duplo(reg_target::HL);
        cpu->bus.write_byte(address, (reg | (1 << atual.bit_index)));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 4;
      }

    }

    inline void SRL(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target_ref(atual.alvo);
      uint8_t bit0 = (reg & 0x01);
      if(atual.alvo != reg_target::HL){
        reg = (reg >> 1);
        cpu->ciclos_esperados = 2;
      }
      else{
        uint16_t address = cpu->registradores.get_duplo(reg_target::HL);
        cpu->bus.write_byte(address, (reg >> 1));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 4;
      }

      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);

      if(bit0)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

    }

    inline void RR(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target_ref(atual.alvo);
      uint8_t bit0 = (reg & 0x01);
      uint8_t carry = ((cpu->registradores.f & BIT_CARRY) > 0) ? 1 : 0;

      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit0)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      if(atual.alvo != reg_target::HL){
        reg = ((reg >> 1) | (carry << 7));
        cpu->ciclos_esperados = 2;
      }
      else{
        uint16_t address = cpu->registradores.get_duplo(reg_target::HL);
        cpu->bus.write_byte(address, ((reg >> 1) | (carry << 7)));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 4;
      }

      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

    }

    inline void RL(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target_ref(atual.alvo);
      uint8_t bit7 = (reg & (1 << 7));
      uint8_t carry = ((cpu->registradores.f & BIT_CARRY) > 0) ? 1 : 0;

      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit7)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      if(atual.alvo != reg_target::HL){
        reg = ((reg << 1) | carry);
        cpu->ciclos_esperados = 2;
      }
      else{
        uint16_t address = cpu->registradores.get_duplo(reg_target::HL);
        cpu->bus.write_byte(address, ((reg << 1) | carry));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 4;
      }

      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

    }

    inline void RRC(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target_ref(atual.alvo);
      uint8_t bit0 = (reg & 0x01);
      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit0)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      if(atual.alvo != reg_target::HL){
        reg = ((reg >> 1) | (bit0 << 7));
        cpu->ciclos_esperados = 2;
      }
      else{
        uint16_t address = cpu->registradores.get_duplo(reg_target::HL);
        cpu->bus.write_byte(address, ((reg >> 1) | (bit0 << 7)));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 4;
      }

      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

    }

    inline void RLC(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target_ref(atual.alvo);
      uint8_t bit7 = ((reg & (1 << 7)) > 0) ? 1 : 0;
      cpu->registradores.f &= ~(BIT_ZERO | BIT_HALFCARRY | BIT_SUBTRACT);
      if(bit7)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      if(atual.alvo != reg_target::HL){
        reg = ((reg << 1) | bit7);
        cpu->ciclos_esperados = 2;
      }
      else{
        uint16_t address = cpu->registradores.get_duplo(reg_target::HL);
        cpu->bus.write_byte(address, ((reg << 1) | bit7));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 4;
      }

      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

    }

    inline void SRA(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target_ref(atual.alvo);
      uint8_t bit7 = ((reg & (1 << 7)) > 0) ? 1 : 0;
      uint8_t carry = (reg & 0x01);
      cpu->registradores.f = 0;

      if(carry)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      if(atual.alvo != reg_target::HL){
        reg = ((reg >> 1) | (bit7 << 7));
        cpu->ciclos_esperados = 2;
      }
      else{
        uint16_t address = cpu->registradores.get_duplo(reg_target::HL);
        cpu->bus.write_byte(address, ((reg >> 1) | (bit7 << 7)));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 4;
      }

      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

    }

    inline void SLA(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target_ref(atual.alvo);
      uint8_t carry = (reg & (1 << 7));
      cpu->registradores.f = 0;

      if(carry)
        cpu->registradores.f |= BIT_CARRY;
      else
        cpu->registradores.f &= ~BIT_CARRY;

      if(atual.alvo != reg_target::HL){
        reg = (reg << 1);
        cpu->ciclos_esperados = 2;
      }
      else{
        uint16_t address = cpu->registradores.get_duplo(reg_target::HL);
        cpu->bus.write_byte(address, (reg << 1));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 4;
      }

      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

    }

    inline void SWAP(const Action& atual, CPU *cpu){
      uint8_t& reg = cpu->get_target_ref(atual.alvo);
      uint8_t lower = (reg << 4);
      uint8_t upper = (reg >> 4);

      if(atual.alvo != reg_target::HL){
        reg = (lower | upper);
        cpu->ciclos_esperados = 2;
      }
      else{
        uint16_t address = cpu->registradores.get_duplo(reg_target::HL);
        cpu->bus.write_byte(address, (lower | upper));
        roda_perifericos(cpu, cpu->bus.timer, cpu->bus.ppu);
        cpu->ciclos_esperados = 4;
      }

      cpu->registradores.f = 0;
      if(!reg)
        cpu->registradores.f |= BIT_ZERO;

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

      cpu->ciclos_esperados = 1;
    }








}

#endif
