#include "interface.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

namespace GB{

const char *getDisplayName(KeyboardKey key){
  const char *tecla = GetKeyName(key);
  if(tecla)
    return tecla;

  switch(key){
    case KEY_UP: return "Up";
    case KEY_DOWN: return "Down";
    case KEY_LEFT:  return "Left";
    case KEY_RIGHT: return "Right";
    case KEY_ENTER: return "Enter";
    case KEY_BACKSPACE: return "Backsp.";
    case KEY_ESCAPE: return "Esc";
    case KEY_SPACE: return "Space";
    case KEY_LEFT_SHIFT: return "LSHIFT";
    case KEY_RIGHT_SHIFT: return "RSHIFT";
    case KEY_TAB: return "Tab";
    case KEY_F1: return "F1";
    case KEY_F2: return "F2";
    case KEY_F3: return "F3";
    case KEY_F4: return "F4";
    case KEY_F5: return "F5";
    case KEY_F6: return "F6";
    case KEY_F7: return "F7";
    case KEY_F8: return "F8";
    case KEY_F9: return "F9";
    case KEY_F10: return "F10";
    case KEY_F11: return "F11";
    case KEY_F12: return "F12";
    case KEY_LEFT_CONTROL: return "LCTRL";
    case KEY_RIGHT_CONTROL: return "RCTRL";
    case KEY_LEFT_ALT: return "ALT";
    case KEY_RIGHT_ALT: return "ALTGR";
    default: return "Undef.";
  }
}

Rectangle get_ret(float x, float y, float w, float h){
  constexpr float width = 1920.0f;
  constexpr float height = 1080.0f;
  float screen_w = GetScreenWidth()/width;
  float screen_h = GetScreenHeight()/height;
  float scale = std::min(screen_w, screen_h);
  return Rectangle{scale*x, scale*y, scale*w, scale*h};
}

void display_controles(GB_State *estado){
  const char *botoes[std::size(gb_botoes)];
  std::string botoes_show[std::size(gb_botoes)];

  for(size_t i {}; i < std::size(gb_botoes); ++i){
    botoes[i] = getDisplayName(estado->controles[i]);
    botoes_show[i] = (std::string(gb_botoes[i]) + " : ");
  }

  constexpr float width = 1920.0f;
  constexpr float height = 1080.0f;
  float screen_w = GetScreenWidth()/width;
  float screen_h = GetScreenHeight()/height;
  float scale = std::min(screen_w, screen_h);
  GuiSetStyle(DEFAULT, TEXT_SIZE, scale*25.0f);
  GuiSetStyle(BUTTON, TEXT_SIZE, scale*25.0f);

  while(1){
    BeginDrawing();
    ClearBackground(BLACK);
    bool tecla_apertada {false};
    
    if(IsWindowResized()){
      screen_w = GetScreenWidth()/width;
      screen_h = GetScreenHeight()/height;
      scale = std::min(screen_w, screen_h);
      GuiSetStyle(DEFAULT, TEXT_SIZE, scale*25.0f);
      GuiSetStyle(BUTTON, TEXT_SIZE, scale*25.0f);
    }
    
    DrawText("CONTROLES", scale*500.0f, scale*80.0f, scale*150.0f, GOLD);
    DrawLine(scale*275.0f, scale*250.0f, scale*1625.0f, scale*250.0f, GOLD);

    for(size_t i {}; i < 8; ++i){
      Rectangle r = get_ret(400.0f, 290.0f + 75.0f*i, 100.0f, 50.0f);

      if(i == 2){
        DrawText(botoes_show[i].c_str(), scale*297.0f, scale*(305.0f + 75.0f*i), scale*22, GOLD);
      }
      else if(i == 3){
        DrawText(botoes_show[i].c_str(), scale*286.0f, scale*(305.0f + 75.0f*i), scale*22, GOLD);
      }
      else if(i == 4){
        DrawText(botoes_show[i].c_str(), scale*344.0f, scale*(305.0f + 75.0f*i), scale*22, GOLD);
      }
      else if(i == 5 || i == 6){
        DrawText(botoes_show[i].c_str(), scale*313.0f, scale*(305.0f + 75.0f*i), scale*22, GOLD);
      }
      else if(i == 7){
        DrawText(botoes_show[i].c_str(), scale*303.0f, scale*(305.0f + 75.0f*i), scale*22, GOLD);
      }
      else{
        DrawText(botoes_show[i].c_str(), scale*361.0f, scale*(305.0f + 75.0f*i), scale*22, GOLD);
      }

      if(GuiButton(r, botoes[i])){
        int tecla = 0;
        while(tecla == 0){
          if(IsWindowResized()){
            screen_w = GetScreenWidth()/width;
            screen_h = GetScreenHeight()/height;
            scale = std::min(screen_w, screen_h);
            GuiSetStyle(DEFAULT, TEXT_SIZE, scale*25.0f);
            GuiSetStyle(BUTTON, TEXT_SIZE, scale*25.0f);
          }

          BeginDrawing();
          ClearBackground(BLACK);
          DrawText("Aperte alguma tecla...", scale*800.0f, screen_h*400.0f, scale*30, GOLD);
          EndDrawing();
          tecla = GetKeyPressed();
        }
        if(tecla != 0){
          estado->controles[i] = static_cast<KeyboardKey>(tecla);
          botoes[i] = getDisplayName(estado->controles[i]);
          tecla_apertada = true;
        }
      }
    }

    for(size_t i {8}; i < 12; ++i){
      Rectangle r = get_ret(860.0f, 290.0f + 75.0f*(i - 8), 100.0f, 50.0f);
      if(i == 8 || i == 11){
        DrawText(botoes_show[i].c_str(), scale*680.0f, scale*(305.0f + 75.0f*(i - 8)), scale*22, GOLD);
      }
      else if(i == 9){
        DrawText(botoes_show[i].c_str(), scale*763.0f, scale*(305.0f + 75.0f*(i - 8)), scale*22, GOLD);
      }
      else{
        DrawText(botoes_show[i].c_str(), scale*747.0f, scale*(305.0f + 75.0f*(i - 8)), scale*22, GOLD);
      }
      
      if(GuiButton(r, botoes[i])){
        int tecla = 0;
        while(tecla == 0){
          if(IsWindowResized()){
            screen_w = GetScreenWidth()/width;
            screen_h = GetScreenHeight()/height;
            scale = std::min(screen_w, screen_h);
            GuiSetStyle(DEFAULT, TEXT_SIZE, scale*25.0f);
            GuiSetStyle(BUTTON, TEXT_SIZE, scale*25.0f);
          }

          BeginDrawing();
          ClearBackground(BLACK);
          DrawText("Aperte alguma tecla...", scale*800.0f, screen_h*400.0f, scale*30, GOLD);
          EndDrawing();
          tecla = GetKeyPressed();
        }
        if(tecla != 0){
          estado->controles[i] = static_cast<KeyboardKey>(tecla);
          botoes[i] = getDisplayName(estado->controles[i]);
          tecla_apertada = true;
        }
      }
    }

    DrawText("Aperte ESC para voltar", scale*300.0f, scale*950.0f, scale*22, GOLD);

    if(apertado(estado->controles[11]) && !tecla_apertada)
      ToggleFullscreen();

    if(apertado(KEY_ESCAPE) && !tecla_apertada){
      estado->atualiza_controles();
      break;
    }

    EndDrawing();
  }
}

bool pausa_jogo(CPU *cpu, GB_State *estado, bool& pausado, bool& resumido){
  if(!pausado) return false; 

  BeginDrawing();
  ShowCursor();
  EndDrawing();
  
  const char opcoes[3][10] = {"Resumir", "Controles", "Sair"};
  uint8_t escolhas {};

  constexpr float width = 1920.0f;
  constexpr float height = 1080.0f;

  float screen_w = GetScreenWidth()/width;
  float screen_h = GetScreenHeight()/height;
  float scale = std::min(screen_w, screen_h);
  GuiSetStyle(BUTTON, TEXT_SIZE, (scale*25.0f));
  GuiSetStyle(DEFAULT, TEXT_SIZE, (scale*25.0f));

  while(1){
    BeginDrawing();
    ClearBackground(BLACK);

    if(IsWindowResized()){
      screen_w = GetScreenWidth()/width;
      screen_h = GetScreenHeight()/height;
      scale = std::min(screen_w, screen_h);
      GuiSetStyle(BUTTON, TEXT_SIZE, (scale*25.0f));
      GuiSetStyle(DEFAULT, TEXT_SIZE, (scale*25.0f));
    }

    if(apertado(estado->controles[11]))
      ToggleFullscreen();

    if(apertado(KEY_ESCAPE) || apertado(estado->controles[9]))
      break;

    DrawText("PAUSE", scale*690.0f, scale*80.0f, scale*150.0f, GOLD);
    DrawLine(scale*275.0f, scale*250.0f, scale*1625.0f, scale*250.0f, GOLD);

    for(size_t i {}; i < 3; ++i){
      Rectangle r = get_ret(790.0f, (320.0f + 135.0f*i), 300.0f, 100.0f);
      if(GuiButton(r, opcoes[i]))
        escolhas |= opt_escolha(i);
    }

    if(escolhas & opt_escolha(0)){
      escolhas &= ~opt_escolha(0);
      resumido = true;
      break;
    }
    if(escolhas & opt_escolha(1)){
      escolhas &= ~opt_escolha(1);
      display_controles(estado);

      screen_w = GetScreenWidth()/width;
      screen_h = GetScreenHeight()/height;
      scale = std::min(screen_w, screen_h);
      GuiSetStyle(BUTTON, TEXT_SIZE, (scale*25.0f));
      GuiSetStyle(DEFAULT, TEXT_SIZE, (scale*25.0f));
    }
    if(escolhas & opt_escolha(2)){
      escolhas &= ~opt_escolha(2);
      return true;
    }

    EndDrawing();
  }

  pausado = false;
  return false;
}

void carrega_rom(GB_State *estado){
    const char *extensoes[] = {"*.gb", "*.gbc"};

    const char *resultado = tinyfd_openFileDialog(
        "Escolha a rom",  // título
        "",               // pasta inicial
        2,                // número de filtros
        extensoes,          // extensões permitidas
        "", // descrição
        0                 // 0 = um arquivo, 1 = múltiplos
    );

    if(!resultado) return;

    inicia_emulador(resultado, estado);
}

void define_pasta(GB_State *estado, std::string_view pasta, ListaArquivos *lista){
  const char *resultado = tinyfd_selectFolderDialog("Selecione uma pasta", "");
  if(!resultado) return;
  
  std::filesystem::path state_path = estado->main_dir / "state.cfg";

  std::fstream arquivo(state_path.string().c_str(), arquivo.in | arquivo.out);
  std::string result = resultado, buffer{};
  std::vector<std::string> linhas;

  while(std::getline(arquivo, buffer)){
    linhas.push_back(buffer);
  }
  arquivo.close();
  std::ofstream novo(state_path.string().c_str());

  for(auto& linha : linhas){
    size_t pos = linha.find(':');
    if(pos == std::string::npos){
      novo << linha << "\n";
      continue;
    }

    if(linha.substr(0, pos) == pasta){
      linha.replace(pos + 2, linha.size(), resultado);
      if(pasta == "rom_path"){
        estado->rom_path = std::string(resultado);
        lista->atualiza_lista(estado);
      }
      else if(pasta == "saves_path")
        estado->saves_path = std::string(resultado);
    }
          
    novo << linha << "\n";
  }
}

void toggle_paleta(GB_State *estado){
  estado->paleta_cgb ^= 1;
  std::filesystem::path state_path = estado->main_dir / "state.cfg";
  std::fstream arquivo(state_path.string().c_str(), arquivo.in | arquivo.out);
  std::string buffer{};
  std::vector<std::string> linhas;

  while(std::getline(arquivo, buffer)){
    linhas.push_back(buffer);
  }
  arquivo.close();
  std::ofstream novo(state_path.string().c_str());

  for(auto& linha : linhas){
    size_t pos = linha.find(':');
    if(pos == std::string::npos){
      novo << linha << "\n";
      continue;
    }

    if(linha.substr(0, pos) == "paleta_cgb"){
      linha.replace(pos + 2, linha.size(), std::to_string(estado->paleta_cgb));
    }
          
    novo << linha << "\n";
  }

}

void init_gui(void){

  constexpr float width = 1920.0f;
  constexpr float height = 1080.0f;

  InitWindow(width, height, "Carlinhos Boy");
  SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
  SetTargetFPS(60);
  
  std::string opcoes[] = {
    "Abrir ROM", "Controles", "Definir pasta de saves", "Definir pasta de roms", "Sair",
  };

  uint8_t escolhas {};

  GB_State estado;
  ListaArquivos lista(&estado, ".gb");
    
  int scroll_index {}, ativo {-1};
  
  float screen_w = GetScreenWidth()/width;
  float screen_h = GetScreenHeight()/height;
  float scale = std::min(screen_w, screen_h);

  GuiSetStyle(BUTTON, TEXT_SIZE, scale*25.0f);
  GuiSetStyle(DEFAULT, TEXT_SIZE, scale*25.0f);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);
    
