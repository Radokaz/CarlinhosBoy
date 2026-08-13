#include "interface.h"
#include "game_state.h"
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

const char *getDisplayName(GamepadButton but){
  switch(but){
    case GAMEPAD_BUTTON_LEFT_FACE_UP: return "Pad Up";
    case GAMEPAD_BUTTON_LEFT_FACE_RIGHT: return "Pad Right";
    case GAMEPAD_BUTTON_LEFT_FACE_DOWN: return "Pad Down";
    case GAMEPAD_BUTTON_LEFT_FACE_LEFT: return "Pad Left";
    case GAMEPAD_BUTTON_RIGHT_FACE_UP: return "Y";
    case GAMEPAD_BUTTON_RIGHT_FACE_RIGHT: return "B";
    case GAMEPAD_BUTTON_RIGHT_FACE_DOWN: return "A";
    case GAMEPAD_BUTTON_RIGHT_FACE_LEFT: return "X";
    case GAMEPAD_BUTTON_LEFT_TRIGGER_1: return "LB";
    case GAMEPAD_BUTTON_LEFT_TRIGGER_2: return "LT";
    case GAMEPAD_BUTTON_RIGHT_TRIGGER_1: return "RB";
    case GAMEPAD_BUTTON_RIGHT_TRIGGER_2: return "RT";
    case GAMEPAD_BUTTON_MIDDLE_LEFT: return "Select";
    case GAMEPAD_BUTTON_MIDDLE: return "Guide";
    case GAMEPAD_BUTTON_MIDDLE_RIGHT: return "Start";
    case GAMEPAD_BUTTON_LEFT_THUMB: return "L3";
    case GAMEPAD_BUTTON_RIGHT_THUMB: return "R3";
    default: return "";  
  }
}


Rectangle get_ret(float x, float y, float w, float h){
  constexpr float width = 1920.0f;
  constexpr float height = 1080.0f;
  float screen_w = GetScreenWidth()/width;
  float screen_h = GetScreenHeight()/height;
  float scale = (std::min)(screen_w, screen_h);
  return Rectangle{scale*x, scale*y, scale*w, scale*h};
}

#ifdef UWP_BUILDING
int GamepadDisponivel(void){
  for(int i {}; i < 4; ++i){
    if(IsGamepadAvailable(i)) return i;
  }

  return -1;
}
#else
int GamepadDisponivel(void){
  for(int i {}; i < 4; ++i){
    if(!IsGamepadAvailable(i)) return -1;
    if(GetGamepadAxisCount(i) < 2) continue;
    const char *name = GetGamepadName(i);
    if(!name || !strlen(name)) continue;
    if(strstr(name, "MOUSE") != nullptr) continue;
    if(strstr(name, "KEYBOARD") != nullptr) continue;

    return i;
  }

  return -1;
}
#endif

float fix_deadzone(float dz){
  return (std::fabsf(dz) < 0.08f) ? 0.0f : dz;
}

float get_width(void){
#ifdef UWP_BUILDING
  return winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread().Bounds().Width;
#else
  return GetScreenWidth();
#endif
}

float get_height(void){
#ifdef UWP_BUILDING
  return winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread().Bounds().Height;
#else
  return GetScreenHeight();
#endif
}

