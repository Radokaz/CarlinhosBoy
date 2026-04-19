#include "cpu.h"

Action le_byte(uint8_t byte, CPU *atual){
  switch(byte){
    using enum reg_target;
    using enum Instrucoes;
    case 0xCB:
      return le_byte_cb(atual->bus.read_byte(atual->pc + 1), atual);
    case 0x03:
      return Action(INCDUP, BC);
    case 0x13:
      return Action(INCDUP, DE);
    case 0x23:
      return Action(INCDUP, HL);
    case 0x33:
      return Action(INCDUP, SP);
    case 0x04:
      return Action(INC, B);
    case 0x14:
      return Action(INC, D);
    case 0x24:
      return Action(INC, H);
    case 0x34:
      return Action(INC, HL);
    case 0x05:
      return Action(DEC, B);
    case 0x15:
      return Action(DEC, D);
    case 0x25:
      return Action(DEC, H);
    case 0x35:
      return Action(DEC, HL);
    case 0x07:
      return Action(RLCA);
    case 0x17:
      return Action(RLA);
    case 0x37:
      return Action(SCF);
    case 0x09:
      return Action(ADDHL, BC);
    case 0x19:
      return Action(ADDHL, DE);
    case 0x29:
      return Action(ADDHL, HL);
    case 0x39:
      return Action(ADDHL, SP);
    case 0x0B:
      return Action(DECDUP, BC);
    case 0x1B:
      return Action(DECDUP, DE);
    case 0x2B:
      return Action(DECDUP, HL);
    case 0x3B:
      return Action(DECDUP, SP);
    case 0x0C:
      return Action(INC, C);
    case 0x1C:
      return Action(INC, E);
    case 0x2C:
      return Action(INC, L);
    case 0x3C:
      return Action(INC, A);
    case 0x0D:
      return Action(DEC, C);
    case 0x1D:
      return Action(DEC, E);
    case 0x2D:
      return Action(DEC, L);
    case 0x3D:
      return Action(DEC, A);
    case 0x0F:
      return Action(RRCA);
    case 0x1F:
      return Action(RRA);
    case 0x2F:
      return Action(CPL);
    case 0x3F:
      return Action(CCF);
    case 0x80:
      return Action(ADD, B);
    case 0x81:
      return Action(ADD, C);
    case 0x82:
      return Action(ADD, D);
    case 0x83:
      return Action(ADD, E);
    case 0x84:
      return Action(ADD, H);
    case 0x85:
      return Action(ADD, L);
    case 0x86:
      return Action(ADD, HL);
    case 0x87:
      return Action(ADD, A);
    case 0x88:
      return Action(ADC, B);
    case 0x89:
      return Action(ADC, C);
    case 0x8A:
      return Action(ADC, D);
    case 0x8B:
      return Action(ADC, E);
    case 0x8C:
      return Action(ADC, H);
    case 0x8D:
      return Action(ADC, L);
    case 0x8E:
      return Action(ADC, HL);
    case 0x8F:
      return Action(ADC, A);
    case 0x90:
      return Action(SUB, B);
    case 0x91:
      return Action(SUB, C);
    case 0x92:
      return Action(SUB, D);
    case 0x93:
      return Action(SUB, E);
    case 0x94:
      return Action(SUB, H);
    case 0x95:
      return Action(SUB, L);
    case 0x96:
      return Action(SUB, HL);
    case 0x97:
      return Action(SUB, A);
    case 0x98:
      return Action(SBC, B);
    case 0x99:
      return Action(SBC, C);
    case 0x9A:
      return Action(SBC, D);
    case 0x9B:
      return Action(SBC, E);
    case 0x9C:
      return Action(SBC, H);
    case 0x9D:
      return Action(SBC, L);
    case 0x9E:
      return Action(SBC, HL);
    case 0x9F:
      return Action(SBC, A);
    case 0xA0:
      return Action(AND, B);
    case 0xA1:
      return Action(AND, C);
    case 0xA2:
      return Action(AND, D);
    case 0xA3:
      return Action(AND, E);
    case 0xA4:
      return Action(AND, H);
    case 0xA5:
      return Action(AND, L);
    case 0xA6:
      return Action(AND, HL);
    case 0xA7:
      return Action(AND, A);
    case 0xA8:
      return Action(XOR, B);
    case 0xA9:
      return Action(XOR, C);
    case 0xAA:
      return Action(XOR, D);
    case 0xAB:
      return Action(XOR, E);
    case 0xAC:
      return Action(XOR, H);
    case 0xAD:
      return Action(XOR, L);
    case 0xAE:
      return Action(XOR, HL);
    case 0xAF:
      return Action(XOR, A);
    case 0xB0:
      return Action(OR, B);
    case 0xB1:
      return Action(OR, C);
    case 0xB2:
      return Action(OR, D);
    case 0xB3:
      return Action(OR, E);
    case 0xB4:
      return Action(OR, H);
    case 0xB5:
      return Action(OR, L);
    case 0xB6:
      return Action(OR, HL);
    case 0xB7:
      return Action(OR, A);
    case 0xB8:
      return Action(CP, B);
    case 0xB9:
      return Action(CP, C);
    case 0xBA:
      return Action(CP, D);
    case 0xBB:
      return Action(CP, E);
    case 0xBC:
      return Action(CP, H);
    case 0xBD:
      return Action(CP, L);
    case 0xBE:
      return Action(CP, HL);
    case 0xBF:
      return Action(CP, A);
    case 0xC6:
      return Action(ADD, n, atual->bus.read_byte(atual->pc + 1));
    case 0xD6:
      return Action(SUB, n, atual->bus.read_byte(atual->pc + 1));
    case 0xE6:
      return Action(AND, n, atual->bus.read_byte(atual->pc + 1));
    case 0xF6:
      return Action(OR, n, atual->bus.read_byte(atual->pc + 1));
    case 0xCE:
      return Action(ADC, n, atual->bus.read_byte(atual->pc + 1));
    case 0xDE:
      return Action(SBC, n, atual->bus.read_byte(atual->pc + 1));
    case 0xEE:
      return Action(XOR, n, atual->bus.read_byte(atual->pc + 1));
    case 0xFE:
      return Action(CP, n, atual->bus.read_byte(atual->pc + 1));
  }
}

