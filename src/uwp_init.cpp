#ifdef UWP_BUILDING
#include "uwp_init.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

namespace GB_UWP{

using namespace GB;

static constexpr float scr_width = 1920.0f;
static constexpr float scr_height = 1080.0f;
static constexpr const char *opcoes_menu[] = {"Abrir ROM", "Controles", "Definir pasta de saves", "Definir pasta de roms", "Sair"};

void init_gui(UWP_State *estado){
  SetConfigFlags(FLAG_VSYNC_HINT);
  InitWindow(scr_width, scr_height, "Carlinhos Boy");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);

  GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(BLACK));
  GuiSetStyle(LISTVIEW, TEXT_COLOR_NORMAL,  ColorToInt(BLACK));
  GuiSetStyle(LISTVIEW, TEXT_COLOR_FOCUSED,  ColorToInt(BLACK));
  GuiSetStyle(LISTVIEW, BASE_COLOR_FOCUSED, ColorToInt(GOLD));
  GuiSetStyle(LISTVIEW, BORDER_COLOR_FOCUSED, ColorToInt(GOLD));

  auto base = winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread().Bounds();
  estado->width = base.Width/scr_width;
  estado->height = base.Height/scr_height;
  SetWindowSize((int)base.Width, (int)base.Height);

  estado->scale = (std::min)(estado->width, estado->height);
  GuiSetStyle(BUTTON, TEXT_SIZE, estado->scale*25.0f);
  GuiSetStyle(DEFAULT, TEXT_SIZE, estado->scale*25.0f);
  GuiSetStyle(LISTVIEW, BORDER_WIDTH, estado->scale*5.0f);
  GuiSetStyle(LISTVIEW, SCROLLBAR_WIDTH, estado->scale*8.0f);
  GuiSetStyle(LISTVIEW, LIST_ITEMS_SPACING, estado->scale*1.0f);
  GuiSetStyle(LISTVIEW, LIST_ITEMS_HEIGHT, estado->scale*28.0f);
  estado->machine_state = uwp_gui_state::MAIN_MENU;
}

void run_gui(UWP_State *estado){
  switch(estado->machine_state){
    using enum uwp_gui_state;

    case MAIN_MENU:{
      run_main_menu(estado);
      break;
    }
    case CONTROLES_1:{
      run_control_menu1(estado);
      break;
    }
    case CONTROLES_2:{
      run_control_menu2(estado);
      break;
    }
    case SAVE_STATES:{
      run_saves_menu(estado);
      break;
    }
    case PAUSE_MENU:{
      run_pause_menu(estado);
      break;
    }
    default:{
      run_game(estado);
      break;
    }
  }
}

void init_game(UWP_State *estado){
  
}

void run_game(UWP_State *estado){

}

