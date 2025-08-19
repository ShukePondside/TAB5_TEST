#pragma once

#include <LittleFS.h>
#include <vector>

std::vector<String> GetFileList(const String& directry_path, const std::vector<String>& extensions);
