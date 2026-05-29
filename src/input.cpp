#include "memorybus.h"

namespace GB{

void le_input(Joypad& pad, bool& paleta_lcd){
  uint8_t controles {};

  if(apt(KEY_M)) 
    controles |= A_BUTTON;
  if(apt(KEY_N)) 
    controles |= B_BUTTON;
  if(apt(KEY_O))
    controles |= START;
  if(apt(KEY_P))
    controles |= SELECT;
  if(apt(KEY_W))
    controles |= UP_BUTTON;
  if(apt(KEY_A))
    controles |= LEFT_BUTTON;
  if(apt(KEY_S))
    controles |= DOWN_BUTTON;
  if(apt(KEY_D))
    controles |= RIGHT_BUTTON;
  if(IsKeyPressed(KEY_T))
    paleta_lcd ^= 1;

  if(controles){
    HideCursor();
  }
  pad.input(controles);
  //std::cout << std::hex << static_cast<int32_t>(pad.controles) << "\n";
}

}
