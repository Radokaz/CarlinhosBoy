#include "init.h"
#include <raylib.h>

#define MAX_TICKS 70224
#define apt(x) IsKeyDown(x)

void le_input(GB::Joypad& pad){
  uint8_t controles {};

  if(apt(KEY_Z)) 
    controles |= A_BUTTON;
  if(apt(KEY_X)) 
    controles |= B_BUTTON;
  if(apt(KEY_ENTER))
    controles |= START;
  if(apt(KEY_BACKSPACE))
    controles |= SELECT;
  if(apt(KEY_W))
    controles |= UP_BUTTON;
  if(apt(KEY_A))
    controles |= LEFT_BUTTON;
  if(apt(KEY_S))
    controles |= DOWN_BUTTON;
  if(apt(KEY_D))
    controles |= RIGHT_BUTTON;

  pad.input(controles);
  //std::cout << std::hex << static_cast<int32_t>(pad.controles) << "\n";
}

int main(int argc, char **argv){
  
  if(argc < 2){
    std::cout << "Especifique uma rom.\n";
    return 1;
  }

  int escala {8};
  InitWindow(160*escala, 144*escala, "Game Boy");
  SetTargetFPS(60);

  Image framebuffer = GenImageColor(160, 144, BLACK);
  Texture2D texture = LoadTextureFromImage(framebuffer);
  UnloadImage(framebuffer);

  GB::Timer timer;
  GB::Joypad pad;
  GB::CPU cpu(&timer.div_count, &pad);
  pad.p1 = &cpu.bus.memoria[0xFF00];

  GB::init_game(&cpu, timer, argv);

  while(!WindowShouldClose()){
    le_input(pad);
    
    for(size_t i {}; i < MAX_TICKS; i+=cpu.last_ticks*4){
      roda_cpu(&cpu, timer);
         
    }

    BeginDrawing();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
