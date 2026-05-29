#include "memorybus.h"

namespace GB{

void PPU::oam_read_corruption(uint8_t scan_row){
  uint16_t a = (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + scan_row*8]) | 
    (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + scan_row*8 + 1]) << 8));
  uint16_t b = (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + (scan_row - 1)*8]) | 
    (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + (scan_row - 1)*8 + 1]) << 8));
  uint16_t c = (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + (scan_row - 1)*8 + 4])  | 
    (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + (scan_row - 1)*8 + 5]) << 8));
  
  uint16_t primeiro = (b | (a & c));
  this->bus->memoria[OAM_INICIO + scan_row*8] = (primeiro & 0xFF);
  this->bus->memoria[OAM_INICIO + scan_row*8 + 1] = ((primeiro >> 8) & 0xFF);
  for(size_t i {2}; i < 8; ++i){
    this->bus->memoria[OAM_INICIO + scan_row*8 + i] = this->bus->memoria[OAM_INICIO + (scan_row - 1)*8 + i];
  }
}

void PPU::oam_write_corruption(uint8_t scan_row){
  uint16_t a = (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + scan_row*8]) | 
    (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + scan_row*8 + 1]) << 8));
  uint16_t b = (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + (scan_row - 1)*8]) | 
    (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + (scan_row - 1)*8 + 1]) << 8));
  uint16_t c = (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + (scan_row - 1)*8 + 4])  | 
    (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + (scan_row - 1)*8 + 5]) << 8));

  uint16_t primeiro = (((a ^ c) & (b ^ c)) ^ c);
  this->bus->memoria[OAM_INICIO + scan_row*8] = (primeiro & 0xFF);
  this->bus->memoria[OAM_INICIO + scan_row*8 + 1] = ((primeiro >> 8) & 0xFF);
  for(size_t i {2}; i < 8; ++i){
    this->bus->memoria[OAM_INICIO + scan_row*8 + i] = this->bus->memoria[OAM_INICIO + (scan_row - 1)*8 + i];
  }
}

void PPU::oam_readwrite_corruption(uint8_t scan_row){
  uint16_t a = (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + (scan_row - 2)*8]) | 
    (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + (scan_row - 2)*8 + 1]) << 8));
  uint16_t b = (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + (scan_row - 1)*8]) | 
    (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + (scan_row - 1)*8 + 1]) << 8));
  uint16_t c = (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + scan_row*8])  | 
    (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + scan_row*8 + 1]) << 8));
  uint16_t d = (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + (scan_row - 1)*8 + 4])  | 
    (static_cast<uint16_t>(this->bus->memoria[OAM_INICIO + (scan_row - 1)*8 + 5]) << 8));
 
  uint16_t primeiro = ((b & (a | c | d)) | (a & c & d));
  this->bus->memoria[OAM_INICIO + (scan_row - 1)*8] = (primeiro & 0xFF);
  this->bus->memoria[OAM_INICIO + (scan_row - 1)*8 + 1] = ((primeiro >> 8) & 0xFF);

  for(size_t i {}; i < 3; i+=2){
    for(size_t j {}; j < 8; ++j){
      this->bus->memoria[OAM_INICIO + (scan_row - i)*8 + j] = this->bus->memoria[OAM_INICIO + (scan_row - 1)*8 + j];
    }
  }
}

void PPU::check_oam(uint16_t registrador, oam_corruption corruption){
  if(this->modo_atual != screen_mode::SOAMRAM) return;
  if(registrador < 0xFE00 || registrador > 0xFEFF) return;

  uint8_t scan_row = this->sprites_lidos/2;
  if(!scan_row) return;

  switch(corruption){
    using enum oam_corruption;

    case WRITE:{
      this->oam_write_corruption(scan_row);
      break;
    }
    case READ:{
      this->oam_read_corruption(scan_row);
      break;
    }
    case READ_WRITE:{
      if(scan_row >= 4 && scan_row < 19)
        this->oam_readwrite_corruption(scan_row);

      this->oam_read_corruption(scan_row);
      break;
    }
  }
}

}