void run_main_menu(UWP_State *estado){

  int gamepad = GamepadDisponivel();
  int& contr_index = estado->contr_index;
  int& axis_timer = estado->axis_timer;
  int& ativo = estado->index_ativo;
  int& scroll_index = estado->scroll_index;
  int& focus = estado->focus;
  float& scale = estado->scale;
  uint8_t& escolhas = estado->escolhas = 0;
  bool& pad_ultimo = estado->state.pad_ultimo;
  bool& paleta_delay = estado->paleta_delay;
  bool& in_list = estado->in_list;

  auto redimensiona = [&](){
    estado->width = get_width()/scr_width;
    estado->height = get_height()/scr_height;
    estado->scale = (std::min)(estado->height, estado->width);
    SetWindowSize((int)get_width(), (int)get_height());
    GuiSetStyle(BUTTON, TEXT_SIZE, (scale*25.0f));
    GuiSetStyle(DEFAULT, TEXT_SIZE, (scale*25.0f));
    GuiSetStyle(LISTVIEW, BORDER_WIDTH, scale*5.0f);
    GuiSetStyle(LISTVIEW, SCROLLBAR_WIDTH, scale*8.0f);
    GuiSetStyle(LISTVIEW, LIST_ITEMS_SPACING, scale*1.0f);
    GuiSetStyle(LISTVIEW, LIST_ITEMS_HEIGHT, scale*28.0f);
  };

  auto controle_input = [&](int pad) {
      float leftStickY = fix_deadzone(GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_Y));
      float leftStickX = fix_deadzone(GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_X));

      if ((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_DOWN) || leftStickY > 0.5f) && !axis_timer) {
          contr_index = (contr_index + 1) % ((in_list) ? estado->lista.paths.size() : std::size(opcoes_menu) + 1);
          pad_ultimo = true;
          if (!axis_timer)
              axis_timer = 10;
          if (in_list) {
              int visible_itens = static_cast<int>((estado->scale * 640.0f) / (GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) + GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING)));
              if (contr_index == 0 && scroll_index) {
                  scroll_index = 0;
              }
              else if (contr_index - scroll_index > visible_itens - 1) {
                  ++scroll_index;
              }
          }
      }
      else if ((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_UP) || leftStickY < -0.5f) && !axis_timer) {
          --contr_index;
          if (contr_index < 0)
              contr_index = (in_list) ? estado->lista.paths.size() - 1 : std::size(opcoes_menu);

          pad_ultimo = true;
          if (!axis_timer)
              axis_timer = 10;
          if (in_list) {
              if (static_cast<size_t>(contr_index) == estado->lista.paths.size() - 1 && !scroll_index) {
                  int visible_itens = static_cast<int>((estado->scale * 640.0f) / (GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) + GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING)));
                  if (contr_index > visible_itens)
                      scroll_index = (estado->lista.paths.size() - visible_itens);
              }
              else if ((contr_index - scroll_index) < 0)
                  --scroll_index;
          }
      }
      else if ((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_LEFT) || leftStickX < -0.5f) && !axis_timer) {
          if (!in_list && contr_index == std::size(opcoes_menu)) {
              --contr_index;
          }
          else {
              if (estado->lista.paths.size()) {
                  contr_index = (in_list) ? 0 : scroll_index;
                  in_list ^= 1;
              }
              else {
                  contr_index = std::size(opcoes_menu);
              }
          }

          pad_ultimo = true;
          if (!axis_timer)
              axis_timer = 10;
      }
      else if ((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) || leftStickX > 0.5f) && !axis_timer) {
          if (estado->lista.paths.size()) {
              contr_index = (in_list) ? 0 : scroll_index;
              in_list ^= 1;
          }
          else {
              contr_index = std::size(opcoes_menu);
          }
          pad_ultimo = true;
          if (!axis_timer)
              axis_timer = 10;
      }
      };

  BeginDrawing();
  ClearBackground(BLACK);
    
  if(IsWindowResized()){
    redimensiona();
  }

  if(estado->switched){
     estado->switched = false;
     redimensiona();
  }

  if(gamepad > -1){
    controle_input(gamepad);
  }

  if(gamepad > -1 && (estado->open_delay == 1) && IsGamepadButtonReleased(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)){
    estado->open_delay = 2;
  }

  DrawText("CARLINHOS BOY", scale*325.0f, scale*80.0f, scale*150.0f, GOLD);
  DrawLine(scale*275.0f, scale*250.0f, scale*1625.0f, scale*250.0f, GOLD);

  for(size_t i {}; i < 5; ++i){
    Rectangle r = get_ret(550.0f, (320.0f + 135.0f*i), 300.0f, 100.0f);
    if(GuiButton(r, opcoes_menu[i])){
      escolhas |= opt_escolha(i);
      pad_ultimo = false;
    }
    if(pad_ultimo && !in_list && static_cast<size_t>(contr_index) == i){
      DrawRectangleLinesEx(get_ret(545.0f, 315.0f + 135.0f*i, 310.0f, 110.0f), scale*3.0f,GREEN);
      if(IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
        escolhas |= opt_escolha(i);
    }
  }
   
  int scroll_prev = scroll_index;
  GuiListViewEx(get_ret(1000.0f, 320.0f, 325.0f, 640.0f), (char**)estado->lista.nomes.data(), estado->lista.paths.size(), &scroll_index, &ativo, &focus);
  if(gamepad > -1 && in_list){
    float offset = GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING) + GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
    if(scroll_index != scroll_prev){
        contr_index += (scroll_index - scroll_prev);
    }

    DrawRectangleLinesEx(Rectangle{scale*1003.0f, scale*323.0f + contr_index*offset, scale*317.0f, offset - scale*3.0f}, scale*3.0f, GREEN);
    if(IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)){
      ativo = contr_index;
      estado->open_delay = 1;
    }
  }

  DrawText("Modo CGB: ", scale*1010.0f, scale*980.0f, scale*25.0f, GOLD);
  if(gamepad > -1 && !in_list && contr_index == 5){
    DrawRectangleLines(scale*1145.0f, scale*965.0f, scale*110.0f, scale*50.0f, GREEN);
    if(IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
      paleta_delay = true;
  }

  bool paleta_trocada = (paleta_delay && IsGamepadButtonReleased(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));
  if(GuiButton(get_ret(1150.0f, 970.0f, 100.0f, 40.0f), (estado->state.paleta_cgb) ? "ON" : "OFF") || paleta_trocada){
    paleta_delay = false;
    toggle_paleta(&(estado->state));
  }

  if((ativo >= 0 && ativo < static_cast<int>(estado->lista.arquivos1.count + estado->lista.arquivos2.count) && !estado->open_delay) || estado->open_delay == 2){
    inicia_emulador(estado->lista.paths[ativo], &estado->state);
    ativo = -1;
    estado->open_delay = 0;
  }
    
  if(escolhas & opt_escolha(0)){
    escolhas &= ~opt_escolha(0);
    carrega_rom(&estado->state);
  }
  if(escolhas & opt_escolha(1)){
    escolhas &= ~opt_escolha(1);
    estado->machine_state = uwp_gui_state::CONTROLES_1;
  }
  if(escolhas & opt_escolha(2)){
    escolhas &= ~opt_escolha(2);
    define_pasta(&estado->state, "saves_path", &estado->lista);
  }
  if(escolhas & opt_escolha(3)){
    escolhas &= ~opt_escolha(3);
    define_pasta(&estado->state, "rom_path", &estado->lista);
  }
  if(escolhas & opt_escolha(4)){
    escolhas &= ~opt_escolha(4);
    estado->quitted = true;
    EndDrawing();
  }
  if(axis_timer)
    --axis_timer;

  EndDrawing();
}

void run_pause_menu(UWP_State *estado){
  
}

void run_saves_menu(UWP_State *estado){

}

void run_control_menu1(UWP_State *estado){

}

void run_control_menu2(UWP_State *estado){

}



}

#endif
