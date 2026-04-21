#include "init.h"

int main(int argc, char **argv){
  
  if(argc < 2){
    std::cout << "Especifique uma rom.\n";
    return 1;
  }

  GB::Timer timer;
  GB::CPU cpu(&timer.div_count);

  GB::init_game(&cpu, timer, argv);

  return 0;
}
