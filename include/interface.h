#ifndef INTERFACE_H
#define INTERFACE_H

#include "actions.h"
#include <string>
#include <cstring>
#include <filesystem>
#include <fstream>

#ifdef UWP_BUILDING
  #include <windows.h>
  #include <winrt/base.h>
  #include <winrt/Windows.Foundation.h>
  #include <winrt/Windows.Foundation.Collections.h>
  #include <winrt/Windows.ApplicationModel.h>
  #include <winrt/Windows.Storage.h>
  #include <winrt/Windows.Storage.Pickers.h>
  #include <winrt/Windows.UI.Core.h>
  #include <winrt/Windows.UI.ViewManagement.h>
  #include <future>
#else
  #include "tinyfiledialogs.h"
#endif

#define opt_escolha(x) (1 << x)

namespace GB{

inline std::filesystem::path getExeDir() {
  return std::filesystem::path(GetApplicationDirectory());
}

static constexpr char gb_botoes[14][15] = {
    "A", "B", "START", "SELECT", "UP", "LEFT", "DOWN", "RIGHT", "LCD_TOGGLE", "MENU", "TURBO", "FULLSCREEN", "SAVE_STATE", "LOAD_STATE"
};

const char *getDisplayName(KeyboardKey key);
const char *getDisplayName(GamepadButton but);
Rectangle get_ret(float x, float y, float w, float h);
int GamepadDisponivel(void);
float fix_deadzone(float dz);
float get_width(void);
float get_height(void);

struct GamepadComb{
  GamepadButton but1;
  GamepadButton but2;

  GamepadComb(void): but1{GAMEPAD_BUTTON_UNKNOWN}, but2{GAMEPAD_BUTTON_UNKNOWN} {}
  GamepadComb(GamepadButton b1): but1{b1}, but2{GAMEPAD_BUTTON_UNKNOWN} {}
  GamepadComb(GamepadButton b1, GamepadButton b2): but1{b1}, but2{b2} {}
  explicit GamepadComb(size_t hash){
    but1 = static_cast<GamepadButton>(hash & 0xFFFFFFFF);
    but2 = static_cast<GamepadButton>((hash & 0xFFFFFFFF00000000) >> 32);
  }

  bool pressionado(int gamepad) const{
    if(but2)
      return IsGamepadButtonPressed(gamepad, but1) && IsGamepadButtonPressed(gamepad, but2);;

    return IsGamepadButtonPressed(gamepad, but1);
  }

  bool segurado(int gamepad) const{
    if(but2)
      return IsGamepadButtonDown(gamepad, but1) && IsGamepadButtonDown(gamepad, but2);;

    return IsGamepadButtonDown(gamepad, but1);
  }

  size_t hash(void) const{
    return (static_cast<size_t>(but1) & 0xFFFFFFFF) | ((static_cast<size_t>(but2) & 0xFFFFFFFF) << 32);
  }

  std::string string(void) const{
    const char *segundo = getDisplayName(but2);
    return (std::strlen(segundo)) ? std::string(getDisplayName(but1)) + " + " + segundo : std::string(getDisplayName(but1));
  }
};

struct GB_State{
  std::array<KeyboardKey, 14> controles;
  std::array<GamepadComb, 14> controles_but;
  std::filesystem::path main_dir;
  std::string rom_path;
  std::string saves_path;
  std::string states_path;
  size_t save_slot {};
  int paleta_cgb {};
  bool pad_ultimo {false};

