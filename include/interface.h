#ifndef INTERFACE_H
#define INTERFACE_H

#include "actions.h"
#include <string>
#include <fstream>
#include "tinyfiledialogs.h"

#define opt_escolha(x) (1 << x)

namespace GB{

inline std::filesystem::path getExeDir() {
  return std::filesystem::path(GetApplicationDirectory());
}

static constexpr char gb_botoes[11][15] = {
    "A", "B", "START", "SELECT", "UP", "LEFT", "DOWN", "RIGHT", "LCD_TOGGLE", "MENU", "TURBO"
};

struct GB_State{
  std::array<KeyboardKey, 11> controles;
  std::filesystem::path main_dir;
  std::string rom_path;
  std::string saves_path;
  int paleta_cgb {};

  GB_State(void){
#ifdef _WIN32
    main_dir = getExeDir();
#else
    main_dir = this->linux_dir();
#endif

    std::filesystem::path state_path = main_dir / "state.cfg";
    std::fstream estado(state_path.string().c_str(), estado.in);

    this->seta_controles();
    if(!estado){
      estado.close();
      std::ofstream novo(state_path.string().c_str());
      paleta_cgb = 1;

      std::filesystem::path svs = main_dir / "Saves";
      std::filesystem::path roms = main_dir / "ROMS";
      std::filesystem::create_directories(svs);
      std::filesystem::create_directories(roms);
      novo << "rom_path: " << roms.string() << "\n";
      novo << "saves_path: " << svs.string() << "\n";
      novo << "paleta_cgb: " << std::to_string(paleta_cgb) << "\n";

      rom_path = roms.string();
      saves_path = svs.string();

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
        else if(key_buffer == "paleta_cgb")
          paleta_cgb = std::stoi(value_buffer);
      }
    }
  }

  std::filesystem::path linux_dir(void){
    const char* xdgData = std::getenv("XDG_DATA_HOME");
    std::filesystem::path base;

    if(xdgData != nullptr && xdgData[0] != '\0'){
      base = std::filesystem::path(xdgData);
    } 
    else{
      const char* home = std::getenv("HOME");
      if(!home){
        home = "/tmp";
      }
      base = std::filesystem::path(home) / ".local" / "share";
    }

    std::filesystem::path dir = base / "CarlinhosBoy";
    std::filesystem::create_directories(dir);
    return dir;
  }

  void seta_controles(void){
    std::filesystem::path control_path = main_dir / "controles.cfg";
    std::fstream control(control_path.string().c_str(), control.in | control.out);

    if(!control){
      control.close();
      std::ofstream controle_novo(control_path.string().c_str());
      controles = {KEY_M, KEY_N, KEY_O, KEY_P, KEY_W, KEY_A, KEY_S, KEY_D, KEY_T, KEY_C, KEY_F};
      
      for(size_t i {}; i < std::size(gb_botoes); ++i){
        controle_novo << gb_botoes[i] << ": " << std::to_underlying<KeyboardKey>(controles[i]) << "\n";
      }

      return;
    }

    std::string buffer;
    size_t i {};
    while(std::getline(control, buffer)){
      size_t pos = buffer.find(':');
      if(pos == std::string::npos) continue;
        
      std::string value = buffer.substr(pos + 1);
      size_t ini = value.find_first_not_of(" ");

      if(ini != std::string::npos){
        controles[i] = static_cast<KeyboardKey>(std::stoi(value.substr(ini)));
      }

      ++i;
    }
  }

  void atualiza_controles(void){
    std::filesystem::path control_path = main_dir / "controles.cfg";
    std::fstream control(control_path.string().c_str(), control.in | control.out);
    
    std::vector<std::string> linhas;
    std::string buffer;

    while(std::getline(control, buffer)){
      linhas.push_back(buffer);
    }
    control.close();

    std::ofstream novo(control_path.string().c_str());

    for(size_t i {}; i < linhas.size(); ++i){
      size_t pos = linhas[i].find(':');
      if(pos == std::string::npos){ 
        novo << linhas[i] << "\n";
        continue;
      }

      std::string key = linhas[i].substr(0, pos);
      for(size_t i {}; i < std::size(gb_botoes); ++i){
        if(key == gb_botoes[i]){
          linhas[i].replace(pos + 2, linhas[i].length(), std::to_string(std::to_underlying<KeyboardKey>(controles[i])));
          break;
        }
      }
      novo << linhas[i] << "\n";
    }
  }

};

struct ListaArquivos{
  
  FilePathList arquivos;
  const char *extensao {};
  std::string geral;

  ListaArquivos(GB_State *estado, const char *ext): extensao{ext}{
    arquivos = LoadDirectoryFilesEx(estado->rom_path.c_str(), extensao, true);
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
    arquivos = LoadDirectoryFilesEx(estado->rom_path.c_str(), extensao, true);
    this->atualiza_string();
  }

};

void debug_func(CPU *cpu);
bool pausa_jogo(CPU *cpu, GB_State *estado, bool& pausado);
void inicia_emulador(std::string_view src, GB_State *estado);
void carrega_rom(GB_State *estado);
void define_pasta(GB_State *estado, std::string_view pasta, ListaArquivos *lista);
void display_controles(GB_State *estado);
void init_gui(void);

}

#endif
