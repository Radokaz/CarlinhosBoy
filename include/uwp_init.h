#ifndef UWP_INIT
#define UWP_INIT

#include "game_state.h"

namespace GB_UWP{

enum class uwp_gui_state: uint8_t{
  MAIN_MENU = 0,
  CONTROLES_1,
  CONTROLES_2,
  SAVE_STATES,
  PAUSE_MENU,
  JOGO
};

struct UWP_State{
  GB::GB_State state;
  GB::ListaArquivos lista{&state};
  std::unique_ptr<GB::Game_State> game {};
  float width {};
  float height {};
  float scale {};
  int scroll_index {};
  int index_ativo {-1};
  int contr_index {};
  int axis_timer {};
  uwp_gui_state machine_state {};
  uint8_t escolhas {};
  uint8_t open_delay {};
  bool paleta_delay {false};
  bool in_list {false};
  bool switched {false};
  bool quitted {false};
};

void init_gui(UWP_State *estado);
void run_gui(UWP_State *estado);
void run_game(UWP_State *estado);
void run_main_menu(UWP_State *estado);
void run_pause_menu(UWP_State *estado);
void run_saves_menu(UWP_State *estado);
void run_control_menu1(UWP_State *estado);
void run_control_menu2(UWP_State *estado);

}

#endif
