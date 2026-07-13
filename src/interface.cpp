#include "interface.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

namespace GB{

void display_controles(GB_State *estado){
  ClearBackground(BLACK);

  const char *botoes[std::size(gb_botoes)];
  for(size_t i {}; i < std::size(gb_botoes); ++i){
    botoes[i] = GetKeyName(estado->controles[i]);
  }
  bool frame_inicial = true;

  while(1){
    BeginDrawing();
    ClearBackground(BLACK);

    DrawRectangle(1750, 400, 500, 900, BLACK);
    
    DrawText("CARLINHOS BOY", 650, 80, 150, GOLD);
    DrawLine(565, 250, 2000, 250, GRAY);

    for(size_t i {}; i < 6; ++i){
      Rectangle r(650, 400 + 200*i, 500, 100);

      if(i == 3 || i == 2 || i == 5){
        DrawText(gb_botoes[i], 850, 350 + 200*i, 25, GOLD);
      }
      else if(i == 4){
        DrawText(gb_botoes[i], 875, 350 + 200*i, 25, GOLD);
      }
      else{
        DrawText(gb_botoes[i], 900, 350 + 200*i, 25, GOLD);
      }

      if(GuiButton(r, botoes[i]) && !frame_inicial){
        int tecla = 0;
        while(tecla == 0){
          BeginDrawing();
          ClearBackground(BLACK);
          DrawText("Aperte alguma tecla...", 1150, 400, 30, GOLD);
          EndDrawing();
          tecla = GetKeyPressed();
        }
        if(tecla != 0){
          estado->controles[i] = static_cast<KeyboardKey>(tecla);
          botoes[i] = GetKeyName(estado->controles[i]);
        }
      }
    }

    for(size_t i {6}; i < 11; ++i){
      Rectangle r(1400, 400 + 200*(i - 6), 500, 100);
      if(i == 8){
        DrawText(gb_botoes[i], 1550, 350 + 200*(i - 6), 25, GOLD);
      }
      else{
        DrawText(gb_botoes[i], 1600, 350 + 200*(i - 6), 25, GOLD);
      }
      
      if(GuiButton(r, botoes[i]) && !frame_inicial){
        int tecla = 0;
        while(tecla == 0){
          BeginDrawing();
          ClearBackground(BLACK);
          DrawText("Aperte alguma tecla...", 1150, 400, 30, GOLD);
          EndDrawing();
          tecla = GetKeyPressed();
        }
        if(tecla != 0){
          estado->controles[i] = static_cast<KeyboardKey>(tecla);
          botoes[i] = GetKeyName(estado->controles[i]);
        }
      }

      frame_inicial = false;
    }

    DrawText("Aperte ESC para voltar", 1500, 1500, 30, GOLD);

    if(apertado(KEY_ESCAPE)){
      estado->atualiza_controles();
      break;
    }

    EndDrawing();
  }
}

bool pausa_jogo(CPU *cpu, GB_State *estado, bool& pausado){
  if(!pausado) return false; 

  BeginDrawing();
  ShowCursor();
  EndDrawing();
  
  const char opcoes[3][10] = {"Resumir", "Controles", "Sair"};
  uint8_t escolhas {};
  while(1){
    BeginDrawing();
    ClearBackground(BLACK);

    if(apertado(KEY_ESCAPE) || apertado(estado->controles[9]))
      break;

    DrawText("PAUSE", 1000, 80, 150, GOLD);
    DrawLine(600, 250, 1850, 250, GRAY);

    for(size_t i {}; i < 3; ++i){
      Rectangle r(1000, 400 + 150*i, 500, 100);
      if(GuiButton(r, opcoes[i]))
        escolhas |= opt_escolha(i);
    }

    if(escolhas & opt_escolha(0)){
      escolhas &= ~opt_escolha(0);
      break;
    }
    if(escolhas & opt_escolha(1)){
      escolhas &= ~opt_escolha(1);
      display_controles(estado);
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
  auto monitor = GetCurrentMonitor();

  InitWindow(GetMonitorWidth(monitor), GetMonitorHeight(monitor), "Carlinhos Boy");
  SetTargetFPS(60);
  
  std::string opcoes[] = {
    "Abrir ROM", "Controles", "Definir pasta de saves", "Definir pasta de roms", "Sair",
  };

  uint8_t escolhas {};

  GB_State estado;
  ListaArquivos lista(&estado, ".gb");
    
  int scroll_index {}, ativo {-1};
  bool mostra_lista {true};

  GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
  GuiSetStyle(BUTTON, TEXT_SIZE, 24);
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    DrawText("CARLINHOS BOY", 650, 80, 150, GOLD);
    DrawLine(565, 250, 2000, 250, GRAY);

    for(size_t i {}; i < 5; ++i){
      Rectangle r(1000, 400 + 150*i, 500, 100);
      if(GuiButton(r, opcoes[i].c_str()))
        escolhas |= opt_escolha(i);
    }

    GuiListView({1750, 400, 500, 900}, lista.geral.c_str(), &scroll_index, &ativo);
    DrawText("Modo CGB", 1750, 1400, 23, GOLD);
    if(GuiButton(Rectangle{1900, 1380, 100, 50}, (estado.paleta_cgb) ? "ON" : "OFF")){
      toggle_paleta(&estado);
    }

    if(ativo >= 0 && ativo < static_cast<int>(lista.arquivos.count)){
      inicia_emulador(lista.arquivos.paths[ativo], &estado);
      ativo = -1;
    }
    
    if(escolhas & opt_escolha(0)){
      mostra_lista = false;
      escolhas &= ~opt_escolha(0);
      carrega_rom(&estado);
      mostra_lista = true;
    }
    if(escolhas & opt_escolha(1)){
      escolhas &= ~opt_escolha(1);
      display_controles(&estado);
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

