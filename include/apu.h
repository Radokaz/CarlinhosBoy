#ifndef APU_H
#define APU_H

#define WAVE_RAM_INICIO 0xFF30
#define WAVE_RAM_FIM   0xFF40

#define AUDIO_INICIO 0xFF10
#define AUDIO_FIM 0xFF30

//bits do registrador NR52 (0xFF26)
#define APU_AUDIO_ON (1 << 7)
#define APU_CH4_ON (1 << 3)
#define APU_CH3_ON (1 << 2)
#define APU_CH2_ON (1 << 1)
#define APU_CH1_ON (1 << 0)

#define APU_CH1_LEFT (1 << 4)
#define APU_CH2_LEFT (1 << 5)
#define APU_CH3_LEFT (1 << 6)
#define APU_CH4_LEFT (1 << 7)
#define APU_CH1_RIGHT (1 << 0)
#define APU_CH2_RIGHT (1 << 1)
#define APU_CH3_RIGHT (1 << 2)
#define APU_CH4_RIGHT (1 << 3)

#define APU_CANAL1 (1 << 0)
#define APU_CANAL2 (1 << 1)
#define APU_CANAL3 (1 << 2)
#define APU_CANAL4 (1 << 3)

#define FREQUENCIA_OSCILADOR 4194304

#include <cstdint>
#include <cmath>
#include <array>
#include <raylib.h>

namespace GB{

inline bool is_audio_on(uint8_t *memoria){ return static_cast<bool>((memoria[0xFF26] & APU_AUDIO_ON) != 0); }
inline bool is_channel1_on(uint8_t *memoria){ return static_cast<bool>((memoria[0xFF26] & APU_CH1_ON) != 0); }
inline bool is_channel2_on(uint8_t *memoria){ return static_cast<bool>((memoria[0xFF26] & APU_CH2_ON) != 0); }
inline bool is_channel3_on(uint8_t *memoria){ return static_cast<bool>((memoria[0xFF26] & APU_CH3_ON) != 0); }
inline bool is_channel4_on(uint8_t *memoria){ return static_cast<bool>((memoria[0xFF26] & APU_CH4_ON) != 0); }

inline bool is_ch1_left(uint8_t *memoria) { return static_cast<bool>((memoria[0xFF25] & APU_CH1_LEFT) != 0); }
inline bool is_ch1_right(uint8_t *memoria) { return static_cast<bool>((memoria[0xFF25] & APU_CH1_RIGHT) != 0); }
inline bool is_ch2_left(uint8_t *memoria) { return static_cast<bool>((memoria[0xFF25] & APU_CH2_LEFT) != 0); }
inline bool is_ch2_right(uint8_t *memoria) { return static_cast<bool>((memoria[0xFF25] & APU_CH2_RIGHT) != 0); }
inline bool is_ch3_left(uint8_t *memoria) { return static_cast<bool>((memoria[0xFF25] & APU_CH3_LEFT) != 0); }
inline bool is_ch3_right(uint8_t *memoria) { return static_cast<bool>((memoria[0xFF25] & APU_CH3_RIGHT) != 0); }
inline bool is_ch4_left(uint8_t *memoria) { return static_cast<bool>((memoria[0xFF25] & APU_CH4_LEFT) != 0); }
inline bool is_ch4_right(uint8_t *memoria) { return static_cast<bool>((memoria[0xFF25] & APU_CH4_RIGHT) != 0); }

static constexpr std::array<std::array<uint8_t, 8>, 4> tabela_onda{{
    {1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 0}, 
    {1, 1, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1}
  }}; // 0 - baixo, 1 - alto

struct CH1{

    uint8_t *memoria;
    
    uint16_t periodo_divider {};
    uint16_t periodo_shadow {};
    uint16_t length_timer {};

    bool dac {false};

    bool sweep_enabled {false};
    bool negate_mode {false};
    uint8_t periodo_pace {};
    uint8_t ind_step {};
    uint8_t direcao_periodo {}; //0 - direita (adição), 1 - esquerda(subtração)

    uint8_t duty_step {};

    uint8_t envelope {};
    uint8_t initial_volume {};
    uint8_t envelope_pace {};
    uint8_t direcao_envelope {}; //0 - reduz volume, 1 - aumenta volume
    
