#include "memorybus.h"

namespace GB{

void le_input(Joypad& pad, bool& paleta_lcd, uint8_t& canais_ativos){
  uint8_t controles {};

  if(segurado(KEY_M)) 
    controles |= A_BUTTON;
  if(segurado(KEY_N)) 
    controles |= B_BUTTON;
  if(segurado(KEY_O))
    controles |= START;
  if(segurado(KEY_P))
    controles |= SELECT;
  if(segurado(KEY_W))
    controles |= UP_BUTTON;
  if(segurado(KEY_A))
    controles |= LEFT_BUTTON;
  if(segurado(KEY_S))
    controles |= DOWN_BUTTON;
  if(segurado(KEY_D))
    controles |= RIGHT_BUTTON;
  if(apertado(KEY_T))
    paleta_lcd ^= 1;
  if(apertado(KEY_ONE)){
    canais_ativos ^= APU_CANAL1;
    std::cout << "Canal 1 " << ((canais_ativos & APU_CANAL1) ? "ATIVADO\n" : "DESATIVADO\n");
  }
  if(apertado(KEY_TWO)){
    canais_ativos ^= APU_CANAL2;
    std::cout << "Canal 2 " << ((canais_ativos & APU_CANAL2) ? "ATIVADO\n" : "DESATIVADO\n");
  }
  if(apertado(KEY_THREE)){
    canais_ativos ^= APU_CANAL3;
    std::cout << "Canal 3 " << ((canais_ativos & APU_CANAL3) ? "ATIVADO\n" : "DESATIVADO\n");
  }
  if(apertado(KEY_FOUR)){
    canais_ativos ^= APU_CANAL4;
    std::cout << "Canal 4 " << ((canais_ativos & APU_CANAL4) ? "ATIVADO\n" : "DESATIVADO\n");
  }

  if(controles){
    HideCursor();
  }
  pad.input(controles);
  //std::cout << std::hex << static_cast<int32_t>(pad.controles) << "\n";
}

}
