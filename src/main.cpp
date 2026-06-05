#include "init.h"

void degub_func(GB::CPU *cpu, GB::PPU *ppu){
  std::cout << "PC: " << std::hex << cpu->pc << "\n";
  std::cout << "A: " << static_cast<int>(cpu->registradores.a) << "\n";
  std::cout << "B: " << static_cast<int>(cpu->registradores.b) << "\n";
  std::cout << "C: " << static_cast<int>(cpu->registradores.c) << "\n";
  std::cout << "D: " << static_cast<int>(cpu->registradores.d) << "\n";
  std::cout << "E: " << static_cast<int>(cpu->registradores.e) << "\n";
  std::cout << "H: " << static_cast<int>(cpu->registradores.h) << "\n";
  std::cout << "L: " << static_cast<int>(cpu->registradores.l) << "\n";
  std::cout << "F: " << static_cast<int>(cpu->registradores.f) << "\n";
  std::cout << "IF: " << static_cast<int>(cpu->get_if()) << "\n";
  std::cout << "Stepping: " << std::boolalpha << cpu->stepping << "\n";
  std::cout << "Halted: " << cpu->halted << "\n";
  std::cout << "IME: " << cpu->ime << "\n";
  std::cout << "Haltbug: " << cpu->haltbug << "\n";
  std::cout << "Ultima instrução: " << static_cast<int>(cpu->last_instruct) << "\n";
  std::cout << "Tac :" << static_cast<int>(cpu->bus.memoria[0xFF07]) << "\n";
  std::cout << "LY: " << std::dec << static_cast<int>(cpu->bus.memoria[0xFF44]) << "\n";
}

int main(int argc, char **argv){
  
  if(argc < 2){
    std::cout << "Especifique uma rom.\n";
    return 1;
  }

  float escala {10.0f};
  InitWindow(160*escala, 144*escala, "Carlinhos Boy");
  SetTargetFPS(60);

  Image framebuffer = GenImageColor(160, 144, RAYWHITE);
  ImageFormat(&framebuffer, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  Texture2D texture = LoadTextureFromImage(framebuffer);
  UnloadImage(framebuffer);
  SetTextureFilter(texture, TEXTURE_FILTER_POINT);

  InitAudioDevice();

  AudioStream stream = LoadAudioStream(
    44100, // sample rate
    16,    // bits por sample
    2      // canais (stereo)
  );
  
  SetAudioStreamCallback(stream, GB::audio_callback);
  PlayAudioStream(stream);

  GB::Joypad pad;
  GB::PPU ppu(&texture);
  GB::Timer timer;
  GB::CPU cpu(&timer, &pad, &ppu);
  GB::APU apu(cpu.bus.memoria.data());
  ppu.memoria = cpu.bus.memoria.data();
  pad.p1 = &cpu.bus.memoria[0xFF00];
  timer.apu = &apu;

  GB::init_game(&cpu, argv);

  constexpr double tempo_frame = 1.0/59.7;

  Vector2 mouse_prev = GetMousePosition();
  Vector2 mouse_atual{};
  double frame_init {}, frame_fim {};

  while(!WindowShouldClose()){
    frame_init = GetTime();

    mouse_atual = GetMousePosition();
    le_input(pad, ppu.paleta_lcd);
    
    if(mouse_atual.x != mouse_prev.x || mouse_atual.y != mouse_prev.y){
      ShowCursor();
    }
    mouse_prev = mouse_atual;

    ppu.frame_pronto = false;
    while(!ppu.frame_pronto){
      roda_cpu(&cpu);
      if(!cpu.stepping)
        break;
        //degub_func(&cpu, &ppu);
    }

    BeginDrawing();
    DrawTextureEx(texture, Vector2{0, 0}, 0, escala, WHITE);
    EndDrawing();

    frame_fim = GetTime() - frame_init;
    if(frame_fim < tempo_frame){
      WaitTime(tempo_frame - frame_fim);
    }
  }

  if(cpu.bus.tem_save){
    cpu.bus.mbc->save();
  }
  
  ShowCursor();
  UnloadAudioStream(stream);
  CloseAudioDevice();
  UnloadTexture(texture);
  CloseWindow();

  return 0;
}
