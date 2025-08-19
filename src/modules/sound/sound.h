#pragma once
#include <Arduino.h>
#include <vector>

#include <driver/i2s.h>
#include "AudioGeneratorWav.h"
#include "AudioGeneratorMP3.h"
#include "AudioFileSourceID3.h"
#include "AudioFileSourceLittleFS.h"
#include "AudioFileSourceSD.h"
#include "AudioOutputI2S.h"
#include "AudioOutputI2SNoDAC.h"

#include "../filesystem/filesystem.h"

namespace Sound {
struct Params {
  bool sound_flg;
  float volume;
  bool loop_enable;
  uint16_t loop_interval;
  bool is_sd_card;
  uint8_t current_index;
  std::vector<String> file_name;
};

void Initialize();
Params GetParams();
void SetParams(const Params* _sound_params);
void tPlay(void* pvParameters);
}  // namespace Sound