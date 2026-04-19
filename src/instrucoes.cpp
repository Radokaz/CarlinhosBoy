#include "cpu.h"

Action le_byte(uint8_t byte, CPU *atual){
  switch(byte){
    using enum reg_target;
    using enum Instrucoes;
    case 0xCB:
      return le_byte_cb(atual->bus.read_byte(atual->pc + 1), atual);
    case 0x03:
      return Action(INCDUP, 1, BC);
    case 0x13:
      return Action(INCDUP, 1, DE);
    case 0x20:
      return Action(JRNZERO, 2);
    case 0x30:
      return Action(JRNCARRY, 2);
    case 0x23:
      return Action(INCDUP, 1, HL);
    case 0x33:
      return Action(INCDUP, 1, SP);
    case 0x04:
      return Action(INC, 1, B);
    case 0x14:
      return Action(INC, 1, D);
    case 0x24:
      return Action(INC, 1, H);
    case 0x34:
      return Action(INC, 1, HL);
    case 0x05:
      return Action(DEC, 1, B);
    case 0x15:
      return Action(DEC, 1, D);
    case 0x25:
      return Action(DEC, 1, H);
    case 0x35:
      return Action(DEC, 1, HL);
    case 0x07:
      return Action(RLCA, 1);
    case 0x17:
      return Action(RLA, 1);
    case 0x37:
      return Action(SCF, 1);
    case 0x18:
      return Action(JRALWAYS, 2);
    case 0x28:
      return Action(JRZERO, 2);
    case 0x38:
      return Action(JRCARRY, 2);
    case 0x09:
      return Action(ADDHL, 1, BC);
    case 0x19:
      return Action(ADDHL, 1, DE);
    case 0x29:
      return Action(ADDHL, 1, HL);
    case 0x39:
      return Action(ADDHL, 1, SP);
    case 0x0B:
      return Action(DECDUP, 1, BC);
    case 0x1B:
      return Action(DECDUP, 1, DE);
    case 0x2B:
      return Action(DECDUP, 1, HL);
    case 0x3B:
      return Action(DECDUP, 1, SP);
    case 0x0C:
      return Action(INC, 1, C);
    case 0x1C:
      return Action(INC, 1, E);
    case 0x2C:
      return Action(INC, 1, L);
    case 0x3C:
      return Action(INC, 1, A);
    case 0x0D:
      return Action(DEC, 1, C);
    case 0x1D:
      return Action(DEC, 1, E);
    case 0x2D:
      return Action(DEC, 1, L);
    case 0x3D:
      return Action(DEC, 1, A);
    case 0x0F:
      return Action(RRCA, 1);
    case 0x1F:
      return Action(RRA, 1);
    case 0x2F:
      return Action(CPL, 1);
    case 0x3F:
      return Action(CCF, 1);
    case 0x80:
      return Action(ADD, 1, B);
    case 0x81:
      return Action(ADD, 1, C);
    case 0x82:
      return Action(ADD, 1, D);
    case 0x83:
      return Action(ADD, 1, E);
    case 0x84:
      return Action(ADD, 1, H);
    case 0x85:
      return Action(ADD, 1, L);
    case 0x86:
      return Action(ADD, 1, HL);
    case 0x87:
      return Action(ADD, 1, A);
    case 0x88:
      return Action(ADC, 1, B);
    case 0x89:
      return Action(ADC, 1, C);
    case 0x8A:
      return Action(ADC, 1, D);
    case 0x8B:
      return Action(ADC, 1, E);
    case 0x8C:
      return Action(ADC, 1, H);
    case 0x8D:
      return Action(ADC, 1, L);
    case 0x8E:
      return Action(ADC, 1, HL);
    case 0x8F:
      return Action(ADC, 1, A);
    case 0x90:
      return Action(SUB, 1, B);
    case 0x91:
      return Action(SUB, 1, C);
    case 0x92:
      return Action(SUB, 1, D);
    case 0x93:
      return Action(SUB, 1, E);
    case 0x94:
      return Action(SUB, 1, H);
    case 0x95:
      return Action(SUB, 1, L);
    case 0x96:
      return Action(SUB, 1, HL);
    case 0x97:
      return Action(SUB, 1, A);
    case 0x98:
      return Action(SBC, 1, B);
    case 0x99:
      return Action(SBC, 1, C);
    case 0x9A:
      return Action(SBC, 1, D);
    case 0x9B:
      return Action(SBC, 1, E);
    case 0x9C:
      return Action(SBC, 1, H);
    case 0x9D:
      return Action(SBC, 1, L);
    case 0x9E:
      return Action(SBC, 1, HL);
    case 0x9F:
      return Action(SBC, 1, A);
    case 0xA0:
      return Action(AND, 1, B);
    case 0xA1:
      return Action(AND, 1, C);
    case 0xA2:
      return Action(AND, 1, D);
    case 0xA3:
      return Action(AND, 1, E);
    case 0xA4:
      return Action(AND, 1, H);
    case 0xA5:
      return Action(AND, 1, L);
    case 0xA6:
      return Action(AND, 1, HL);
    case 0xA7:
      return Action(AND, 1, A);
    case 0xA8:
      return Action(XOR, 1, B);
    case 0xA9:
      return Action(XOR, 1, C);
    case 0xAA:
      return Action(XOR, 1, D);
    case 0xAB:
      return Action(XOR, 1, E);
    case 0xAC:
      return Action(XOR, 1, H);
    case 0xAD:
      return Action(XOR, 1, L);
    case 0xAE:
      return Action(XOR, 1, HL);
    case 0xAF:
      return Action(XOR, 1, A);
    case 0xB0:
      return Action(OR, 1, B);
    case 0xB1:
      return Action(OR, 1, C);
    case 0xB2:
      return Action(OR, 1, D);
    case 0xB3:
      return Action(OR, 1, E);
    case 0xB4:
      return Action(OR, 1, H);
    case 0xB5:
      return Action(OR, 1, L);
    case 0xB6:
      return Action(OR, 1, HL);
    case 0xB7:
      return Action(OR, 1, A);
    case 0xB8:
      return Action(CP, 1, B);
    case 0xB9:
      return Action(CP, 1, C);
    case 0xBA:
      return Action(CP, 1, D);
    case 0xBB:
      return Action(CP, 1, E);
    case 0xBC:
      return Action(CP, 1, H);
    case 0xBD:
      return Action(CP, 1, L);
    case 0xBE:
      return Action(CP, 1, HL);
    case 0xBF:
      return Action(CP, 1, A);
    case 0xC2:
      return Action(JPNZERO, 3);
    case 0xD2:
      return Action(JPNCARRY, 3);
    case 0xC3:
      return Action(JPALWAYS, 3);
    case 0xC6:
      return Action(ADD, 2, n, atual->bus.read_byte(atual->pc + 1));
    case 0xD6:
      return Action(SUB, 2, n, atual->bus.read_byte(atual->pc + 1));
    case 0xE6:
      return Action(AND, 2, n, atual->bus.read_byte(atual->pc + 1));
    case 0xF6:
      return Action(OR, 2, n, atual->bus.read_byte(atual->pc + 1));
    case 0xE9:
      return Action(JPALWAYS, 3, HL);
    case 0xCA:
      return Action(JPZERO, 3);
    case 0xDA:
      return Action(JPCARRY, 3);
    case 0xCE:
      return Action(ADC, 2, n, atual->bus.read_byte(atual->pc + 1));
    case 0xDE:
      return Action(SBC, 2, n, atual->bus.read_byte(atual->pc + 1));
    case 0xEE:
      return Action(XOR, 2, n, atual->bus.read_byte(atual->pc + 1));
    case 0xFE:
      return Action(CP, 2, n, atual->bus.read_byte(atual->pc + 1));
    default:
      throw std::runtime_error("Endereço inválido.\n");
  }
}

