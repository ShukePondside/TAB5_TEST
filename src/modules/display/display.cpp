#include "display.h"

namespace Display {

SemaphoreHandle_t display_mutex;  // ディスプレイ描画管理
Images images;                    // 画像管理

void Draw(uint8_t index, uint8_t x, uint8_t y) {
  xSemaphoreTake(display_mutex, portMAX_DELAY);
  M5.Display.drawPng(images.at(index).first, images.at(index).second, x, y);
  xSemaphoreGive(display_mutex);
}

void Initialize() {
  display_mutex = xSemaphoreCreateMutex();

  if (!ReadImageFile("/image/test.png", &images)) {
    Serial.println("画像の読み込みに失敗しました");
  } else {
    Serial.println("画像の読み込みに成功しました");
  }
}

SemaphoreHandle_t GetDisplayMutex() {
  return display_mutex;
}

bool ReadImageFile(const char* file_path, Images* images) {
  auto file = LittleFS.open(file_path, "r");
  if (!file) {
    log_e("%s:ファイルを開けません", file_path);
    return false;
  }

  // ファイルサイズを取得
  size_t image_size = file.size();
  if (image_size == 0) {
    log_e("%s:ファイルサイズが0です", file_path);
    return false;
  }

  // メモリを確保
  uint8_t* image_buf = new uint8_t[image_size];
  if (!image_buf) {
    log_e("%s:メモリ確保に失敗しました", file_path);
    return false;
  }

  // ファイルを読み込み
  file.read(image_buf, image_size);
  file.close();

  images->push_back(std::make_pair(image_buf, image_size));

  return true;
}

}  // namespace Display