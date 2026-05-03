#include "init.h"

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

void degub_func(GB::CPU *cpu, GB::PPU *ppu){
  std::cout << "PC: " << std::hex << cpu->pc << "\n";
  std::cout << "F: " << static_cast<int>(cpu->registradores.f) << "\n";
  std::cout << "IF: " << static_cast<int>(cpu->get_if()) << "\n";
  std::cout << "Stepping: " << std::boolalpha << cpu->stepping << "\n";
  std::cout << "Halted: " << cpu->halted << "\n";
  std::cout << "IME: " << cpu->ime << "\n";
  std::cout << "Haltbug: " << cpu->haltbug << "\n";
  std::cout << "Ultima instrução: " << static_cast<int>(cpu->last_instruct) << "\n";
  std::cout << "Tac :" << static_cast<int>(cpu->bus.memoria[0xFF07]) << "\n";
  /*std::cout << "\n";
  std::cout << "\n";
  std::cout << "\n";
  std::cout << "\n";
  std::cout << "\n";
  std::cout << "\n";*/
}

int main(int argc, char **argv){
  
  if(argc < 2){
    std::cout << "Especifique uma rom.\n";
    return 1;
  }

  float escala {8.0f};
  InitWindow(160*escala, 144*escala, "Game Boy");
  SetTargetFPS(60);

  Image framebuffer = GenImageColor(160, 144, RAYWHITE);
  ImageFormat(&framebuffer, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  Texture2D texture = LoadTextureFromImage(framebuffer);
  UnloadImage(framebuffer);
  SetTextureFilter(texture, TEXTURE_FILTER_POINT);

  GB::Timer timer;
  GB::Joypad pad;
  GB::PPU ppu(&texture);
  GB::CPU cpu(&timer, &pad, &ppu);
  ppu.bus = &cpu.bus;
  pad.p1 = &cpu.bus.memoria[0xFF00];

  GB::init_game(&cpu, timer, argv);

  while(!WindowShouldClose()){
    le_input(pad);
    
    for(size_t i {}; i < MAX_TICKS; i+=cpu.last_ticks){
      roda_cpu(&cpu, &timer, &ppu);
      //degub_func(&cpu, &ppu);
    }

    BeginDrawing();
    DrawTextureEx(texture, Vector2{0, 0}, 0, escala, WHITE);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
