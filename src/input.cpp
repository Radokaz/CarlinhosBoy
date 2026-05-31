#include "memorybus.h"

namespace GB{

void le_input(Joypad& pad, bool& paleta_lcd){
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

  if(controles){
    HideCursor();
  }
  pad.input(controles);
  //std::cout << std::hex << static_cast<int32_t>(pad.controles) << "\n";
}

}
