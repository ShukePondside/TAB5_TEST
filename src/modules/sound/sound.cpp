#include "sound.h"

namespace Sound {
Params sound_params;
SemaphoreHandle_t params_mutex = nullptr;

void Initialize() {
  params_mutex = xSemaphoreCreateMutex();

  Params cfg;

  // サウンド初期設定/////////
  cfg.sound_flg = false;
  cfg.volume = 0.2;
  cfg.loop_enable = false;
  cfg.loop_interval = 0;
  cfg.is_sd_card = false;
  cfg.current_index = 0;
  ///////////////////////////

  // サウンドファイル読み込み///////////////////////
  std::vector<String> exts = {".mp3", ".wav"};
  String directory = "/sound";
  cfg.file_name = GetFileList(directory, exts);
  ////////////////////////////////////////////////

  SetParams(&cfg);  // サウンド設定更新

  // 正常にファイルが読み込まれたかの確認用/////////////////////
  Params check_sound = GetParams();
  log_d("SoundFile qty:%d", check_sound.file_name.size());
  for (const auto& filename : check_sound.file_name) {
    log_d("%s", filename.c_str());
  }
  ///////////////////////////////////////////////////////////

  // バックグラウンドでタスク実行
  xTaskCreatePinnedToCore(Sound::tPlay, "", 4096, NULL, 1, NULL, APP_CPU_NUM);
}

Params GetParams() {
  xSemaphoreTake(params_mutex, portMAX_DELAY);
  Params val = sound_params;
  xSemaphoreGive(params_mutex);
  return val;
}

void SetParams(const Params* _sound_params) {
  xSemaphoreTake(params_mutex, portMAX_DELAY);
  sound_params = *_sound_params;
  xSemaphoreGive(params_mutex);
}

void tPlay(void* pvParameters) {
  Params cfg;

  while (1) {
    cfg = GetParams();
    if (!cfg.sound_flg) {
      delay(1);
      continue;
    }
    log_d("volume:%f", cfg.volume);
    log_d("file_name:%s", cfg.file_name[cfg.current_index].c_str());
    AudioFileSource* file;
    String file_fullpath = "/sound/" + cfg.file_name[cfg.current_index];
    if (!cfg.is_sd_card) {
      file = new AudioFileSourceLittleFS(file_fullpath.c_str());
    } else {
      file = new AudioFileSourceSD(file_fullpath.c_str());
    }

    AudioOutputI2S* out;

#if defined(ARDUINO_M5STACK_FIRE)  // M5StackFIRE
    out = new AudioOutputI2S(I2S_NUM_0, AudioOutputI2S::INTERNAL_DAC);
#endif
#if defined(ARDUINO_M5Stack_ATOM)  // Atom
    out = new AudioOutputI2S(I2S_NUM_0);
    out->SetPinout(CONFIG_I2S_BCK_PIN, CONFIG_I2S_LRCK_PIN, CONFIG_I2S_DATA_PIN);
#endif
#if defined(ARDUINO_M5Stack_ATOMS3)  // AtomS3
#endif
#if defined(ARDUINO_M5Stick_C)  // M5StickC
#endif

    // Common
    out->SetOutputModeMono(true);
    out->SetGain(cfg.volume);  // MAX:3.999

    AudioGenerator* sound = nullptr;
    if (cfg.file_name[cfg.current_index].endsWith(".mp3")) {
      sound = new AudioGeneratorMP3();
    } else if (cfg.file_name[cfg.current_index].endsWith(".wav")) {
      sound = new AudioGeneratorWAV();
    }
    if (sound != nullptr) {
      sound->begin(file, out);
      while (sound->isRunning()) {
        cfg = GetParams();  // sound_flgの状態チェック
        if (!sound->loop() || !cfg.sound_flg) {
          sound->stop();
          if (!cfg.loop_enable) {
            cfg.sound_flg = false;
            SetParams(&cfg);
          } else {
            delay(cfg.loop_interval);
          }
        }
        delay(1);
      }
      delete sound;
    }

    delete out;
    delete file;
  }
}

}  // namespace Sound