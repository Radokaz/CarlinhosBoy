#ifndef JOYPAD_H
#define JOYPAD_H

#define START (1 << 7)
#define SELECT (1 << 6)
#define B_BUTTON (1 << 5)
#define A_BUTTON (1 << 4)
#define DOWN_BUTTON (1 << 3)
#define UP_BUTTON (1 << 2)
#define LEFT_BUTTON (1 << 1)
#define RIGHT_BUTTON (1 << 0)

#define segurado(x) IsKeyDown(x)
#define apertado(x) IsKeyPressed(x)

namespace GB{

  struct Joypad{
    const void *teclas {};
    uint8_t output {0xFF};
    uint8_t controles_prev {};
    bool select_dir {};
    bool select_but {};
    uint8_t *p1 {};
    uint8_t controles {};

    Joypad(const void *tec): teclas{tec} {}

    uint8_t& get_output(void){
      this->set_select();
      uint8_t result {0x3F};

      if(select_dir){
        result &= ~(1 << 4);
        if(controles & RIGHT_BUTTON)
          result &= ~(1 << 0);
        if(controles & LEFT_BUTTON)
          result &= ~(1 << 1);
        if(controles & UP_BUTTON)
          result &= ~(1 << 2);
        if(controles & DOWN_BUTTON)
          result &= ~(1 << 3);
      }

      if(select_but){
        result &= ~(1 << 5);
        if(controles & A_BUTTON)
          result &= ~(1 << 0);
        if(controles & B_BUTTON)
          result &= ~(1 << 1);
        if(controles & SELECT)
          result &= ~(1 << 2);
        if(controles & START)
          result &= ~(1 << 3);
      }

      output = result | 0b11000000;
      return output;
    }

    void input(uint8_t in){
      controles_prev = controles;
      controles = in;
    }

    uint8_t get_select(){
      return (*p1 & 0x30);
    }

    void set_select(void){
      select_dir = (this->get_select() & 0x10) ? false : true;
      select_but = (this->get_select() & 0x20) ? false : true;
    }

  };

  void le_input(Joypad& pad, bool& paleta_lcd, uint8_t& canais_ativos, bool& pausado);

}


#endif
