#include <Arduino.h>
#include <M5Unified.h>

#include "FS.h"
#include "SPIFFS.h"

const uint32_t BUF_SIZE = 255;
File fp;

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Lcd.init();                  // 初期化(135×240)
  M5.Lcd.setRotation(1);          // 液晶表示角度設定
  M5.Lcd.setTextWrap(true);       // テキストが画面からはみ出した時の折り返し有効
  M5.Lcd.clear();                 // 画面クリア
  M5.Lcd.setFont(&fonts::Font4);  // 横幅8 縦幅16
  M5.Lcd.setCursor(5, 0);         // ヘッダー描画
  M5.Lcd.println("<Initialize>"); // 〃
  SPIFFS.begin(true);
  listDir(SPIFFS, "/", 0);
  SPIFFS.end();
}

void loop() {

  if (M5.BtnA.wasPressed()) {
    SPIFFS.begin(true);
    fp = SPIFFS.open("/TEST.txt", FILE_WRITE);
    char buf[BUF_SIZE + 1];
    String s = "Kyocera DxIoT.\r\n";
    s.toCharArray(buf, BUF_SIZE);
    fp.write((uint8_t *)buf, BUF_SIZE);
    fp.close();
    SPIFFS.end();
    Serial.println("Write Success");
  }

  if (M5.BtnB.wasPressed()) {
    SPIFFS.begin(true);
    fp = SPIFFS.open("/TEST.txt", FILE_READ);
    if (fp)
      Serial.println("Read Success");
    else
      Serial.println("Read Miss");
    char buf[BUF_SIZE + 1];
    fp.read((uint8_t *)buf, BUF_SIZE);
    Serial.print(buf);
    fp.close();
    SPIFFS.end();
  }

  if (M5.BtnC.wasPressed()) {
    SPIFFS.begin(true);
    if (SPIFFS.remove("/TEST.txt"))
      Serial.println("Delete Success");
    else
      Serial.println("Delete Miss");
    SPIFFS.end();
  }

  M5.update();
}
