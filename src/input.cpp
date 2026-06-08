#include "memorybus.h"

namespace GB{

void le_input(Joypad& pad, bool& paleta_lcd, uint8_t& canais_ativos, bool& pausado){
  uint8_t controles {};

  const KeyboardKey *map = reinterpret_cast<const KeyboardKey*>(pad.teclas);

  if(segurado(map[0])) 
    controles |= A_BUTTON;
  if(segurado(map[1])) 
    controles |= B_BUTTON;
  if(segurado(map[2]))
    controles |= START;
  if(segurado(map[3]))
    controles |= SELECT;
  if(segurado(map[4]))
    controles |= UP_BUTTON;
  if(segurado(map[5]))
    controles |= LEFT_BUTTON;
  if(segurado(map[6]))
    controles |= DOWN_BUTTON;
  if(segurado(map[7]))
    controles |= RIGHT_BUTTON;
  if(apertado(map[8]))
    paleta_lcd ^= 1;
  if(apertado(map[9]))
    pausado = true;

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
