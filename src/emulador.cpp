#include "init.h"
#include "game_state.h"

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
  std::cout << "Ultima instrução: " << std::hex << static_cast<int>(cpu->last_instruct) << "\n";
  std::cout << "Tac :" << static_cast<int>(cpu->bus.memoria[0xFF07]) << "\n";
  std::cout << "LY: " << std::dec << static_cast<int>(cpu->bus.memoria[0xFF44]) << "\n";
}

#ifndef UWP_BUILDING
void inicia_emulador(std::string_view src, GB_State *estado){

  Image framebuffer = GenImageColor(160, 144, RAYWHITE);
  ImageFormat(&framebuffer, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  Texture2D texture = LoadTextureFromImage(framebuffer);
  UnloadImage(framebuffer);
  SetTextureFilter(texture, TEXTURE_FILTER_POINT);
  
  float escala = (std::min)(get_width()/1920.0f, get_height()/1080.0f)*7.0f;
  float texture_w = 160*escala;
  float texture_h = 144*escala;
  float posX = (get_width() - texture_w)/2.0f;
  float posY = (get_height() - texture_h)/2.0f;

  InitAudioDevice();

  AudioStream stream = LoadAudioStream(
    44100, // sample rate
    16,    // bits por sample
    2      // canais (stereo)
  );
  
  SetAudioStreamCallback(stream, GB::audio_callback);
  PlayAudioStream(stream);

  Game_State game(&texture, estado, estado->states_path.data(), src.data());

  if(!init_game(&game.cpu, src.data(), estado->saves_path.data(), estado->paleta_cgb)){
    ShowCursor();
    UnloadAudioStream(stream);
    CloseAudioDevice();
    UnloadTexture(texture);
    
    return;
  }

  constexpr double tempo_frame60 = 1.0/60.0;
  constexpr double tempo_frame120 = 1.0/120.0;

  Vector2 mouse_prev = GetMousePosition();
  Vector2 mouse_atual{};
  double frame_init {}, frame_fim {};
  bool pausado {false};
  bool resumido {false};
  bool janela_alterada {false};
  bool is_120 {false};
  SetTargetFPS(0);

  while(1){
    ClearBackground(BLACK);
    
    frame_init = GetTime();
    mouse_atual = GetMousePosition();
    if(mouse_atual.x != mouse_prev.x || mouse_atual.y != mouse_prev.y){
      ShowCursor();
    }
    mouse_prev = mouse_atual;

    le_input(&game, estado->save_slot, pausado, is_120, janela_alterada);
    if(pausa_jogo(&game, estado, pausado, resumido)){
      break;
    }

    if(IsWindowResized() || resumido || janela_alterada){
      janela_alterada = false;
      resumido = false;
      escala = (std::min)(get_width()/1920.0f, get_height()/1080.0f)*7.0f;
      texture_w = 160*escala;
      texture_h = 144*escala;
      posX = (get_width() - texture_w)/2.0f;
      posY = (get_height() - texture_h)/2.0f;
    }

    if(game.cpu.bus.tem_rtc){
      game.cpu.bus.mbc->atualiza_rtc();
    }

    game.ppu.frame_pronto = false;
    while(!game.ppu.frame_pronto){
      roda_cpu(&game.cpu);
      if(!game.cpu.stepping)
        break;
        //degub_func(&game.cpu);
    }

    BeginDrawing();
    DrawTextureEx(texture, Vector2{posX, posY}, 0, escala, WHITE);
    EndDrawing();

    if(game.cpu.bus.mbc && game.cpu.bus.mbc->jogo_salvo){
      checa_save(game.cpu.bus.mbc.get());
    }

    frame_fim = GetTime() - frame_init;
    double tempo_atual = (is_120) ? tempo_frame120 : tempo_frame60;

    if(frame_fim < tempo_atual){
      WaitTime(tempo_atual - frame_fim);
    }
  }

  if(game.cpu.bus.mbc && game.cpu.bus.mbc->tem_save){
    game.cpu.bus.mbc->save();
  }
  
  SetTargetFPS(60);
  ShowCursor();
  UnloadAudioStream(stream);
  CloseAudioDevice();
  UnloadTexture(texture);
}
#else
void inicia_emulador(std::string_view src, GB_State* estado) {

}
#endif

}