void display_controles(GB_State *estado){
  const char *botoes[std::size(gb_botoes)];
  std::string botoes_controle[std::size(gb_botoes)];
  std::string botoes_show[std::size(gb_botoes)];

  for(size_t i {}; i < std::size(gb_botoes); ++i){
    botoes[i] = getDisplayName(estado->controles[i]);
    botoes_controle[i] = estado->controles_but[i].string();
    botoes_show[i] = (std::string(gb_botoes[i]) + " : ");
  }

  constexpr const char *subs[2] = {"Aperte ESC para voltar", "Aperte B para voltar"};
  constexpr float width = 1920.0f;
  constexpr float height = 1080.0f;
  float screen_w = get_width()/width;
  float screen_h = get_height()/height;
  float scale = (std::min)(screen_w, screen_h);
  GuiSetStyle(DEFAULT, TEXT_SIZE, scale*25.0f);
  GuiSetStyle(BUTTON, TEXT_SIZE, scale*25.0f);
  int8_t contr_index {};
  size_t slot_index {};
  int axis_timer {};
  bool key_consumed {false};
  bool pad_consumed {false};
  bool tecla_apertada {true};

  auto redimensiona = [&](){
    screen_w = get_width()/width;
    screen_h = get_height()/height;
    scale = (std::min)(screen_w, screen_h);
    GuiSetStyle(DEFAULT, TEXT_SIZE, scale*25.0f);
    GuiSetStyle(BUTTON, TEXT_SIZE, scale*25.0f);
  };

  bool& pad_ultimo = estado->pad_ultimo;
  auto controle_input = [&](int pad){

    float leftStickX = fix_deadzone(GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_X));
    float leftStickY = fix_deadzone(GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_Y));

    if((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) || leftStickX > 0.5f) && !axis_timer){
        if(contr_index + 8 >= static_cast<int>(std::size(gb_botoes))){
          if(contr_index > 5 && contr_index < 8)
            contr_index = std::size(gb_botoes) - 1;
          else
            contr_index-=8;
        }
        else
          contr_index+=8;

        pad_ultimo = true;
        if(!axis_timer)
          axis_timer = 10;
      }
      else if((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_LEFT) || leftStickX < -0.5f) && !axis_timer){
        if(contr_index - 8 < 0){
          if(contr_index + 8 >= static_cast<int>(std::size(gb_botoes)))
            contr_index = std::size(gb_botoes) - 1;
          else
            contr_index += 8;
        }
        else{
          contr_index-=8;
        }

        pad_ultimo = true;
        if(!axis_timer)
          axis_timer = 10;
      }
      else if((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_DOWN) || leftStickY > 0.5f) && !axis_timer){
        ++contr_index;
        if(contr_index == 8)
          contr_index = 0;
        else if(contr_index == static_cast<int>(std::size(gb_botoes)))
          contr_index = 8;

        pad_ultimo = true;
        if(!axis_timer)
          axis_timer = 10;
      }
      else if((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_UP) || leftStickY < -0.5f) && !axis_timer){
        --contr_index;
        if(contr_index == 7)
          contr_index = std::size(gb_botoes) - 1;
        else if(contr_index < 0)
          contr_index = 7;

        pad_ultimo = true;
        if(!axis_timer)
          axis_timer = 10;
      }
  };

  while(1){
    BeginDrawing();
    ClearBackground(BLACK);
    
    if(IsWindowResized()){
      redimensiona();
    }

    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
      pad_ultimo = false;
    }
    
    DrawText("CONTROLES", scale*500.0f, scale*80.0f, scale*150.0f, GOLD);
    DrawLine(scale*275.0f, scale*250.0f, scale*1625.0f, scale*250.0f, GOLD);

    int gamepad = GamepadDisponivel();
    if(gamepad > -1){
      controle_input(gamepad);
    }

    if(key_consumed && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)){
      int tecla = 0;
      pad_ultimo = false;
      while(tecla == 0){
        if(IsWindowResized()){
          redimensiona();
        }

        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Pressione alguma tecla...", scale*800.0f, screen_h*400.0f, scale*30, GOLD);
        EndDrawing();
        tecla = GetKeyPressed();
      }
      estado->controles[slot_index] = static_cast<KeyboardKey>(tecla);
      botoes[slot_index] = getDisplayName(estado->controles[slot_index]);
      tecla_apertada = true;
      key_consumed = false;
    }

    if(pad_consumed && IsGamepadButtonReleased(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)){
      int count {};
      pad_ultimo = true;
      GamepadButton comb[2] = {GAMEPAD_BUTTON_UNKNOWN, GAMEPAD_BUTTON_UNKNOWN};
      while(count < 1){
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Aperte algum botao...", scale*800.0f, screen_h*400.0f, scale*30, GOLD);
        EndDrawing();

        for(int bt {GAMEPAD_BUTTON_LEFT_FACE_UP}; bt <= GAMEPAD_BUTTON_RIGHT_THUMB; ++bt){
          if(IsGamepadButtonDown(gamepad, bt)){
            comb[count++] = static_cast<GamepadButton>(bt);
            if(count == 2)
              break;
          }
        }
      }
      estado->controles_but[slot_index] = GamepadComb(comb[0], comb[1]);
      botoes_controle[slot_index] = estado->controles_but[slot_index].string();
      tecla_apertada = true;
      pad_consumed = false;
    }

