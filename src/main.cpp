#include "init.h"

int main(void){
  #ifdef UWP_BUILDING
	SetEnvironmentVariableW(L"GALLIUM_DRIVER", L"d3d12");
  #endif

  GB::init_gui();
  return 0;
}
