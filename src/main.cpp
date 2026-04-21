#include "init.h"

int main(int argc, char **argv){
  
  if(argc < 2){
    std::cout << "Especifique uma rom.\n";
    return 1;
  }

  GB::CPU cpu;
  GB::init_game(&cpu, argv);

  return 0;
}