    uint8_t envelope_count {};
    uint8_t periodo_count {};

    CH1(uint8_t *mem): memoria{mem} {};

    bool is_length_enabled();
    void init_ch1(void);
    void seta_length(void);
    void seta_envelope(void);
    void sweep_periodo(void);
    void sweep_envelope(void);
    void incrementa_divider(void);
    void sweep_length(void);
    uint8_t get_sample(void);
    void clear(void);
};

struct CH2{

    uint8_t *memoria;
    
    bool dac {false};

    uint16_t periodo_divider {};
    uint16_t periodo_shadow {};
    uint16_t length_timer {};

    uint8_t duty_step {};

    uint8_t envelope {};
    uint8_t initial_volume {};
    uint8_t envelope_pace {};
    uint8_t direcao_envelope {}; //0 - reduz volume, 1 - aumenta volume
    
    uint8_t envelope_count {};

    CH2(uint8_t *mem): memoria{mem} {};

    bool is_length_enabled(void);
    void init_ch2(void);
    void seta_length(void);
    void seta_envelope(void);
    void sweep_envelope(void);
    void incrementa_divider(void);
    void sweep_length(void);
    uint8_t get_sample(void);
    void clear(void);
};

struct CH3{

  uint8_t *memoria;

  bool dac {false};

  uint16_t periodo_divider {};
  uint16_t periodo_shadow {};

  uint16_t length_timer {};
  uint8_t output_level {}; //0-3
  uint8_t wram_index {}; //0-31
  uint8_t last_sample {};
  uint8_t last_byte {};
  uint8_t trigger_delay {};

  CH3(uint8_t *mem): memoria{mem} {}

  bool is_length_enabled(void);
  void init_ch3(void);
  void seta_output(void);
  void seta_length(void);
  void sweep_length(void);
  void incrementa_divider(void);
  void read_waveram(void);
  uint8_t get_sample(void);
  void clear(void);
};

struct CH4{

  uint8_t *memoria;

  bool dac {false};

  uint32_t period {8};
  uint32_t clock_timer {};
  uint16_t length_timer {};

  uint16_t lfsr {};
  uint8_t clock_shifter {};
  uint8_t lfsr_width {};
  uint8_t clock_divider {};
  uint8_t ultimo_bit {};

  uint8_t envelope {};
  uint8_t initial_volume {};
  uint8_t envelope_pace {};
  uint8_t direcao_envelope {};
  uint8_t envelope_count {};

  CH4(uint8_t *mem): memoria{mem} {}

  bool is_length_enabled(void);
  void init_ch4(void);
  void seta_length(void);
  void seta_clock(void);
  void seta_envelope(void);
  void sweep_length(void);
  void sweep_clock(void);
  void incrementa_clock(void);
  void sweep_envelope(void);
  uint8_t get_sample(void);
  void clear(void);
};

struct APU{
  
  uint8_t *memoria {};

  CH1 ch1;
  CH2 ch2;
  CH3 ch3;
  CH4 ch4;

  uint64_t sample_ciclos {};
  uint32_t sample_accumulator {};
  float capacitor_esq {};
  float capacitor_dir {};
  float sample_esq {};
  float sample_dir {};

  uint16_t volume_dir {};
  uint16_t volume_esq {};

  uint8_t div_apu {}; //sincroniza os parâmetros de onda em todos os canais
  uint8_t div_prev {};
  uint8_t apu_hack {};
  uint8_t canais_ativos {0x0F};

  uint8_t ch1_prev {};
  uint8_t ch2_prev {};
  uint8_t ch3_prev {};
  uint8_t ch4_prev {};
  
  APU(uint8_t *mem): memoria{mem}, ch1{mem}, ch2{mem}, ch3{mem}, ch4{mem} {}

  void atualiza_volume(void);
  void limpa_registradores(void);
  void power_on(void);

  uint8_t& read(uint16_t endereco);
  void write(uint16_t endereco, uint8_t valor);
  
  void mixer(uint8_t atual, uint8_t& ultimo, bool esq, bool dir);
  void amplifier(void);
  void output(void);

  void frame_sequencer(void);
  void step(void);
};

void audio_callback(void* buffer, unsigned int frames);

}


#endif
