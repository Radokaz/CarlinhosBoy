#ifndef CPU_H
#define CPU_H

#include <cstddef>
#include <cstdint>

#define BIT_ZERO (1 << 7) //quando o resultado de uma instrução é zero
#define BIT_SUBTRACT (1 << 6) //quando a ultima instrução foi uma subtração
#define BIT_HALFCARRY (1 << 5) //quando há estouro do bit 3 ou 11
#define BIT_CARRY (1 << 4) //quando há estouro do bit 7 ou 15

enum class reg_target{
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
  HL
};

enum class Instrucoes{
  ADD, //(adição) - soma o valor de um registrador específico com o do registrador A
  ADDHL, //(add to HL) - just like ADD except that the target is added to the HL register
  ADC, //(add with carry) - just like ADD except that the value of the carry flag is also added to the number
  SUB, //(subtract) - subtract the value stored in a specific register with the value in the A register
  SBC, //(subtract with carry) - just like ADD except that the value of the carry flag is also subtracted from the number
  AND, //(logical and) - do a bitwise and on the value in a specific register and the value in the A register
  OR, //(logical or) - do a bitwise or on the value in a specific register and the value in the A register
  XOR, //(logical xor) - do a bitwise xor on the value in a specific register and the value in the A register
  CP, //(compare) - just like SUB except the result of the subtraction is not stored back into A
  INC, //(increment) - increment the value in a specific register by 1
  DEC, //(decrement) - decrement the value in a specific register by 1
  CCF, //(complement carry flag) - toggle the value of the carry flag
  SCF, //(set carry flag) - set the carry flag to true
  RRA, //(rotate right A register) - bit rotate A register right through the carry flag
  RLA, //(rotate left A register) - bit rotate A register left through the carry flag
  RRCA, //(rotate right A register) - bit rotate A register right (not through the carry flag)
  RRLA, //(rotate left A register) - bit rotate A register left (not through the carry flag)
  CPL, //(complement) - toggle every bit of the A register
  BIT, //(bit test) - test to see if a specific bit of a specific register is set
  RESET, //(bit reset) - set a specific bit of a specific register to 0
  SET, //(bit set) - set a specific bit of a specific register to 1
  SRL, //(shift right logical) - bit shift a specific register right by 1
  RR, //(rotate right) - bit rotate a specific register right by 1 through the carry flag
  RL, //(rotate left) - bit rotate a specific register left by 1 through the carry flag
  RRC, //(rorate right) - bit rotate a specific register right by 1 (not through the carry flag)
  RLC, //(rorate left) - bit rotate a specific register left by 1 (not through the carry flag)
  SRA, //(shift right arithmetic) - arithmetic shift a specific register right by 1
  SLA, //(shift left arithmetic) - arithmetic shift a specific register left by 1
  SWAP, //(swap nibbles) - switch upper and lower nibble of a specific register
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
      case reg_16::AF:
        return ((static_cast<uint16_t>(a) << 8) | static_cast<uint16_t>(f));
      case reg_16::BC:
        return ((static_cast<uint16_t>(b) << 8) | static_cast<uint16_t>(c));
      case reg_16::DE:
        return ((static_cast<uint16_t>(d) << 8) | static_cast<uint16_t>(e));
      case reg_16::HL:
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

  uint8_t read_byte(uint16_t endereco){
    return memoria[endereco];
  }
};

struct CPU{
  Registradores registradores;
  uint16_t pc; //program counter
  Memorybus bus;
  
  void execute(Instrucoes atual, reg_target alvo, uint8_t bit_index = 0);

  uint8_t& get_target(reg_target alvo);
  uint16_t get_target_duplo(reg_target alvo) const;
  uint8_t get_bit(reg_target alvo, uint8_t bit) const;
};

#endif 
