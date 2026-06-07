#include "init.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "tinyfiledialogs.h"

#define opt_escolha(x) (1 << x)
#define state_path "state.cfg"

namespace GB{

struct GB_State{
  std::string rom_path;
  std::string saves_path;

  GB_State(void){
    std::fstream estado(state_path, estado.in);

    if(!estado){
      estado.close();
      std::ofstream novo(state_path);
      novo << "rom_path: ROMS\n";
      novo << "saves_path: Saves\n";

      rom_path = "ROMS";
      saves_path = "Saves";

      return;
    }

    std::string buffer;
    while(std::getline(estado, buffer)){
    
      size_t pos = buffer.find(':');
      if(pos != std::string::npos){
      
        std::string key_buffer = buffer.substr(0, pos);
        std::string value_buffer = buffer.substr(pos + 1);

        size_t ini = value_buffer.find_first_not_of(" ");
        if(ini != std::string::npos)
          value_buffer = value_buffer.substr(ini);

        if(key_buffer == "rom_path")
          rom_path = value_buffer;
        else if(key_buffer == "saves_path")
          saves_path = value_buffer;
      }
    }
  }
};

struct ListaArquivos{
  
  FilePathList arquivos;
  std::string geral;

  ListaArquivos(GB_State *estado){
    arquivos = LoadDirectoryFilesEx(estado->rom_path.c_str(), ".gb", true);
    this->atualiza_string();
  }

  void atualiza_string(void){
    geral.clear();
    TraceLog(LOG_INFO, "Arquivos encontrados: %d", arquivos.count);
    for(int64_t i {}; i < arquivos.count; ++i){
      TraceLog(LOG_INFO, " %s", arquivos.paths[i]);
      geral+=GetFileName(arquivos.paths[i]);
      if(i < (static_cast<int64_t>(arquivos.count) - 1))
        geral+=';';
    }
  }

  void atualiza_lista(GB_State *estado){
    UnloadDirectoryFiles(arquivos);
    arquivos = LoadDirectoryFilesEx(estado->rom_path.c_str(), ".gb", true);
    this->atualiza_string();
  }

};
  
inline void carrega_rom(GB_State *estado){
    const char *extensoes[] = {"*.gb"};

    const char *resultado = tinyfd_openFileDialog(
        "Escolha a rom",  // título
        "",               // pasta inicial
        1,                // número de filtros
        extensoes,          // extensões permitidas
        "", // descrição
        0                 // 0 = um arquivo, 1 = múltiplos
    );

    if(!resultado) return;

    inicia_emulador(resultado, estado->saves_path);
}

inline void define_pasta(GB_State *estado, std::string_view pasta, ListaArquivos *lista){
  const char *resultado = tinyfd_selectFolderDialog("Selecione uma pasta", "");
  if(!resultado) return;

  std::fstream arquivo(state_path, arquivo.in | arquivo.out);
  std::string result = resultado, buffer{};
  std::vector<std::string> linhas;

  while(std::getline(arquivo, buffer)){
    linhas.push_back(buffer);
  }
  arquivo.close();
  std::ofstream novo(state_path);

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

inline void init_gui(void){
  auto monitor = GetCurrentMonitor();

  InitWindow(GetMonitorWidth(monitor), GetMonitorHeight(monitor), "Carlinhos Boy");
  SetTargetFPS(60);
  
  std::string opcoes[4] = {
    "Abrir ROM", "Definir pasta de saves", "Definir pasta de roms", "Sair"
  };

  uint8_t escolhas {};
  std::filesystem::path rom_path = std::string("ROMS");

  GB_State estado;
  ListaArquivos lista(&estado);
    
  int scroll_index {}, ativo {-1};
  bool mostra_lista {true};

  GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
  GuiSetStyle(BUTTON, TEXT_SIZE, 24);
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    DrawText("CARLINHOS BOY", 650, 80, 150, GOLD);
    DrawLine(565, 250, 2000, 250, GRAY);

    for(size_t i {}; i < 4; ++i){
      Rectangle r(1000, 400 + 150*i, 500, 100);
      if(GuiButton(r, opcoes[i].c_str()))
        escolhas ^= opt_escolha(i);
    }

    GuiListView({1750, 400, 500, 800}, lista.geral.c_str(), &scroll_index, &ativo);

    if(ativo >= 0 && ativo < static_cast<int>(lista.arquivos.count)){
      inicia_emulador(lista.arquivos.paths[ativo], estado.saves_path);
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
      define_pasta(&estado, "rom_path", &lista);
    }
    if(escolhas & opt_escolha(2)){
      escolhas &= ~opt_escolha(2);
      define_pasta(&estado, "saves_path", &lista);
    }
    if(escolhas & opt_escolha(3)){
      break;
    }

    EndDrawing();
  }

  UnloadDirectoryFiles(lista.arquivos);
  CloseWindow();
}

}