  GB_State(void){
#ifdef UWP_BUILDING
    main_dir = this->uwp_dir();
#elifdef _WIN32
    main_dir = getExeDir();
#else
    main_dir = this->linux_dir();
#endif

    pad_ultimo = (GamepadDisponivel() > -1);
    this->seta_controles();

    std::filesystem::path state_path = main_dir / "state.cfg";
    std::fstream estado(state_path, std::ios::in);
    save_slot = 1;
    if(!estado){
      estado.close();
      std::ofstream novo(state_path);
      paleta_cgb = 1;

      std::filesystem::path svs = main_dir / "Saves";
      std::filesystem::path roms = main_dir / "ROMS";
      std::filesystem::path states = main_dir / "States";
      std::filesystem::create_directories(svs);
      std::filesystem::create_directories(roms);
      std::filesystem::create_directories(states);
      novo << "rom_path: " << roms.string() << "\n";
      novo << "saves_path: " << svs.string() << "\n";
      novo << "states_path: " << states.string() << "\n";
      novo << "paleta_cgb: " << std::to_string(paleta_cgb) << "\n";

      rom_path = roms.string();
      saves_path = svs.string();
      states_path = states.string();

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
        else if(key_buffer == "states_path")
          states_path = value_buffer;
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

#ifdef UWP_BUILDING
  std::filesystem::path uwp_dir(void){
    auto pasta = winrt::Windows::Storage::ApplicationData::Current().LocalFolder();
    return std::filesystem::path(std::wstring(pasta.Path()));
  }
#endif

  void seta_controles(void){
    std::filesystem::path control_path = main_dir / "controles.cfg";
    std::fstream control(control_path, std::ios::in | std::ios::out);

    if(!control){
      control.close();
      std::ofstream controle_novo(control_path);
      controles = {KEY_M, KEY_N, KEY_O, KEY_P, KEY_W, KEY_A, KEY_S, KEY_D, KEY_T, KEY_C, KEY_F, KEY_F11, KEY_F1, KEY_F2};

      controles_but = {{GAMEPAD_BUTTON_RIGHT_FACE_DOWN, GAMEPAD_BUTTON_RIGHT_FACE_LEFT, GAMEPAD_BUTTON_MIDDLE_RIGHT, 
      GAMEPAD_BUTTON_MIDDLE_LEFT, GAMEPAD_BUTTON_LEFT_FACE_UP, GAMEPAD_BUTTON_LEFT_FACE_LEFT, GAMEPAD_BUTTON_LEFT_FACE_DOWN,
      GAMEPAD_BUTTON_LEFT_FACE_RIGHT, GAMEPAD_BUTTON_LEFT_TRIGGER_1, {GAMEPAD_BUTTON_MIDDLE_LEFT, GAMEPAD_BUTTON_MIDDLE_RIGHT}, 
      GAMEPAD_BUTTON_RIGHT_TRIGGER_2, GAMEPAD_BUTTON_LEFT_TRIGGER_2, GAMEPAD_BUTTON_LEFT_THUMB, GAMEPAD_BUTTON_RIGHT_THUMB}};
      
      for(size_t i {}; i < std::size(gb_botoes); ++i){
        controle_novo << gb_botoes[i] << ": " << std::to_underlying<KeyboardKey>(controles[i]) << "/" << controles_but[i].hash() << "\n";
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
      size_t end = value.find_first_of("/");

      if(ini != std::string::npos){
        controles[i] = static_cast<KeyboardKey>(std::stoi(value.substr(ini, end - ini)));
        controles_but[i] = GamepadComb(static_cast<size_t>(std::stoull(value.substr(end + 1))));
      }

      ++i;
    }
  }

  void atualiza_controles(void){
    std::filesystem::path control_path = main_dir / "controles.cfg";
    std::fstream control(control_path, std::ios::in | std::ios::out);
    
    std::array<std::string, std::size(gb_botoes)> linhas;
    std::string buffer;

    for(size_t count {}; std::getline(control, buffer); ++count){
      linhas[count] = buffer;
    }
    control.close();
    buffer.clear();

    std::ofstream novo(control_path);

    for(size_t i {}; i < linhas.size(); ++i){
      size_t pos = linhas[i].find(':');
      for(size_t j {}; j < std::size(gb_botoes); ++j){
        if(!std::strncmp(linhas[i].c_str(), gb_botoes[j], pos)){
          linhas[i].erase(pos + 2);
          buffer = std::to_string(std::to_underlying<KeyboardKey>(controles[j])) + "/" + std::to_string(controles_but[j].hash());
          linhas[i].append(buffer);
          break;
        }
      }
      novo << linhas[i] << "\n";
    }
  }

};

struct ListaArquivos{
  
  FilePathList arquivos1;
  FilePathList arquivos2;
  std::string geral;
  std::vector<const char *> paths;

  ListaArquivos(GB_State *estado){
    arquivos1 = LoadDirectoryFilesEx(estado->rom_path.c_str(), ".gb", false);
    arquivos2 = LoadDirectoryFilesEx(estado->rom_path.c_str(), ".gbc", false);
    this->atualiza_string();
  }

  void atualiza_string(void){
    geral.clear();
    size_t tamanho = static_cast<size_t>(arquivos1.count) + arquivos2.count;
    paths.resize(tamanho);
    TraceLog(LOG_INFO, "Arquivos encontrados: %d", tamanho);

    char **aux = arquivos1.paths;
    size_t off {};
    for(size_t i {}; i < tamanho; ++i){
      if(i == arquivos1.count){
        aux = arquivos2.paths;
        off = arquivos1.count;
      }
      paths[i] = aux[i - off];
    }

    std::sort(paths.begin(), paths.end(), [](const char *a, const char *b){ return std::strcmp(a, b) < 0; });
    for(size_t i {}; i < tamanho; ++i){
      geral+=GetFileName(paths[i]);
      if(i < (tamanho - 1))
        geral+=';';
    }
  }

  void atualiza_lista(GB_State *estado){
    UnloadDirectoryFiles(arquivos1);
    UnloadDirectoryFiles(arquivos2);
    arquivos1 = LoadDirectoryFilesEx(estado->rom_path.c_str(), ".gb", false);
    arquivos2 = LoadDirectoryFilesEx(estado->rom_path.c_str(), ".gbc", false);
    this->atualiza_string();
  }

  ~ListaArquivos(){
    UnloadDirectoryFiles(arquivos1);
    UnloadDirectoryFiles(arquivos2);
  }

};

void debug_func(CPU *cpu);
void inicia_emulador(std::string_view src, GB_State *estado);
void carrega_rom(GB_State *estado);
void define_pasta(GB_State *estado, std::string_view pasta, ListaArquivos *lista);
void toggle_paleta(GB_State* estado);
void display_controles(GB_State *estado);
void init_gui(void);

}

#endif
