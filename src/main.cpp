#ifdef UWP_BUILDING

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "uwp_init.h"

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static UWP_State* global_state = nullptr;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]){
   SetEnvironmentVariableW(L"GALLIUM_DRIVER", L"d3d12");
   GB::init_uwp();
   return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate){
   appstate->funcao_atual();
   return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event){
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result){
    if(appstate){
      global_state->clean();
      CloseWindow();
    }
}

#else
#include "init.h"

int main(void){
  GB::init_gui();
  return 0;
}
#endif