#ifndef UWP_BUILDING
    if(apertado(estado->controles[11]) && !tecla_apertada){
      ToggleFullscreen();
      redimensiona();
      pad_ultimo = false;
    }
    if(gamepad > -1 && estado->controles_but[11].pressionado(gamepad) && !tecla_apertada){
      ToggleFullscreen();
      redimensiona();
      pad_ultimo = true;
    }
#endif

    if((apertado(KEY_ESCAPE) || (gamepad > -1 && IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))) && !tecla_apertada){
      estado->atualiza_controles();
      EndDrawing();
      break;
    }

    for(size_t i {}; i < 8; ++i){
      float wid = (pad_ultimo) ? 270.0f : 100.0f;
      Rectangle r = get_ret(400.0f, 290.0f + 75.0f*i, wid, 50.0f);

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

      if(pad_ultimo && i == static_cast<size_t>(contr_index)){
        DrawRectangleLinesEx(get_ret(395.0f, (285.0f + 75.0f*i), wid + 10.0f, 60.0f), scale*3.0f, GREEN);
      }
      if(GuiButton(r, (pad_ultimo) ? botoes_controle[i].c_str() : botoes[i])){
        key_consumed = true;
        slot_index = i;
      }

      if(gamepad > -1 && static_cast<size_t>(contr_index) == i && IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) && !tecla_apertada){
        pad_consumed = true;
        slot_index = i;
      }
    }

    for(size_t i {8}; i < 14; ++i){
      float wid = (pad_ultimo) ? 270.0f : 100.0f;
      Rectangle r = get_ret(860.0f, 290.0f + 75.0f*(i - 8), wid, 50.0f);
      if(i == 8 || i == 11){
        DrawText(botoes_show[i].c_str(), scale*680.0f, scale*(305.0f + 75.0f*(i - 8)), scale*22, GOLD);
      }
      else if(i == 9){
        DrawText(botoes_show[i].c_str(), scale*763.0f, scale*(305.0f + 75.0f*(i - 8)), scale*22, GOLD);
      }
      else if(i > 11){
        DrawText(botoes_show[i].c_str(), scale*680.0f, scale*(305.0f + 75.0f*(i - 8)), scale*22, GOLD);
      }
      else{
        DrawText(botoes_show[i].c_str(), scale*747.0f, scale*(305.0f + 75.0f*(i - 8)), scale*22, GOLD);
      }

      if(pad_ultimo && i == static_cast<size_t>(contr_index)){
        DrawRectangleLinesEx(get_ret(855.0f, 285.0f + 75.0f*(i - 8), wid + 10.0f, 60.0f), scale*3.0f, GREEN);
      }
      if(GuiButton(r, (pad_ultimo) ? botoes_controle[i].c_str() : botoes[i])){
        key_consumed = true;
        slot_index = i;
      }

      if(gamepad > -1 && static_cast<size_t>(contr_index) == i && IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) && !tecla_apertada){
        pad_consumed = true;
        slot_index = i;
      }
    }
    
    DrawText((pad_ultimo) ? subs[1] : subs[0], scale*300.0f, scale*950.0f, scale*22, GOLD);
    EndDrawing();

    tecla_apertada = false;
    if(axis_timer)
      --axis_timer;
  }
}

