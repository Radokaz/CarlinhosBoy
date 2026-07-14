#ifndef CPU_H
#define CPU_H

#include "memorybus.h"

#define BIT_ZERO (1 << 7) //quando o resultado de uma instrução é zero
#define BIT_SUBTRACT (1 << 6) //quando a ultima instrução foi uma subtração
#define BIT_HALFCARRY (1 << 5) //quando há estouro do bit 3 ou 11
#define BIT_CARRY (1 << 4) //quando há estouro do bit 7 ou 15

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
  uint8_t a {0x01};
  uint8_t b {0x00};
  uint8_t c {0x13};
  uint8_t d {0x00};
  uint8_t e {0xD8};
  uint8_t f {0xB0};
  uint8_t h {0x01};
  uint8_t l {0x4D};

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

struct CPU{
  Memorybus bus;
  Registradores registradores;
  uint16_t pc {0x0100}; 
  uint16_t sp {0xFFFE}; 
  uint16_t last_instruct {};
  int16_t pausa_ciclos {};
  uint8_t irq_latch {};
  uint8_t ime_count {};
  uint8_t ciclos_esperados {};
  bool skipa_fetch {false};
  bool halted {false};
  bool haltbug {false};
  bool stepping {true};
  bool ime {false};
  uint8_t modo {}; //0: DMG only, 1: DMG e CGB, 2: CGB only
  int8_t pausa_offset {};
  uint8_t hack {};
  
  CPU(Timer *tm, Joypad *jp, PPU *p): bus(tm, jp, p) {}

  void step(void);
  void check(void);
  bool check_joypad(void);

  void push(reg_target alvo);
  void pop(reg_target alvo);
  void push(uint16_t valor);
  uint16_t pop(void);
  void call(uint16_t endereco);
  void ret(void);

  void attend_irq(void);
  void jump_vblank(void);
  void jump_serial(void);
  void jump_timer(void);
  void jump_lcdstat(void);
  void jump_joypad(void);

  void incrementa_pc(void);
  uint8_t& get_ie(void) { return bus.memoria[0xFFFF]; }
  uint8_t& get_if(void) { return bus.memoria[0xFF0F]; }
  uint8_t& get_joypad(void) { return bus.memoria[0xFF00]; }
  uint8_t& get_target_ref(reg_target alvo);
  uint8_t get_target_value(reg_target alvo);
  uint16_t get_target_duplo(reg_target alvo);
  uint8_t get_bit(reg_target alvo, uint8_t bit);
};

struct Action{
  void (*execute)(const Action&, CPU*);
  uint8_t tamanho;
  reg_target alvo;
  uint16_t N;
  uint8_t bit_index;
  reg_target ld_alvo;
  bool bus_address;

  Action(void (*ptr)(const Action&, CPU*), uint8_t tam, reg_target a = reg_target::NULO, uint16_t n = 0, uint8_t b = 0, reg_target ld = reg_target::NULO, bool ba = false): 
    execute{ptr}, tamanho{tam}, alvo{a}, N{n}, bit_index{b}, ld_alvo {ld}, bus_address {ba} {}
};

Action le_byte(uint8_t byte, CPU *atual);
Action le_byte_cb(uint8_t byte, CPU *atual);
void roda_cpu(CPU *atual);
void roda_perifericos(CPU *atual, Timer *timer, PPU *ppu);

}

#endif 