    if(IsWindowResized()){
      screen_w = GetScreenWidth()/width;
      screen_h = GetScreenHeight()/height;
      scale = std::min(screen_w, screen_h);
      GuiSetStyle(BUTTON, TEXT_SIZE, (scale*25.0f));
      GuiSetStyle(DEFAULT, TEXT_SIZE, (scale*25.0f));
    }

    if(apertado(estado.controles[11]))
      ToggleFullscreen();

    DrawText("CARLINHOS BOY", scale*325.0f, scale*80.0f, scale*150.0f, GOLD);
    DrawLine(scale*275.0f, scale*250.0f, scale*1625.0f, scale*250.0f, GOLD);

    for(size_t i {}; i < 5; ++i){
      Rectangle r = get_ret(550.0f, (320.0f + 135.0f*i), 300.0f, 100.0f);
      if(GuiButton(r, opcoes[i].c_str()))
        escolhas |= opt_escolha(i);
    }

    GuiListView(get_ret(1000.0f, 320.0f, 325.0f, 640.0f), lista.geral.c_str(), &scroll_index, &ativo);
    DrawText("Modo CGB: ", scale*1010.0f, scale*980.0f, scale*25.0f, GOLD);
    if(GuiButton(get_ret(1150.0f, 970.0f, 100.0f, 40.0f), (estado.paleta_cgb) ? "ON" : "OFF")){
      toggle_paleta(&estado);
    }