void display_saves(Game_State *game, GB_State *estado){
  constexpr float width = 1920.0f;
  constexpr float height = 1080.0f;
  constexpr const char *subs[2] = {"Aperte ESC para voltar", "Aperte B para voltar"};

  float screen_w = get_width()/width;
  float screen_h = get_height()/height;
  float scale = (std::min)(screen_w, screen_h);
  GuiSetStyle(BUTTON, TEXT_SIZE, (scale*25.0f));
  GuiSetStyle(DEFAULT, TEXT_SIZE, (scale*25.0f));

  auto frames = game->load_framebuffer();

  float tex_scale = scale*3.0f;
  float textureX = 160*tex_scale;
  float textureY = 144*tex_scale;
  float posX = scale*700.0f;
  float posY = scale*475.0f;
  int8_t contr_index = estado->save_slot - 1;
  bool botao_apertado {false};
  uint8_t opt {3};
  int axis_timer {};

  auto redimensiona = [&](){
    screen_w = get_width()/width;
    screen_h = get_height()/height;
    scale = (std::min)(screen_w, screen_h);
    GuiSetStyle(BUTTON, TEXT_SIZE, (scale*25.0f));
    GuiSetStyle(DEFAULT, TEXT_SIZE, (scale*25.0f));
      
    tex_scale = scale*3.0f;
    textureX = 160*tex_scale;
    textureY = 144*tex_scale;
    posX = scale*700.0f;
    posY = scale*475.0f;
  };

  bool& pad_ultimo = estado->pad_ultimo;
  auto controle_input = [&](int pad){
    float leftStickX = fix_deadzone(GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_X));
    float leftStickY = fix_deadzone(GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_Y));

    if((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) || leftStickX > 0.5f) && !axis_timer){
        if(contr_index < 10)
          contr_index = 10;
        else{
          contr_index = (contr_index + 1) % 13;
        }

        pad_ultimo = true;
        if(!axis_timer)
          axis_timer = 10;
    }
    else if((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_LEFT) || leftStickX < -0.5f) && !axis_timer){
      if(contr_index < 10){
        contr_index = 12;
      }
      else{
        --contr_index;
        if(contr_index < 10){
          contr_index = 0;
        }
      }
      
      pad_ultimo = true;
      if(!axis_timer)
          axis_timer = 10;
    }
    else if((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_DOWN) || leftStickY > 0.5f) && !axis_timer){
      if(contr_index > 9){
        pad_ultimo = true;
        return;
      }
      contr_index = (contr_index + 1) % 10;
      pad_ultimo = true;
      if(!axis_timer)
          axis_timer = 10;
    }
    else if((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_UP) || leftStickY < -0.5f) && !axis_timer){
      if(contr_index > 9){
        pad_ultimo = true;
        return;
      }
      --contr_index;
      if(contr_index < 0)
        contr_index = 9;

      pad_ultimo = true;
      if(!axis_timer)
          axis_timer = 10;
    }
  };

  while(1){
    BeginDrawing();
    ClearBackground(BLACK);

    if(IsWindowResized()){
      redimensiona();
    }

    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
      pad_ultimo = false;
    }

    int gamepad = GamepadDisponivel();
    if(gamepad > -1){
      controle_input(gamepad);
    }

#ifndef UWP_BUILDING
    if(apertado(estado->controles[11])){
      ToggleFullscreen();
      redimensiona();
      pad_ultimo = false;
    }
    if(gamepad > -1 && estado->controles_but[11].pressionado(gamepad)){
      ToggleFullscreen();
      redimensiona();
      pad_ultimo = true;
    }