Action le_byte_cb(uint8_t byte, CPU *atual){
  switch(byte){
    using enum reg_target;
    using enum Instrucoes;

    case 0x00:
      return Action(RLC, B);
    case 0x01:
      return Action(RLC, C);
    case 0x02:
      return Action(RLC, D);
    case 0x03:
      return Action(RLC, E);
    case 0x04:
      return Action(RLC, H);
    case 0x05:
      return Action(RLC, L);
    case 0x06:
      return Action(RLC, HL);
    case 0x07:
      return Action(RLC, A);
    case 0x08:
      return Action(RRC, B);
    case 0x09:
      return Action(RRC, C);
    case 0x0A:
      return Action(RRC, D);
    case 0x0B:
      return Action(RRC, E);
    case 0x0C:
      return Action(RRC, H);
    case 0x0D:
      return Action(RRC, L);
    case 0x0E:
      return Action(RRC, HL);
    case 0x0F:
      return Action(RRC, A);

    case 0x10:
      return Action(RL, B);
    case 0x11:
      return Action(RL, C);
    case 0x12:
      return Action(RL, D);
    case 0x13:
      return Action(RL, E);
    case 0x14:
      return Action(RL, H);
    case 0x15:
      return Action(RL, L);
    case 0x16:
      return Action(RL, HL);
    case 0x17:
      return Action(RL, A);
    case 0x18:
      return Action(RR, B);
    case 0x19:
      return Action(RR, C);
    case 0x1A:
      return Action(RR, D);
    case 0x1B:
      return Action(RR, E);
    case 0x1C:
      return Action(RR, H);
    case 0x1D:
      return Action(RR, L);
    case 0x1E:
      return Action(RR, HL);
    case 0x1F:
      return Action(RR, A);

    case 0x20:
      return Action(SLA, B);
    case 0x21:
      return Action(SLA, C);
    case 0x22:
      return Action(SLA, D);
    case 0x23:
      return Action(SLA, E);
    case 0x24:
      return Action(SLA, H);
    case 0x25:
      return Action(SLA, L);
    case 0x26:
      return Action(SLA, HL);
    case 0x27:
      return Action(SLA, A);
    case 0x28:
      return Action(SRA, B);
    case 0x29:
      return Action(SRA, C);
    case 0x2A:
      return Action(SRA, D);
    case 0x2B:
      return Action(SRA, E);
    case 0x2C:
      return Action(SRA, H);
    case 0x2D:
      return Action(SRA, L);
    case 0x2E:
      return Action(SRA, HL);
    case 0x2F:
      return Action(SRA, A);

    case 0x30:
      return Action(SWAP, B);
    case 0x31:
      return Action(SWAP, C);
    case 0x32:
      return Action(SWAP, D);
    case 0x33:
      return Action(SWAP, E);
    case 0x34:
      return Action(SWAP, H);
    case 0x35:
      return Action(SWAP, L);
    case 0x36:
      return Action(SWAP, HL);
    case 0x37:
      return Action(SWAP, A);
    case 0x38:
      return Action(SRL, B);
    case 0x39:
      return Action(SRL, C);
    case 0x3A:
      return Action(SRL, D);
    case 0x3B:
      return Action(SRL, E);
    case 0x3C:
      return Action(SRL, H);
    case 0x3D:
      return Action(SRL, L);
    case 0x3E:
      return Action(SRL, HL);
    case 0x3F:
      return Action(SRL, A);

    case 0x40:
      return Action(BIT, B);
    case 0x41:
      return Action(BIT, C);
    case 0x42:
      return Action(BIT, D);
    case 0x43:
      return Action(BIT, E);
    case 0x44:
      return Action(BIT, H);
    case 0x45:
      return Action(BIT, L);
    case 0x46:
      return Action(BIT, HL);
    case 0x47:
      return Action(BIT, A);
    case 0x48:
      return Action(BIT, B, 0, 1);
    case 0x49:
      return Action(BIT, C, 0, 1);
    case 0x4A:
      return Action(BIT, D, 0, 1);
    case 0x4B:
      return Action(BIT, E, 0, 1);
    case 0x4C:
      return Action(BIT, H, 0, 1);
    case 0x4D:
      return Action(BIT, L, 0, 1);
    case 0x4E:
      return Action(BIT, HL, 0, 1);
    case 0x4F:
      return Action(BIT, A, 0, 1);

    case 0x50:
      return Action(BIT, B, 0 ,2);
    case 0x51:
      return Action(BIT, C, 0, 2);
    case 0x52:
      return Action(BIT, D, 0, 2);
    case 0x53:
      return Action(BIT, E, 0, 2);
    case 0x54:
      return Action(BIT, H, 0, 2);
    case 0x55:
      return Action(BIT, L, 0, 2);
    case 0x56:
      return Action(BIT, HL, 0, 2);
    case 0x57:
      return Action(BIT, A, 0, 2);
    case 0x58:
      return Action(BIT, B, 0, 3);
    case 0x59:
      return Action(BIT, C, 0, 3);
    case 0x5A:
      return Action(BIT, D, 0, 3);
    case 0x5B:
      return Action(BIT, E, 0, 3);
    case 0x5C:
      return Action(BIT, H, 0, 3);
    case 0x5D:
      return Action(BIT, L, 0, 3);
    case 0x5E:
      return Action(BIT, HL, 0, 3);
    case 0x5F:
      return Action(BIT, A, 0, 3);

    case 0x60:
      return Action(BIT, B, 0 ,4);
    case 0x61:
      return Action(BIT, C, 0, 4);
    case 0x62:
      return Action(BIT, D, 0, 4);
    case 0x63:
      return Action(BIT, E, 0, 4);
    case 0x64:
      return Action(BIT, H, 0, 4);
    case 0x65:
      return Action(BIT, L, 0, 4);
    case 0x66:
      return Action(BIT, HL, 0, 4);
    case 0x67:
      return Action(BIT, A, 0, 4);
    case 0x68:
      return Action(BIT, B, 0, 5);
    case 0x69:
      return Action(BIT, C, 0, 5);
    case 0x6A:
      return Action(BIT, D, 0, 5);
    case 0x6B:
      return Action(BIT, E, 0, 5);
    case 0x6C:
      return Action(BIT, H, 0, 5);
    case 0x6D:
      return Action(BIT, L, 0, 5);
    case 0x6E:
      return Action(BIT, HL, 0, 5);
    case 0x6F:
      return Action(BIT, A, 0, 5);

    case 0x70:
      return Action(BIT, B, 0 ,6);
    case 0x71:
      return Action(BIT, C, 0, 6);
    case 0x72:
      return Action(BIT, D, 0, 6);
    case 0x73:
      return Action(BIT, E, 0, 6);
    case 0x74:
      return Action(BIT, H, 0, 6);
    case 0x75:
      return Action(BIT, L, 0, 6);
    case 0x76:
      return Action(BIT, HL, 0, 6);
    case 0x77:
      return Action(BIT, A, 0, 6);
    case 0x78:
      return Action(BIT, B, 0, 7);
    case 0x79:
      return Action(BIT, C, 0, 7);
    case 0x7A:
      return Action(BIT, D, 0, 7);
    case 0x7B:
      return Action(BIT, E, 0, 7);
    case 0x7C:
      return Action(BIT, H, 0, 7);
    case 0x7D:
      return Action(BIT, L, 0, 7);
    case 0x7E:
      return Action(BIT, HL, 0, 7);
    case 0x7F:
      return Action(BIT, A, 0, 7);

    case 0x80:
      return Action(RESET, B);
    case 0x81:
      return Action(RESET, C);
    case 0x82:
      return Action(RESET, D);
    case 0x83:
      return Action(RESET, E);
    case 0x84:
      return Action(RESET, H);
    case 0x85:
      return Action(RESET, L);
    case 0x86:
      return Action(RESET, HL);
    case 0x87:
      return Action(RESET, A);
    case 0x88:
      return Action(RESET, B, 0, 1);
    case 0x89:
      return Action(RESET, C, 0, 1);
    case 0x8A:
      return Action(RESET, D, 0, 1);
    case 0x8B:
      return Action(RESET, E, 0, 1);
    case 0x8C:
      return Action(RESET, H, 0, 1);
    case 0x8D:
      return Action(RESET, L, 0, 1);
    case 0x8E:
      return Action(RESET, HL, 0, 1);
    case 0x8F:
      return Action(RESET, A, 0, 1);

    case 0x90:
      return Action(RESET, B, 0 ,2);
    case 0x91:
      return Action(RESET, C, 0, 2);
    case 0x92:
      return Action(RESET, D, 0, 2);
    case 0x93:
      return Action(RESET, E, 0, 2);
    case 0x94:
      return Action(RESET, H, 0, 2);
    case 0x95:
      return Action(RESET, L, 0, 2);
    case 0x96:
      return Action(RESET, HL, 0, 2);
    case 0x97:
      return Action(RESET, A, 0, 2);
    case 0x98:
      return Action(RESET, B, 0, 3);
    case 0x99:
      return Action(RESET, C, 0, 3);
    case 0x9A:
      return Action(RESET, D, 0, 3);
    case 0x9B:
      return Action(RESET, E, 0, 3);
    case 0x9C:
      return Action(RESET, H, 0, 3);
    case 0x9D:
      return Action(RESET, L, 0, 3);
    case 0x9E:
      return Action(RESET, HL, 0, 3);
    case 0x9F:
      return Action(RESET, A, 0, 3);

    case 0xA0:
      return Action(RESET, B, 0 ,4);
    case 0xA1:
      return Action(RESET, C, 0, 4);
    case 0xA2:
      return Action(RESET, D, 0, 4);
    case 0xA3:
      return Action(RESET, E, 0, 4);
    case 0xA4:
      return Action(RESET, H, 0, 4);
    case 0xA5:
      return Action(RESET, L, 0, 4);
    case 0xA6:
      return Action(RESET, HL, 0, 4);
    case 0xA7:
      return Action(RESET, A, 0, 4);
    case 0xA8:
      return Action(RESET, B, 0, 5);
    case 0xA9:
      return Action(RESET, C, 0, 5);
    case 0xAA:
      return Action(RESET, D, 0, 5);
    case 0xAB:
      return Action(RESET, E, 0, 5);
    case 0xAC:
      return Action(RESET, H, 0, 5);
    case 0xAD:
      return Action(RESET, L, 0, 5);
    case 0xAE:
      return Action(RESET, HL, 0, 5);
    case 0xAF:
      return Action(RESET, A, 0, 5);

    case 0xB0:
      return Action(RESET, B, 0 ,6);
    case 0xB1:
      return Action(RESET, C, 0, 6);
    case 0xB2:
      return Action(RESET, D, 0, 6);
    case 0xB3:
      return Action(RESET, E, 0, 6);
    case 0xB4:
      return Action(RESET, H, 0, 6);
    case 0xB5:
      return Action(RESET, L, 0, 6);
    case 0xB6:
      return Action(RESET, HL, 0, 6);
    case 0xB7:
      return Action(RESET, A, 0, 6);
    case 0xB8:
      return Action(RESET, B, 0, 7);
    case 0xB9:
      return Action(RESET, C, 0, 7);
    case 0xBA:
      return Action(RESET, D, 0, 7);
    case 0xBB:
      return Action(RESET, E, 0, 7);
    case 0xBC:
      return Action(RESET, H, 0, 7);
    case 0xBD:
      return Action(RESET, L, 0, 7);
    case 0xBE:
      return Action(RESET, HL, 0, 7);
    case 0xBF:
      return Action(RESET, A, 0, 7);

    case 0xC0:
      return Action(SET, B);
    case 0xC1:
      return Action(SET, C);
    case 0xC2:
      return Action(SET, D);
    case 0xC3:
      return Action(SET, E);
    case 0xC4:
      return Action(SET, H);
    case 0xC5:
      return Action(SET, L);
    case 0xC6:
      return Action(SET, HL);
    case 0xC7:
      return Action(SET, A);
    case 0xC8:
      return Action(SET, B, 0, 1);
    case 0xC9:
      return Action(SET, C, 0, 1);
    case 0xCA:
      return Action(SET, D, 0, 1);
    case 0xCB:
      return Action(SET, E, 0, 1);
    case 0xCC:
      return Action(SET, H, 0, 1);
    case 0xCD:
      return Action(SET, L, 0, 1);
    case 0xCE:
      return Action(SET, HL, 0, 1);
    case 0xCF:
      return Action(SET, A, 0, 1);

    case 0xD0:
      return Action(SET, B, 0 ,2);
    case 0xD1:
      return Action(SET, C, 0, 2);
    case 0xD2:
      return Action(SET, D, 0, 2);
    case 0xD3:
      return Action(SET, E, 0, 2);
    case 0xD4:
      return Action(SET, H, 0, 2);
    case 0xD5:
      return Action(SET, L, 0, 2);
    case 0xD6:
      return Action(SET, HL, 0, 2);
    case 0xD7:
      return Action(SET, A, 0, 2);
    case 0xD8:
      return Action(SET, B, 0, 3);
    case 0xD9:
      return Action(SET, C, 0, 3);
    case 0xDA:
      return Action(SET, D, 0, 3);
    case 0xDB:
      return Action(SET, E, 0, 3);
    case 0xDC:
      return Action(SET, H, 0, 3);
    case 0xDD:
      return Action(SET, L, 0, 3);
    case 0xDE:
      return Action(SET, HL, 0, 3);
    case 0xDF:
      return Action(SET, A, 0, 3);

    case 0xE0:
      return Action(SET, B, 0 ,4);
    case 0xE1:
      return Action(SET, C, 0, 4);
    case 0xE2:
      return Action(SET, D, 0, 4);
    case 0xE3:
      return Action(SET, E, 0, 4);
    case 0xE4:
      return Action(SET, H, 0, 4);
    case 0xE5:
      return Action(SET, L, 0, 4);
    case 0xE6:
      return Action(SET, HL, 0, 4);
    case 0xE7:
      return Action(SET, A, 0, 4);
    case 0xE8:
      return Action(SET, B, 0, 5);
    case 0xE9:
      return Action(SET, C, 0, 5);
    case 0xEA:
      return Action(SET, D, 0, 5);
    case 0xEB:
      return Action(SET, E, 0, 5);
    case 0xEC:
      return Action(SET, H, 0, 5);
    case 0xED:
      return Action(SET, L, 0, 5);
    case 0xEE:
      return Action(SET, HL, 0, 5);
    case 0xEF:
      return Action(SET, A, 0, 5);

    case 0xF0:
      return Action(SET, B, 0 ,6);
    case 0xF1:
      return Action(SET, C, 0, 6);
    case 0xF2:
      return Action(SET, D, 0, 6);
    case 0xF3:
      return Action(SET, E, 0, 6);
    case 0xF4:
      return Action(SET, H, 0, 6);
    case 0xF5:
      return Action(SET, L, 0, 6);
    case 0xF6:
      return Action(SET, HL, 0, 6);
    case 0xF7:
      return Action(SET, A, 0, 6);
    case 0xF8:
      return Action(SET, B, 0, 7);
    case 0xF9:
      return Action(SET, C, 0, 7);
    case 0xFA:
      return Action(SET, D, 0, 7);
    case 0xFB:
      return Action(SET, E, 0, 7);
    case 0xFC:
      return Action(SET, H, 0, 7);
    case 0xFD:
      return Action(SET, L, 0, 7);
    case 0xFE:
      return Action(SET, HL, 0, 7);
    case 0xFF:
      return Action(SET, A, 0, 7);
  }
}