    if(ativo >= 0 && ativo < static_cast<int>(lista.arquivos.count)){
      inicia_emulador(lista.arquivos.paths[ativo], &estado);
      ativo = -1;

      screen_w = GetScreenWidth()/width;
      screen_h = GetScreenHeight()/height;
      scale = std::min(screen_w, screen_h);
      GuiSetStyle(BUTTON, TEXT_SIZE, (scale*25.0f));
      GuiSetStyle(DEFAULT, TEXT_SIZE, (scale*25.0f));
    }
    
    if(escolhas & opt_escolha(0)){
      escolhas &= ~opt_escolha(0);
      carrega_rom(&estado);

      screen_w = GetScreenWidth()/width;
      screen_h = GetScreenHeight()/height;
      scale = std::min(screen_w, screen_h);
      GuiSetStyle(BUTTON, TEXT_SIZE, (scale*25.0f));
      GuiSetStyle(DEFAULT, TEXT_SIZE, (scale*25.0f));
    }
    if(escolhas & opt_escolha(1)){
      escolhas &= ~opt_escolha(1);
      display_controles(&estado);

      screen_w = GetScreenWidth()/width;
      screen_h = GetScreenHeight()/height;
      scale = std::min(screen_w, screen_h);
      GuiSetStyle(BUTTON, TEXT_SIZE, (scale*25.0f));
      GuiSetStyle(DEFAULT, TEXT_SIZE, (scale*25.0f));
    }
    if(escolhas & opt_escolha(2)){
      escolhas &= ~opt_escolha(2);
      define_pasta(&estado, "saves_path", &lista);
    }
    if(escolhas & opt_escolha(3)){
      escolhas &= ~opt_escolha(3);
      define_pasta(&estado, "rom_path", &lista);
    }
    if(escolhas & opt_escolha(4)){
      break;
    }

    EndDrawing();
  }

  UnloadDirectoryFiles(lista.arquivos);
  CloseWindow();
}


}

