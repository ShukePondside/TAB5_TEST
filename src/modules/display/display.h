#pragma once
#include <M5Unified.h>
#include <LittleFS.h>

namespace Display {

using Image = std::pair<uint8_t*, size_t>;
using Images = std::vector<Image>;

void Initialize();
SemaphoreHandle_t GetDisplayMutex();
bool ReadImageFile(const char* file_path, Images* images);
void Draw(uint8_t index, uint8_t x, uint8_t y);

}  // namespace Display