Action le_byte_cb(uint8_t byte, CPU *atual){
  switch(byte){
    using enum reg_target;
    using enum Instrucoes;

    case 0x00:
      return Action(RLC, 2, B);
    case 0x01:
      return Action(RLC, 2, C);
    case 0x02:
      return Action(RLC, 2, D);
    case 0x03:
      return Action(RLC, 2, E);
    case 0x04:
      return Action(RLC, 2, H);
    case 0x05:
      return Action(RLC, 2, L);
    case 0x06:
      return Action(RLC, 2, HL);
    case 0x07:
      return Action(RLC, 2, A);
    case 0x08:
      return Action(RRC, 2, B);
    case 0x09:
      return Action(RRC, 2, C);
    case 0x0A:
      return Action(RRC, 2, D);
    case 0x0B:
      return Action(RRC, 2, E);
    case 0x0C:
      return Action(RRC, 2, H);
    case 0x0D:
      return Action(RRC, 2, L);
    case 0x0E:
      return Action(RRC, 2, HL);
    case 0x0F:
      return Action(RRC, 2, A);

    case 0x10:
      return Action(RL, 2, B);
    case 0x11:
      return Action(RL, 2, C);
    case 0x12:
      return Action(RL, 2, D);
    case 0x13:
      return Action(RL, 2, E);
    case 0x14:
      return Action(RL, 2, H);
    case 0x15:
      return Action(RL, 2, L);
    case 0x16:
      return Action(RL, 2, HL);
    case 0x17:
      return Action(RL, 2, A);
    case 0x18:
      return Action(RR, 2, B);
    case 0x19:
      return Action(RR, 2, C);
    case 0x1A:
      return Action(RR, 2, D);
    case 0x1B:
      return Action(RR, 2, E);
    case 0x1C:
      return Action(RR, 2, H);
    case 0x1D:
      return Action(RR, 2, L);
    case 0x1E:
      return Action(RR, 2, HL);
    case 0x1F:
      return Action(RR, 2, A);

    case 0x20:
      return Action(SLA, 2, B);
    case 0x21:
      return Action(SLA, 2, C);
    case 0x22:
      return Action(SLA, 2, D);
    case 0x23:
      return Action(SLA, 2, E);
    case 0x24:
      return Action(SLA, 2, H);
    case 0x25:
      return Action(SLA, 2, L);
    case 0x26:
      return Action(SLA, 2, HL);
    case 0x27:
      return Action(SLA, 2, A);
    case 0x28:
      return Action(SRA, 2, B);
    case 0x29:
      return Action(SRA, 2, C);
    case 0x2A:
      return Action(SRA, 2, D);
    case 0x2B:
      return Action(SRA, 2, E);
    case 0x2C:
      return Action(SRA, 2, H);
    case 0x2D:
      return Action(SRA, 2, L);
    case 0x2E:
      return Action(SRA, 2, HL);
    case 0x2F:
      return Action(SRA, 2, A);

    case 0x30:
      return Action(SWAP, 2, B);
    case 0x31:
      return Action(SWAP, 2, C);
    case 0x32:
      return Action(SWAP, 2, D);
    case 0x33:
      return Action(SWAP, 2, E);
    case 0x34:
      return Action(SWAP, 2, H);
    case 0x35:
      return Action(SWAP, 2, L);
    case 0x36:
      return Action(SWAP, 2, HL);
    case 0x37:
      return Action(SWAP, 2, A);
    case 0x38:
      return Action(SRL, 2, B);
    case 0x39:
      return Action(SRL, 2, C);
    case 0x3A:
      return Action(SRL, 2, D);
    case 0x3B:
      return Action(SRL, 2, E);
    case 0x3C:
      return Action(SRL, 2, H);
    case 0x3D:
      return Action(SRL, 2, L);
    case 0x3E:
      return Action(SRL, 2, HL);
    case 0x3F:
      return Action(SRL, 2, A);

    case 0x40:
      return Action(BIT, 2, B);
    case 0x41:
      return Action(BIT, 2, C);
    case 0x42:
      return Action(BIT, 2, D);
    case 0x43:
      return Action(BIT, 2, E);
    case 0x44:
      return Action(BIT, 2, H);
    case 0x45:
      return Action(BIT, 2, L);
    case 0x46:
      return Action(BIT, 2, HL);
    case 0x47:
      return Action(BIT, 2, A);
    case 0x48:
      return Action(BIT, 2, B, 0, 1);
    case 0x49:
      return Action(BIT, 2, C, 0, 1);
    case 0x4A:
      return Action(BIT, 2, D, 0, 1);
    case 0x4B:
      return Action(BIT, 2, E, 0, 1);
    case 0x4C:
      return Action(BIT, 2, H, 0, 1);
    case 0x4D:
      return Action(BIT, 2, L, 0, 1);
    case 0x4E:
      return Action(BIT, 2, HL, 0, 1);
    case 0x4F:
      return Action(BIT, 2, A, 0, 1);

    case 0x50:
      return Action(BIT, 2, B, 0 ,2);
    case 0x51:
      return Action(BIT, 2, C, 0, 2);
    case 0x52:
      return Action(BIT, 2, D, 0, 2);
    case 0x53:
      return Action(BIT, 2, E, 0, 2);
    case 0x54:
      return Action(BIT, 2, H, 0, 2);
    case 0x55:
      return Action(BIT, 2, L, 0, 2);
    case 0x56:
      return Action(BIT, 2, HL, 0, 2);
    case 0x57:
      return Action(BIT, 2, A, 0, 2);
    case 0x58:
      return Action(BIT, 2, B, 0, 3);
    case 0x59:
      return Action(BIT, 2, C, 0, 3);
    case 0x5A:
      return Action(BIT, 2, D, 0, 3);
    case 0x5B:
      return Action(BIT, 2, E, 0, 3);
    case 0x5C:
      return Action(BIT, 2, H, 0, 3);
    case 0x5D:
      return Action(BIT, 2, L, 0, 3);
    case 0x5E:
      return Action(BIT, 2, HL, 0, 3);
    case 0x5F:
      return Action(BIT, 2, A, 0, 3);

    case 0x60:
      return Action(BIT, 2, B, 0 ,4);
    case 0x61:
      return Action(BIT, 2, C, 0, 4);
    case 0x62:
      return Action(BIT, 2, D, 0, 4);
    case 0x63:
      return Action(BIT, 2, E, 0, 4);
    case 0x64:
      return Action(BIT, 2, H, 0, 4);
    case 0x65:
      return Action(BIT, 2, L, 0, 4);
    case 0x66:
      return Action(BIT, 2, HL, 0, 4);
    case 0x67:
      return Action(BIT, 2, A, 0, 4);
    case 0x68:
      return Action(BIT, 2, B, 0, 5);
    case 0x69:
      return Action(BIT, 2, C, 0, 5);
    case 0x6A:
      return Action(BIT, 2, D, 0, 5);
    case 0x6B:
      return Action(BIT, 2, E, 0, 5);
    case 0x6C:
      return Action(BIT, 2, H, 0, 5);
    case 0x6D:
      return Action(BIT, 2, L, 0, 5);
    case 0x6E:
      return Action(BIT, 2, HL, 0, 5);
    case 0x6F:
      return Action(BIT, 2, A, 0, 5);

    case 0x70:
      return Action(BIT, 2, B, 0 ,6);
    case 0x71:
      return Action(BIT, 2, C, 0, 6);
    case 0x72:
      return Action(BIT, 2, D, 0, 6);
    case 0x73:
      return Action(BIT, 2, E, 0, 6);
    case 0x74:
      return Action(BIT, 2, H, 0, 6);
    case 0x75:
      return Action(BIT, 2, L, 0, 6);
    case 0x76:
      return Action(BIT, 2, HL, 0, 6);
    case 0x77:
      return Action(BIT, 2, A, 0, 6);
    case 0x78:
      return Action(BIT, 2, B, 0, 7);
    case 0x79:
      return Action(BIT, 2, C, 0, 7);
    case 0x7A:
      return Action(BIT, 2, D, 0, 7);
    case 0x7B:
      return Action(BIT, 2, E, 0, 7);
    case 0x7C:
      return Action(BIT, 2, H, 0, 7);
    case 0x7D:
      return Action(BIT, 2, L, 0, 7);
    case 0x7E:
      return Action(BIT, 2, HL, 0, 7);
    case 0x7F:
      return Action(BIT, 2, A, 0, 7);

    case 0x80:
      return Action(RESET, 2, B);
    case 0x81:
      return Action(RESET, 2, C);
    case 0x82:
      return Action(RESET, 2, D);
    case 0x83:
      return Action(RESET, 2, E);
    case 0x84:
      return Action(RESET, 2, H);
    case 0x85:
      return Action(RESET, 2, L);
    case 0x86:
      return Action(RESET, 2, HL);
    case 0x87:
      return Action(RESET, 2, A);
    case 0x88:
      return Action(RESET, 2, B, 0, 1);
    case 0x89:
      return Action(RESET, 2, C, 0, 1);
    case 0x8A:
      return Action(RESET, 2, D, 0, 1);
    case 0x8B:
      return Action(RESET, 2, E, 0, 1);
    case 0x8C:
      return Action(RESET, 2, H, 0, 1);
    case 0x8D:
      return Action(RESET, 2, L, 0, 1);
    case 0x8E:
      return Action(RESET, 2, HL, 0, 1);
    case 0x8F:
      return Action(RESET, 2, A, 0, 1);

    case 0x90:
      return Action(RESET, 2, B, 0 ,2);
    case 0x91:
      return Action(RESET, 2, C, 0, 2);
    case 0x92:
      return Action(RESET, 2, D, 0, 2);
    case 0x93:
      return Action(RESET, 2, E, 0, 2);
    case 0x94:
      return Action(RESET, 2, H, 0, 2);
    case 0x95:
      return Action(RESET, 2, L, 0, 2);
    case 0x96:
      return Action(RESET, 2, HL, 0, 2);
    case 0x97:
      return Action(RESET, 2, A, 0, 2);
    case 0x98:
      return Action(RESET, 2, B, 0, 3);
    case 0x99:
      return Action(RESET, 2, C, 0, 3);
    case 0x9A:
      return Action(RESET, 2, D, 0, 3);
    case 0x9B:
      return Action(RESET, 2, E, 0, 3);
    case 0x9C:
      return Action(RESET, 2, H, 0, 3);
    case 0x9D:
      return Action(RESET, 2, L, 0, 3);
    case 0x9E:
      return Action(RESET, 2, HL, 0, 3);
    case 0x9F:
      return Action(RESET, 2, A, 0, 3);

    case 0xA0:
      return Action(RESET, 2, B, 0 ,4);
    case 0xA1:
      return Action(RESET, 2, C, 0, 4);
    case 0xA2:
      return Action(RESET, 2, D, 0, 4);
    case 0xA3:
      return Action(RESET, 2, E, 0, 4);
    case 0xA4:
      return Action(RESET, 2, H, 0, 4);
    case 0xA5:
      return Action(RESET, 2, L, 0, 4);
    case 0xA6:
      return Action(RESET, 2, HL, 0, 4);
    case 0xA7:
      return Action(RESET, 2, A, 0, 4);
    case 0xA8:
      return Action(RESET, 2, B, 0, 5);
    case 0xA9:
      return Action(RESET, 2, C, 0, 5);
    case 0xAA:
      return Action(RESET, 2, D, 0, 5);
    case 0xAB:
      return Action(RESET, 2, E, 0, 5);
    case 0xAC:
      return Action(RESET, 2, H, 0, 5);
    case 0xAD:
      return Action(RESET, 2, L, 0, 5);
    case 0xAE:
      return Action(RESET, 2, HL, 0, 5);
    case 0xAF:
      return Action(RESET, 2, A, 0, 5);

    case 0xB0:
      return Action(RESET, 2, B, 0 ,6);
    case 0xB1:
      return Action(RESET, 2, C, 0, 6);
    case 0xB2:
      return Action(RESET, 2, D, 0, 6);
    case 0xB3:
      return Action(RESET, 2, E, 0, 6);
    case 0xB4:
      return Action(RESET, 2, H, 0, 6);
    case 0xB5:
      return Action(RESET, 2, L, 0, 6);
    case 0xB6:
      return Action(RESET, 2, HL, 0, 6);
    case 0xB7:
      return Action(RESET, 2, A, 0, 6);
    case 0xB8:
      return Action(RESET, 2, B, 0, 7);
    case 0xB9:
      return Action(RESET, 2, C, 0, 7);
    case 0xBA:
      return Action(RESET, 2, D, 0, 7);
    case 0xBB:
      return Action(RESET, 2, E, 0, 7);
    case 0xBC:
      return Action(RESET, 2, H, 0, 7);
    case 0xBD:
      return Action(RESET, 2, L, 0, 7);
    case 0xBE:
      return Action(RESET, 2, HL, 0, 7);
    case 0xBF:
      return Action(RESET, 2, A, 0, 7);

    case 0xC0:
      return Action(SET, 2, B);
    case 0xC1:
      return Action(SET, 2, C);
    case 0xC2:
      return Action(SET, 2, D);
    case 0xC3:
      return Action(SET, 2, E);
    case 0xC4:
      return Action(SET, 2, H);
    case 0xC5:
      return Action(SET, 2, L);
    case 0xC6:
      return Action(SET, 2, HL);
    case 0xC7:
      return Action(SET, 2, A);
    case 0xC8:
      return Action(SET, 2, B, 0, 1);
    case 0xC9:
      return Action(SET, 2, C, 0, 1);
    case 0xCA:
      return Action(SET, 2, D, 0, 1);
    case 0xCB:
      return Action(SET, 2, E, 0, 1);
    case 0xCC:
      return Action(SET, 2, H, 0, 1);
    case 0xCD:
      return Action(SET, 2, L, 0, 1);
    case 0xCE:
      return Action(SET, 2, HL, 0, 1);
    case 0xCF:
      return Action(SET, 2, A, 0, 1);

    case 0xD0:
      return Action(SET, 2, B, 0 ,2);
    case 0xD1:
      return Action(SET, 2, C, 0, 2);
    case 0xD2:
      return Action(SET, 2, D, 0, 2);
    case 0xD3:
      return Action(SET, 2, E, 0, 2);
    case 0xD4:
      return Action(SET, 2, H, 0, 2);
    case 0xD5:
      return Action(SET, 2, L, 0, 2);
    case 0xD6:
      return Action(SET, 2, HL, 0, 2);
    case 0xD7:
      return Action(SET, 2, A, 0, 2);
    case 0xD8:
      return Action(SET, 2, B, 0, 3);
    case 0xD9:
      return Action(SET, 2, C, 0, 3);
    case 0xDA:
      return Action(SET, 2, D, 0, 3);
    case 0xDB:
      return Action(SET, 2, E, 0, 3);
    case 0xDC:
      return Action(SET, 2, H, 0, 3);
    case 0xDD:
      return Action(SET, 2, L, 0, 3);
    case 0xDE:
      return Action(SET, 2, HL, 0, 3);
    case 0xDF:
      return Action(SET, 2, A, 0, 3);

    case 0xE0:
      return Action(SET, 2, B, 0 ,4);
    case 0xE1:
      return Action(SET, 2, C, 0, 4);
    case 0xE2:
      return Action(SET, 2, D, 0, 4);
    case 0xE3:
      return Action(SET, 2, E, 0, 4);
    case 0xE4:
      return Action(SET, 2, H, 0, 4);
    case 0xE5:
      return Action(SET, 2, L, 0, 4);
    case 0xE6:
      return Action(SET, 2, HL, 0, 4);
    case 0xE7:
      return Action(SET, 2, A, 0, 4);
    case 0xE8:
      return Action(SET, 2, B, 0, 5);
    case 0xE9:
      return Action(SET, 2, C, 0, 5);
    case 0xEA:
      return Action(SET, 2, D, 0, 5);
    case 0xEB:
      return Action(SET, 2, E, 0, 5);
    case 0xEC:
      return Action(SET, 2, H, 0, 5);
    case 0xED:
      return Action(SET, 2, L, 0, 5);
    case 0xEE:
      return Action(SET, 2, HL, 0, 5);
    case 0xEF:
      return Action(SET, 2, A, 0, 5);

    case 0xF0:
      return Action(SET, 2, B, 0 ,6);
    case 0xF1:
      return Action(SET, 2, C, 0, 6);
    case 0xF2:
      return Action(SET, 2, D, 0, 6);
    case 0xF3:
      return Action(SET, 2, E, 0, 6);
    case 0xF4:
      return Action(SET, 2, H, 0, 6);
    case 0xF5:
      return Action(SET, 2, L, 0, 6);
    case 0xF6:
      return Action(SET, 2, HL, 0, 6);
    case 0xF7:
      return Action(SET, 2, A, 0, 6);
    case 0xF8:
      return Action(SET, 2, B, 0, 7);
    case 0xF9:
      return Action(SET, 2, C, 0, 7);
    case 0xFA:
      return Action(SET, 2, D, 0, 7);
    case 0xFB:
      return Action(SET, 2, E, 0, 7);
    case 0xFC:
      return Action(SET, 2, H, 0, 7);
    case 0xFD:
      return Action(SET, 2, L, 0, 7);
    case 0xFE:
      return Action(SET, 2, HL, 0, 7);
    case 0xFF:
      return Action(SET, 2, A, 0, 7);

    default:
      throw std::runtime_error("Endereço inválido.\n");
  }
}
