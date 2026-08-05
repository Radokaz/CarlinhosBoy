#include "memorybus.h"
#include "game_state.h"

namespace GB{

void le_input(Game_State *estado, size_t save_slot, bool& pausado, bool& is_120, bool& janela_alterada){
  uint8_t controles {};

  const KeyboardKey *map = reinterpret_cast<const KeyboardKey*>(estado->pad.teclas);

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
    estado->ppu.paleta_lcd ^= 1;
  if(apertado(map[9]) || apertado(KEY_ESCAPE))
    pausado = true;
  if(apertado(map[10])){
    is_120 ^= 1;
    SetTargetFPS(static_cast<int>(is_120)*60 + 60);
  }
  if(apertado(map[11])){
    ToggleFullscreen();
    janela_alterada = true;
  }
  if(apertado(map[12])){
    estado->save_state(save_slot);
  }
  if(apertado(map[13])){
    estado->load_state(save_slot);
  }
  
#define AUDIO_CHANNEL_DEBUG
#ifdef AUDIO_CHANNEL_DEBUG
  if(apertado(KEY_ONE)){
    APU::canais_ativos ^= APU_CANAL1;
    std::cout << "Canal 1 " << ((APU::canais_ativos & APU_CANAL1) ? "ATIVADO\n" : "DESATIVADO\n");
  }
  if(apertado(KEY_TWO)){
    APU::canais_ativos ^= APU_CANAL2;
    std::cout << "Canal 2 " << ((APU::canais_ativos & APU_CANAL2) ? "ATIVADO\n" : "DESATIVADO\n");
  }
  if(apertado(KEY_THREE)){
    APU::canais_ativos ^= APU_CANAL3;
    std::cout << "Canal 3 " << ((APU::canais_ativos & APU_CANAL3) ? "ATIVADO\n" : "DESATIVADO\n");
  }
  if(apertado(KEY_FOUR)){
    APU::canais_ativos ^= APU_CANAL4;
    std::cout << "Canal 4 " << ((APU::canais_ativos & APU_CANAL4) ? "ATIVADO\n" : "DESATIVADO\n");
  }
#endif

  if(controles){
    HideCursor();
  }
  estado->pad.input(controles);
  //std::cout << std::hex << static_cast<int32_t>(pad.controles) << "\n";
}

}
