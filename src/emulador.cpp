#include "init.h"

namespace GB{

void degub_func(CPU *cpu){
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

void inicia_emulador(std::string_view src, GB_State *estado){
  constexpr float escala {10.0f};
  ClearBackground(BLACK);

  Image framebuffer = GenImageColor(160, 144, RAYWHITE);
  ImageFormat(&framebuffer, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  Texture2D texture = LoadTextureFromImage(framebuffer);
  UnloadImage(framebuffer);
  SetTextureFilter(texture, TEXTURE_FILTER_POINT);

  constexpr float texture_w = 160*escala;
  constexpr float texture_h = 144*escala;

  float posX = (GetScreenWidth()  - texture_w)/2.0f;
  float posY = (GetScreenHeight() - texture_h)/2.0f;

  InitAudioDevice();

  AudioStream stream = LoadAudioStream(
    44100, // sample rate
    16,    // bits por sample
    2      // canais (stereo)
  );
  
  SetAudioStreamCallback(stream, GB::audio_callback);
  PlayAudioStream(stream);

  Joypad pad(reinterpret_cast<const void*>(estado->controles.data()));
  PPU ppu(&texture);
  Timer timer;
  CPU cpu(&timer, &pad, &ppu);
  APU apu(cpu.bus.memoria.data());
  ppu.memoria = cpu.bus.memoria.data();
  pad.p1 = &cpu.bus.memoria[0xFF00];
  timer.apu = &apu;

  if(!init_game(&cpu, src.data(), estado->saves_path.data())){
    ShowCursor();
    UnloadAudioStream(stream);
    CloseAudioDevice();
    UnloadTexture(texture);
    
    return;
  }

  constexpr double tempo_frame60 = 1.0/59.7;
  constexpr double tempo_frame120 = 1.0/119.4;

  Vector2 mouse_prev = GetMousePosition();
  Vector2 mouse_atual{};
  double frame_init {}, frame_fim {};
  bool pausado {false};
  bool is_120 = false;
  SetTargetFPS(60);

  while(1){
    if(apertado(KEY_ESCAPE)) break;
    frame_init = GetTime();

    mouse_atual = GetMousePosition();
    le_input(pad, ppu.paleta_lcd, apu.canais_ativos, pausado, is_120);
    
    if(mouse_atual.x != mouse_prev.x || mouse_atual.y != mouse_prev.y){
      ShowCursor();
    }
    mouse_prev = mouse_atual;
    if(pausa_jogo(&cpu, estado, pausado)){
      break;
    }

    ppu.frame_pronto = false;
    while(!ppu.frame_pronto){
      roda_cpu(&cpu);
      if(!cpu.stepping)
        break;
        //degub_func(&cpu);
    }

    BeginDrawing();
    DrawRectangle(1750, 400, 500, 900, BLACK);
    DrawTextureEx(texture, Vector2{posX, posY}, 0, escala, WHITE);
    EndDrawing();

    frame_fim = GetTime() - frame_init;
    double tempo_atual = (is_120) ? tempo_frame120 : tempo_frame60;

    if(frame_fim < tempo_atual){
      WaitTime(tempo_atual - frame_fim);
    }
  }

  if(cpu.bus.mbc && cpu.bus.mbc->tem_save){
    cpu.bus.mbc->save();
  }
  
  ShowCursor();
  UnloadAudioStream(stream);
  CloseAudioDevice();
  UnloadTexture(texture);
}


}