#endif

    if(apertado(KEY_ESCAPE) || (gamepad > -1 && IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))){
      std::fstream arquivo(game->save_path, std::ios::in | std::ios::out | std::ios::binary);
      if(arquivo){
        arquivo.seekg(MAX_SAVE_SLOTS*sizeof(size_t), std::ios::beg);
        arquivo.write(reinterpret_cast<char*>(&estado->save_slot), sizeof(size_t));
      }

      EndDrawing();
      break;
    }

    if(botao_apertado && IsGamepadButtonReleased(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)){
      switch(opt){
        case 0:{
          game->save_state(estado->save_slot);
          if(frames[estado->save_slot - 1])
            UnloadTexture(*frames[estado->save_slot - 1]);
          frames[estado->save_slot - 1] = game->load_image(estado->save_slot);
          break;
        }
        case 1:{
          game->load_state(estado->save_slot);
          break;
        }
        case 2:{
          game->delete_state(estado->save_slot);
          if(frames[estado->save_slot - 1]){
            UnloadTexture(*frames[estado->save_slot - 1]);
            frames[estado->save_slot - 1].reset();
          }
          break;
        }
        default: break;
      }

      opt = 3;
      botao_apertado = false;
    }

    DrawText("SAVE STATES", scale*410.0f, scale*80.0f, scale*150.0f, GOLD);
    DrawLine(scale*275.0f, scale*250.0f, scale*1625.0f, scale*250.0f, GOLD);

    for(size_t i {}; i < 10; ++i){
      Rectangle r = get_ret(450.0f, (320.0f + 60.0f*i), 100.0f, 50.0f);
      if(GuiButton(r, ((game->save_states[i]) ? (std::string("Save") + std::to_string(i + 1)).c_str() : "Vazio"))){
        estado->save_slot = i + 1;
      }
      bool pad_apertado = (gamepad > -1 && IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) && 
          static_cast<size_t>(contr_index) == i);
      if(pad_apertado){
        estado->save_slot = i + 1;
      }

      if(estado->save_slot == i + 1){
        DrawRectangleLinesEx(get_ret(445.0f, 315.0f + 60.0f*i, 110.0f, 60.0f), scale*3.0f, BLUE);
        if(frames[i]){
          DrawTextureEx(*frames[i], Vector2{posX, posY}, 0, tex_scale, WHITE);
        }
      }
      if(pad_ultimo && static_cast<size_t>(contr_index) == i){
        DrawRectangleLinesEx(get_ret(445.0f, 315.0f + 60.0f*i, 110.0f, 60.0f), scale*3.0f, GREEN);
      }
    }

    for(size_t i {}; i < 3; ++i){
      Rectangle r = get_ret(700.0f + 200.0f*i, 320.0f, 150.0f, 100.0f);
      bool pad_apertado = (gamepad > -1 && IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) && 
          (contr_index - 10 == static_cast<int>(i)));

      if(pad_ultimo && contr_index - 10 == static_cast<int>(i)){
        DrawRectangleLinesEx(get_ret(695.0f + 200.0f*i, 315.0f, 160.0f, 110.0f), scale*3.0f, GREEN);
      }
      if(pad_apertado){
        pad_ultimo = true;
        botao_apertado = true;
        opt = i;
      }

      if(!i){
        if(GuiButton(r, "Save")){
          game->save_state(estado->save_slot);
          if(frames[estado->save_slot - 1])
            UnloadTexture(*frames[estado->save_slot - 1]);

          frames[estado->save_slot - 1] = game->load_image(estado->save_slot);
          pad_ultimo = false;
        }
      }
      else if(i == 1){
        if(GuiButton(r, "Load")){
          game->load_state(estado->save_slot);
          pad_ultimo = false;
        }
      }
      else{
        if(GuiButton(r, "Delete")){
          game->delete_state(estado->save_slot);
          if(frames[estado->save_slot - 1]){
            UnloadTexture(*frames[estado->save_slot - 1]);
            frames[estado->save_slot - 1].reset();
          }
          pad_ultimo = false;
        }
      }
    }

    DrawText((pad_ultimo) ? subs[1] : subs[0], scale*300.0f, scale*970.0f, scale*22, GOLD);
    if(axis_timer)
      --axis_timer;

    EndDrawing();
  }
  
  for(size_t i {}; i < MAX_SAVE_SLOTS; ++i){
    if(frames[i])
      UnloadTexture(*frames[i]);
  }
}

bool pausa_jogo(Game_State *game, GB_State *estado, bool& pausado, bool& resumido){
  if(!pausado) return false; 

  BeginDrawing();
  ShowCursor();
  EndDrawing();
  
  constexpr char opcoes[4][15] = {"Resumir", "Controles", "Save States", "Sair"};
  uint8_t escolhas {};
  int8_t contr_index {};
  int axis_timer {};

  constexpr float width = 1920.0f;
  constexpr float height = 1080.0f;

  float screen_w = get_width()/width;
  float screen_h = get_height()/height;
  float scale = (std::min)(screen_w, screen_h);
  GuiSetStyle(BUTTON, TEXT_SIZE, (scale*25.0f));
  GuiSetStyle(DEFAULT, TEXT_SIZE, (scale*25.0f));
  SetTargetFPS(60);

  auto redimensiona = [&](){
    screen_w = get_width()/width;
    screen_h = get_height()/height;
    scale = (std::min)(screen_w, screen_h);
    GuiSetStyle(BUTTON, TEXT_SIZE, scale*25.0f);
    GuiSetStyle(DEFAULT, TEXT_SIZE, scale*25.0f);
  };

  bool& pad_ultimo = estado->pad_ultimo;
  auto controle_input = [&](int pad){
    float leftStickY = fix_deadzone(GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_Y));

    if((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_DOWN) || leftStickY > 0.5f) && !axis_timer){
      contr_index = (contr_index + 1) % std::size(opcoes);
      pad_ultimo = true;
      if(!axis_timer)
        axis_timer = 10;
    }
    else if((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_UP) || leftStickY < -0.5f) && !axis_timer){
      --contr_index;
      if(contr_index < 0)
        contr_index = std::size(opcoes) - 1;
            
      pad_ultimo = true;
      if(!axis_timer)
        axis_timer = 10;
    }
  };

  while(1){
    BeginDrawing();
    ClearBackground(BLACK);

    if(IsWindowResized()){
      redimensiona();
    }

    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
      pad_ultimo = false;
    }
    
    int gamepad = GamepadDisponivel();
    if(gamepad > -1){
      controle_input(gamepad);
    }

