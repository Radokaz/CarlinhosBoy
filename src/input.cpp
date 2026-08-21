#include "memorybus.h"
#include "game_state.h"

namespace GB{

void le_input(Game_State *estado, size_t save_slot, bool& pausado, bool& is_120, bool& janela_alterada){
  uint8_t controles {};

  const KeyboardKey *map = reinterpret_cast<const KeyboardKey*>(estado->pad.teclas);

  int gamepad = GamepadDisponivel();
  if(gamepad > -1){
    le_input_controle(estado, save_slot, pausado, is_120, janela_alterada, controles, gamepad);
  }

#ifndef UWP_BUILDING
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
  if(segurado(KEY_LEFT_ALT)){
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
  }
#endif

  if(controles){
    HideCursor();
  }
#endif

  estado->pad.input(controles);
  //std::cout << std::hex << static_cast<int32_t>(pad.controles) << "\n";
}

void le_input_controle(Game_State *estado, size_t save_slot, bool& pausado, bool& is_120, bool& janela_alterada, uint8_t& controles, int gamepad){
  const GamepadComb *but = reinterpret_cast<const GamepadComb *>(estado->pad.botoes);

  float leftStickX = fix_deadzone(GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X));
  float leftStickY = fix_deadzone(GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y));

  if(but[0].segurado(gamepad)) 
    controles |= A_BUTTON;
  if(but[1].segurado(gamepad) || IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) //botão B já pré-mapeado
    controles |= B_BUTTON;
  if(but[2].segurado(gamepad))
    controles |= START;
  if(but[3].segurado(gamepad))
    controles |= SELECT;
  if(but[4].segurado(gamepad) || leftStickY < -0.5f)
    controles |= UP_BUTTON;
  if(but[5].segurado(gamepad) || leftStickX < -0.5f)
    controles |= LEFT_BUTTON;
  if(but[6].segurado(gamepad) || leftStickY > 0.5f)
    controles |= DOWN_BUTTON;
  if(but[7].segurado(gamepad) || leftStickX > 0.5f)
    controles |= RIGHT_BUTTON;
  if(but[8].pressionado(gamepad))
    estado->ppu.paleta_lcd ^= 1;
  if(but[9].segurado(gamepad))
    pausado = true;
  if(but[10].pressionado(gamepad)){
    is_120 ^= 1;
  }
#ifndef UWP_BUILDING
  if(but[11].pressionado(gamepad)){
    ToggleFullscreen();
    janela_alterada = true;
  }
#endif
  if(but[12].pressionado(gamepad)){
    estado->save_state(save_slot);
  }
  if(but[13].pressionado(gamepad)){
    estado->load_state(save_slot);
  }
}

}
