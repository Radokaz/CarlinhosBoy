#ifndef DMA_H
#define DMA_H

struct DMA{
  bool ativo {false};
  uint8_t byte {};
  uint8_t valor {};
  uint8_t atraso {};

  void start(uint8_t valor){
    ativo = true;
    byte = 0;
    this->valor = valor;
    atraso = 2;
  }

  void step(uint8_t *memoria){
    if(ativo){

      if(atraso){
        --atraso;
        return;
      }
    

    memoria[0xFE00 + this->byte] = memoria[(this->valor * 0x100) + this->byte];
    ++this->byte;

    if(this->byte >= 0xA0)
      ativo = false;
    }
  }
};

#endif