#ifndef UWP_BUILDING
    if(apertado(estado->controles[11])){
      ToggleFullscreen();
      redimensiona();
      pad_ultimo = false;
    }
    if(gamepad > -1 && estado->controles_but[11].pressionado(gamepad)){
      ToggleFullscreen();
      redimensiona();
      pad_ultimo = true;
    }
#endif

    bool gamepad_apertado = (gamepad > -1 && 
        (IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) || estado->controles_but[9].pressionado(gamepad)));
    if(apertado(KEY_ESCAPE) || apertado(estado->controles[9]) || gamepad_apertado){
      resumido = true;
      EndDrawing();
      break;
    }
      
    DrawText("PAUSE", scale*690.0f, scale*80.0f, scale*150.0f, GOLD);
    DrawLine(scale*275.0f, scale*250.0f, scale*1625.0f, scale*250.0f, GOLD);

    for(size_t i {}; i < 4; ++i){
      Rectangle r = get_ret(790.0f, (320.0f + 135.0f*i), 300.0f, 100.0f);

      if(pad_ultimo && static_cast<size_t>(contr_index) == i){
        DrawRectangleLinesEx(get_ret(785.0f, 315.0f + 135.0f*i, 310.0f, 110.0f), scale*3.0f, GREEN);
        if(IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
          escolhas |= opt_escolha(i);
      }
      if(GuiButton(r, opcoes[i])){
        escolhas |= opt_escolha(i);
        pad_ultimo = false;
      }
    }

    if(escolhas & opt_escolha(0)){
      escolhas &= ~opt_escolha(0);
      resumido = true;
      EndDrawing();
      break;
    }
    if(escolhas & opt_escolha(1)){
      escolhas &= ~opt_escolha(1);
      display_controles(estado);
      redimensiona();
    }
    if(escolhas & opt_escolha(2)){
      escolhas &= ~opt_escolha(2);
      display_saves(game, estado);
      redimensiona();
    }
    if(escolhas & opt_escolha(3)){
      escolhas &= ~opt_escolha(3);
      EndDrawing();
      return true;
    }

    if(axis_timer)
      --axis_timer;

    EndDrawing();
  }

  pausado = false;
  SetTargetFPS(0);
  return false;
}

void carrega_rom(GB_State *estado){
  constexpr const char *extensoes[] = {"*.gb", "*.gbc"};

#ifdef UWP_BUILDING
  std::string resultado = std::async(std::launch::async, [extensoes]() -> std::string{
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    winrt::Windows::Storage::Pickers::FileOpenPicker picker;
    picker.SuggestedStartLocation(winrt::Windows::Storage::Pickers::PickerLocationId::ComputerFolder);

    for(const auto& ext : extensoes) {
      picker.FileTypeFilter().Append(winrt::to_hstring(ext));
    }

    winrt::Windows::Storage::StorageFile arquivo = picker.PickSingleFileAsync().get();
    if(arquivo){
      return winrt::to_string(arquivo.Path());
    }
    return "";
   }).get();

  if(!resultado.length()) return;
  inicia_emulador(resultado.c_str(), estado);
#else
  const char *resultado = tinyfd_openFileDialog(
    "Escolha a rom",  // título
    "",               // pasta inicial
    2,                // número de filtros
    extensoes,        // extensões permitidas
    "",               // descrição
    0);               // 0 = um arquivo;

  if(!resultado) return;
  inicia_emulador(resultado, estado);
#endif
}

