#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <array>
#include <stdexcept>
#include "joypad.h"

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

struct Memorybus{
  mutable std::array<uint8_t, 0xFFFF + 1> memoria{};
  uint16_t *div_count;
  Joypad *pad;

  Memorybus(uint16_t *div): div_count{div} {}

  uint8_t& read_byte(uint16_t endereco){
    switch(endereco){
        case 0xFF07: //tac
          memoria[endereco] |= 0b11111000; 
          break;
        case 0xFF0F: //if
          memoria[endereco] |= 0b11100000; 
          break;
        case 0xFF00: //joypad
          return pad->get_output(); 
        case 0xFF41: //stat
          memoria[endereco] |= 0b10000000; 
          break;
        default: break;
    }
    return memoria[endereco];
  }

  void write_byte(uint16_t endereco, uint8_t valor){
    if(endereco == 0xFF04){ //div
      memoria[endereco] = 0;
      *div_count = 0;
      return;
    }
    if(endereco == 0xFF00){ //joypad
      memoria[0xFF00] = (memoria[0xFF00] & 0x0F) | (valor & 0x30);
      return;
    }
    if(endereco == 0xFF02 && (valor & 0x81) == 0x81){ //serial
      memoria[0xFF01] = 0xFF;
      memoria[0xFF02] &= ~BIT_SERIAL;
      memoria[0xFF0F] |= BIT_SERIAL;
    }
    memoria[endereco] = valor;
  }
};

struct Timer{
    uint16_t div_count {0xAC00};
    uint16_t tima_count {};
    bool timaoverflow {false};

    void step(uint8_t ciclos, Memorybus& bus){

      if(timaoverflow){
        bus.read_byte(0xFF05) = bus.read_byte(0xFF06);
        bus.read_byte(0xFF0F) |= BIT_TIMER;
        timaoverflow = false;
      }

      div_count+=ciclos;
      bus.read_byte(0xFF04) = this->get_div();

      uint8_t tac = bus.read_byte(0xFF07);
      if(!(tac & 0x04)) return;

      uint16_t limite{};
      switch(tac & 0x03){
        case 0x00:
          limite = 1024;
          break;
        case 0x01:
          limite = 16;
          break;
        case 0x02:
          limite = 64;
          break;
        case 0x03:
          limite = 256;
          break;
      }

      tima_count+=ciclos;
      if(tima_count >= limite){
        tima_count -= limite;
        uint8_t tima = bus.read_byte(0xFF05);
        if(tima == 0xFF){
          bus.read_byte(0xFF05) = 0;
          timaoverflow = true;
        }
        else
          ++bus.read_byte(0xFF05);
      }
    }

    uint8_t get_div(void) { return static_cast<uint8_t>((div_count >> 8) & 0xFF); }
  };

struct CPU{
  Memorybus bus;
  Registradores registradores;
  uint16_t pc {0x0100}; 
  uint16_t sp {0xFFFE}; 
  uint8_t last_ticks {};
  bool jp_flag {false};
  bool halted {false};
  bool haltbug {false};
  bool stepping {true};
  bool ime {false};
  bool ime_ie {false};
  
  CPU(uint16_t *div, Joypad *jp): bus(div, jp) {}

  void step(Timer& timer);
  void check(void);
  bool check_joypad(void);

  void push(reg_target alvo);
  void pop(reg_target alvo);
  void push(uint16_t valor);
  uint16_t pop(void);
  void call(uint16_t endereco);
  void ret(void);

  void jump_vblank(void);
  void jump_serial(void);
  void jump_timer(void);
  void jump_lcdstat(void);
  void jump_joypad(void);

  uint8_t& get_ie(void) { return bus.read_byte(0xFFFF); }
  uint8_t& get_if(void) { return bus.read_byte(0xFF0F); }
  uint8_t& get_joypad(void) { return bus.read_byte(0xFF00); }
  uint8_t& get_target(reg_target alvo);
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

  Action(void (*ptr)(const Action&, CPU*), uint8_t tam, reg_target a = reg_target::NULO, uint16_t n = 0, uint8_t b = 0, reg_target ld = reg_target::NULO): 
    execute{ptr}, tamanho{tam}, alvo{a}, N{n}, bit_index{b}, ld_alvo {ld} {}
};

Action le_byte(uint8_t byte, CPU *atual);
Action le_byte_cb(uint8_t byte, CPU *atual);
void roda_cpu(CPU *atual, Timer& timer);

}
#endif 
