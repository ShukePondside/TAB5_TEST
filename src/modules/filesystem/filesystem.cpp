#include "filesystem.h"

std::vector<String> GetFileList(const String& directry_path, const std::vector<String>& extensions) {
  std::vector<String> fileList;
  File dir = LittleFS.open(directry_path);
  if (!dir || !dir.isDirectory()) {
    log_e("Failed to open directory:%s", directry_path.c_str());
    return fileList;
  }
  File file = dir.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String name = String(file.name());
      name.toLowerCase();  // 拡張子チェックを確実にするため小文字に統一
      for (const auto& ext : extensions) {
        if (name.endsWith(ext)) {
          fileList.push_back(name);
          //   fileList.push_back(directry_path + "/" + name);
          break;  // 最初に一致したら追加して終了
        }
      }
    }
    file = dir.openNextFile();
  }
  return fileList;
}