void define_pasta(GB_State *estado, std::string_view pasta, ListaArquivos *lista){
#ifdef UWP_BUILDING
  std::string resultado = std::async(std::launch::async, []() -> std::string{
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    winrt::Windows::Storage::Pickers::FolderPicker picker;
    picker.SuggestedStartLocation(winrt::Windows::Storage::Pickers::PickerLocationId::ComputerFolder);
    picker.FileTypeFilter().Append(L"*");

    winrt::Windows::Storage::StorageFolder pasta = picker.PickSingleFolderAsync().get();
    if(pasta){
      return winrt::to_string(pasta.Path());
    }
    return ""; }).get();

  if(!resultado.length()) return;
#else
  const char *resultado = tinyfd_selectFolderDialog("Selecione uma pasta", "");
  if(!resultado) return;
#endif

  std::filesystem::path state_path = estado->main_dir / "state.cfg";

  std::fstream arquivo(state_path, std::ios::in | std::ios::out);
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

void toggle_paleta(GB_State *estado){
  estado->paleta_cgb ^= 1;
  std::filesystem::path state_path = estado->main_dir / "state.cfg";
  std::fstream arquivo(state_path, std::ios::in | std::ios::out);
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
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);

  constexpr const char *opcoes[] = {
    "Abrir ROM", "Controles", "Definir pasta de saves", "Definir pasta de roms", "Sair",
  };

  uint8_t escolhas {};
  bool paleta_delay {false};
  uint8_t open_delay {};

  GB_State estado;
  ListaArquivos lista(&estado);
    
  int scroll_index {}, ativo {-1};
  int contr_index {};
  int axis_timer {};
  estado.pad_ultimo = (GamepadDisponivel() > -1);
  bool& pad_ultimo = estado.pad_ultimo;
  
  GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(BLACK));
  GuiSetStyle(LISTVIEW, TEXT_COLOR_NORMAL,  ColorToInt(BLACK));
  GuiSetStyle(LISTVIEW, TEXT_COLOR_FOCUSED,  ColorToInt(BLACK));
  GuiSetStyle(LISTVIEW, BASE_COLOR_FOCUSED, ColorToInt(GOLD));
  GuiSetStyle(LISTVIEW, BORDER_COLOR_FOCUSED, ColorToInt(GOLD));

#ifdef UWP_BUILDING
  auto base = winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread().Bounds();
  float screen_w = base.Width/width;
  float screen_h = base.Height/height;
  SetWindowSize((int)base.Width, (int)base.Height);
#else
  float screen_w = GetScreenWidth()/width;
  float screen_h = GetScreenHeight()/height;
#endif

  float scale = (std::min)(screen_w, screen_h);
  GuiSetStyle(BUTTON, TEXT_SIZE, scale*25.0f);
  GuiSetStyle(DEFAULT, TEXT_SIZE, scale*25.0f);
  GuiSetStyle(LISTVIEW, BORDER_WIDTH, scale*5.0f);
  GuiSetStyle(LISTVIEW, SCROLLBAR_WIDTH, scale*8.0f);
  GuiSetStyle(LISTVIEW, LIST_ITEMS_SPACING, scale*1.0f);
  GuiSetStyle(LISTVIEW, LIST_ITEMS_HEIGHT, scale*28.0f);

  auto redimensiona = [&](){
    screen_w = get_width()/width;
    screen_h = get_height()/height;
    scale = (std::min)(screen_w, screen_h);
    GuiSetStyle(BUTTON, TEXT_SIZE, (scale*25.0f));
    GuiSetStyle(DEFAULT, TEXT_SIZE, (scale*25.0f));
    GuiSetStyle(LISTVIEW, BORDER_WIDTH, scale*5.0f);
    GuiSetStyle(LISTVIEW, SCROLLBAR_WIDTH, scale*8.0f);
    GuiSetStyle(LISTVIEW, LIST_ITEMS_SPACING, scale*1.0f);
    GuiSetStyle(LISTVIEW, LIST_ITEMS_HEIGHT, scale*28.0f);
  };

  bool in_list {false};
  auto controle_input = [&](int pad){
    float leftStickY = fix_deadzone(GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_Y));
    float leftStickX = fix_deadzone(GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_X));

    if((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_DOWN) || leftStickY > 0.5f) && !axis_timer){
      contr_index = (contr_index + 1) % ((in_list) ? lista.paths.size() : std::size(opcoes) + 1);
      pad_ultimo = true;
      if(!axis_timer)
        axis_timer = 10;
    }
    else if((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_UP) || leftStickY < -0.5f) && !axis_timer){
      --contr_index;
      if(contr_index < 0)
        contr_index = (in_list) ? lista.paths.size() - 1 : std::size(opcoes);
            
      pad_ultimo = true;
      if(!axis_timer)
        axis_timer = 10;
    }
    else if((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_LEFT) || leftStickX < -0.5f) && !axis_timer){
      if(!in_list && contr_index == std::size(opcoes)){
        --contr_index;
      }
      else{
        if(lista.paths.size()){
          contr_index = 0;
          in_list ^= 1;
        }
        else{
          contr_index = std::size(opcoes);
        }
      }

      pad_ultimo = true;
      if(!axis_timer)
        axis_timer = 10;
    }
    else if((IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) || leftStickX > 0.5f) && !axis_timer){
      if(lista.paths.size()){
        contr_index = 0;
        in_list ^= 1;
      }
      else{
        contr_index = std::size(opcoes);
      }
      pad_ultimo = true;
      if(!axis_timer)
        axis_timer = 10;
    }
  };

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);
    
    if(IsWindowResized()){
      redimensiona();
    }

    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
      pad_ultimo = false;
    }

    int gamepad = GamepadDisponivel();
    if(gamepad > -1){
      controle_input(gamepad);
    }

