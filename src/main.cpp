#ifdef UWP_BUILDING

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "uwp_init.h"

static UWP_State* global_state = nullptr;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]){
   SetEnvironmentVariableW(L"GALLIUM_DRIVER", L"d3d12");
   global_state = new UWP_State;
   *appstate = reinterpret_cast<void*>(global_state);
   GB_UWP::init_uwp(global_state);
   return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate){
  GB_UWP::run_gui(global_state);
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event){
  PollSDLEvents(event);
  if(event->type == SDL_EVENT_QUIT || global_state->quitted){
    return SDL_APP_SUCCESS;
  }
  
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result){
  if(appstate){
    delete global_state;
  }
  CloseWindow();
}

#else
#include "init.h"

int main(void){
  GB::init_gui();
  return 0;
}

#endif
