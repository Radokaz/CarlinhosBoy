#include "memorybus.h"

namespace GB{

void DMA::start(uint8_t valor){
  ativo = true;
  byte = 0;
  this->valor = valor;
  atraso = 1;
}

void DMA::step(Memorybus *bus){
  if(ativo){

  if(atraso){
    --atraso;
    return;
  }
    
  uint16_t endereco = (this->valor*0x100) + this->byte;
  uint8_t result = bus->read_byte(endereco);
  bus->memoria[0xFE00 + this->byte] = result;

  ++this->byte;
  if(this->byte >= 0xA0)
    ativo = false;
  }
}

}