#ifndef UWP_BUILDING
    if(apertado(estado.controles[11])){
      ToggleFullscreen();
      redimensiona();
      pad_ultimo = false;
    }
    if(gamepad > -1 && estado.controles_but[11].pressionado(gamepad)){
      ToggleFullscreen();
      redimensiona();
      pad_ultimo = true;
    }
#endif

    if(gamepad > -1 && (open_delay == 1) && IsGamepadButtonReleased(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)){
      open_delay = 2;
    }

    DrawText("CARLINHOS BOY", scale*325.0f, scale*80.0f, scale*150.0f, GOLD);
    DrawLine(scale*275.0f, scale*250.0f, scale*1625.0f, scale*250.0f, GOLD);

    for(size_t i {}; i < 5; ++i){
      Rectangle r = get_ret(550.0f, (320.0f + 135.0f*i), 300.0f, 100.0f);
      if(GuiButton(r, opcoes[i])){
        escolhas |= opt_escolha(i);
        pad_ultimo = false;
      }
      if(pad_ultimo && !in_list && static_cast<size_t>(contr_index) == i){
        DrawRectangleLinesEx(get_ret(545.0f, 315.0f + 135.0f*i, 310.0f, 110.0f), scale*3.0f,GREEN);
        if(IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
          escolhas |= opt_escolha(i);
      }
    }
   
    GuiListView(get_ret(1000.0f, 320.0f, 325.0f, 640.0f), lista.geral.c_str(), &scroll_index, &ativo);
    if(gamepad > -1 && in_list){
      float offset = GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING) + GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
      DrawRectangleLinesEx(Rectangle{scale*1003.0f, scale*323.0f + contr_index*offset, scale*317.0f, offset - scale*3.0f}, scale*3.0f, GREEN);
      if(IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)){
        ativo = contr_index;
        open_delay = 1;
      }
    }

    DrawText("Modo CGB: ", scale*1010.0f, scale*980.0f, scale*25.0f, GOLD);
    if(gamepad > -1 && !in_list && contr_index == 5){
      DrawRectangleLines(scale*1145.0f, scale*965.0f, scale*110.0f, scale*50.0f, GREEN);
      if(IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
        paleta_delay = true;
    }

    bool paleta_trocada = (paleta_delay && IsGamepadButtonReleased(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));
    if(GuiButton(get_ret(1150.0f, 970.0f, 100.0f, 40.0f), (estado.paleta_cgb) ? "ON" : "OFF") || paleta_trocada){
      paleta_delay = false;
      toggle_paleta(&estado);
    }

    if((ativo >= 0 && ativo < static_cast<int>(lista.arquivos1.count + lista.arquivos2.count) && !open_delay) || open_delay == 2){
      inicia_emulador(lista.paths[ativo], &estado);
      ativo = -1;
      open_delay = 0;
      redimensiona();
    }
    
    if(escolhas & opt_escolha(0)){
      escolhas &= ~opt_escolha(0);
      carrega_rom(&estado);
      redimensiona();
    }
    if(escolhas & opt_escolha(1)){
      escolhas &= ~opt_escolha(1);
      display_controles(&estado);
      redimensiona();
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
      EndDrawing();
      break;
    }
    if(axis_timer)
      --axis_timer;

    EndDrawing();
  }

  CloseWindow();
}


}

