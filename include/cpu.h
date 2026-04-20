#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <array>
#include <stdexcept>

#define BIT_ZERO (1 << 7) //quando o resultado de uma instrução é zero
#define BIT_SUBTRACT (1 << 6) //quando a ultima instrução foi uma subtração
#define BIT_HALFCARRY (1 << 5) //quando há estouro do bit 3 ou 11
#define BIT_CARRY (1 << 4) //quando há estouro do bit 7 ou 15

//flags de interrupção
#define BIT_VBLANK (1 << 0)
#define BIT_LCDSTAT (1 << 1)
#define BIT_TIMER (1 << 2)
#define BIT_SERIAL (1 << 3)
#define BIT_JOYPAD (1 << 4)

namespace GB{

enum class reg_target: uint8_t{
  A,
  B,
  C,
  D,
  E,
  F,
  H,
  L,
  AF,
  BC,
  DE,
  HL,
  SP,
  n,
  A8,
  A16,
  HLI,
  HLD,
  CPTR,
  NULO
};

struct Registradores{
  uint8_t a {};
  uint8_t b {};
  uint8_t c {};
  uint8_t d {};
  uint8_t e {};
  uint8_t f {};
  uint8_t h {};
  uint8_t l {};

  uint16_t get_duplo(reg_target registrador) const{

    switch(registrador){
      case reg_target::AF:
        return ((static_cast<uint16_t>(a) << 8) | static_cast<uint16_t>(f));
      case reg_target::BC:
        return ((static_cast<uint16_t>(b) << 8) | static_cast<uint16_t>(c));
      case reg_target::DE:
        return ((static_cast<uint16_t>(d) << 8) | static_cast<uint16_t>(e));
      case reg_target::HL:
        return ((static_cast<uint16_t>(h) << 8) | static_cast<uint16_t>(l));
      default:
        throw std::runtime_error("Registrador inválido.\n");
    }
  }
  
  void set_duplo(reg_target registrador, uint16_t valor){

    switch(registrador){
      case reg_target::AF:
        a = static_cast<uint8_t>((valor & 0xFF00) >> 8);
        f = static_cast<uint8_t>(valor & 0xFF);
        break;
      case reg_target::BC:
        b = static_cast<uint8_t>((valor & 0xFF00) >> 8);
        c = static_cast<uint8_t>(valor & 0xFF);
        break;
      case reg_target::DE:
        d = static_cast<uint8_t>((valor & 0xFF00) >> 8);
        e = static_cast<uint8_t>(valor & 0xFF);
        break;
      case reg_target::HL:
        h = static_cast<uint8_t>((valor & 0xFF00) >> 8);
        l = static_cast<uint8_t>(valor & 0xFF);
        break;
      default:
        throw std::runtime_error("Registrador inválido.\n");
    }
  }
};

struct Memorybus{
  std::array<uint8_t, 0xFFFF + 1> memoria{};

  uint8_t& read_byte(uint16_t endereco){
    return memoria[endereco];
  }
  uint8_t read_byte_const(uint16_t endereco) const{
    return memoria[endereco];
  }
};

struct CPU{
  Memorybus bus;
  Registradores registradores;
  uint16_t pc {0x0100}; 
  uint16_t sp {0xFFFE}; 
  bool jp_flag {false};
  bool halted {false};
  bool stopped {true};
  bool ime {false};
  bool ime_ie {false};
  
  void step(void);
  void check(void);

  void push(reg_target alvo);
  void pop(reg_target alvo);
  void push(uint16_t valor);
  uint16_t pop(void);
  void call(uint16_t endereco);
  void ret(void);

  uint8_t& get_ie(void) { return bus.read_byte(0xFFFF); }
  uint8_t& get_if(void) { return bus.read_byte(0xFF0F); }
  uint8_t& get_target(reg_target alvo);
  uint16_t get_target_duplo(reg_target alvo) const;
  uint8_t get_bit(reg_target alvo, uint8_t bit) const;
};

struct Action{
  void (*execute)(const Action&, CPU*);
  uint8_t tamanho;
  reg_target alvo;
  uint16_t N;
  uint8_t bit_index;
  reg_target ld_alvo;

  Action(void (*ptr)(const Action&, CPU*), uint8_t tam, reg_target a = reg_target::NULO, uint16_t n = 0, uint8_t b = 0, reg_target ld = reg_target::NULO): 
    execute{ptr}, tamanho{tam}, alvo{a}, N{n}, bit_index{b}, ld_alvo {ld} {}
};

Action le_byte(uint8_t byte, CPU *atual);
Action le_byte_cb(uint8_t byte, CPU *atual);
void roda_cpu(CPU *atual);

}
#endif 
