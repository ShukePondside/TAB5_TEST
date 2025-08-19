#include <M5Unified.h>
#include <LittleFS.h>

#include <WiFi.h>
#include <esp_now.h>

#include "modules/display/display.h"
#include "modules/EspNow/EspNow.h"
#include "modules/filesystem/filesystem.h"
// #include "modules/sound/sound.h"

unsigned long start_mills;
unsigned long interval;

void setup() {
  auto cfg = M5.config();  // M5Stack初期設定用の構造体を代入
  M5.begin(cfg);           // M5デバイスの初期化

  M5.Display.setRotation(2);
  M5.Display.setBrightness(255);
  M5.Display.setFont(&fonts::FreeMonoBold24pt7b);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(1.5);
  M5.Display.setCursor(10, 10);
  M5.Display.print("Hello World!!");  // 画面にHello World!!と1行表示

  log_d("モジュール初期化");
  Display::Initialize();  // ディスプレイ初期化
  // Sound::Initialize();    // サウンド設定初期化
  EspNow::Initialize();  // ESP-NOW設定初期化

  // ブロードキャストをペアリング ///////////////////////////////////////
  EspNow::AddPeer("broad_cast", {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
  /////////////////////////////////////////////////////////////////////

  // 送信メッセージヘッダ構築 /////////////////////////
  EspNow::Message send_message;
  strcpy(send_message.magic, MAGIC);
  send_message.terminal_kind = EspNow::KIND_MASTER;
  ///////////////////////////////////////////////////

  // 各端末にブロードキャストで再起動指令 ////////////
  send_message.command = EspNow::CMD_RESET;
  esp_err_t result = esp_now_send(EspNow::GetPeerInfo().at("broad_cast").data(), (uint8_t *)&send_message, sizeof(send_message));
  //////////////////////////////////////////////

  uint8_t slave_qty = 0;
  int8_t bgm_qty = 0;

  xSemaphoreTake(Display::GetDisplayMutex(), portMAX_DELAY);
  M5.Display.setCursor(0, 40);
  M5.Display.println("Slave-QTY:" + String(slave_qty));
  M5.Display.setCursor(0, 80);
  M5.Display.println("BGM-QTY:" + String(bgm_qty));
  xSemaphoreGive(Display::GetDisplayMutex());

  send_message.command = EspNow::CMD_PEERING;
  EspNow::Message recive_message;
  interval = 3000;  // ペアリング実行時間(msec)
  start_mills = millis();
  while (millis() - start_mills < interval) {
    esp_err_t result = esp_now_send(EspNow::GetPeerInfo().at("broad_cast").data(), (uint8_t *)&send_message, sizeof(send_message));
    while (xQueueReceive(EspNow::GetAppQueue(), &recive_message, pdMS_TO_TICKS(100))) {
      switch (recive_message.terminal_kind) {
        case EspNow::KIND_SLAVE:
          if (EspNow::AddPeer("slave_" + std::to_string(slave_qty + 1), recive_message.mac)) {
            slave_qty++;
            xSemaphoreTake(display_mutex, portMAX_DELAY);
            M5.Display.setCursor(0, 40);
            M5.Display.println("Slave-QTY:" + String(slave_qty));
            xSemaphoreGive(display_mutex);
            log_d("Slaveとのペアリングに成功しました");
            log_d("Slave-Qty:%d", slave_qty);
          };
          break;
        case EspNow::KIND_BGM:
          if (EspNow::AddPeer("bgm_" + std::to_string(bgm_qty + 1), recive_message.mac)) {
            bgm_qty++;
            xSemaphoreTake(display_mutex, portMAX_DELAY);
            M5.Display.setCursor(0, 80);
            M5.Display.println("BGM-QTY:" + String(bgm_qty));
            xSemaphoreGive(display_mutex);
            log_d("GBMとのペアリングに成功しました");
            log_d("BGM-Qty:%d", bgm_qty);
          };
          break;
        default:
          break;
      }
    }
  }
  // ブロードキャストのペアリング削除 /////////////////////////////////
  EspNow::DeletePeer("broad_cast");
  //////////////////////////////////////////////////////////////////

  M5.Display.clearDisplay(TFT_BLACK);
}

void loop() {
  M5.update();

  M5.Display.startWrite();
  if (M5.Touch.isEnabled()) {
    auto touch_state = M5.Touch.getDetail();
    auto press = touch_state.isPressed();
    auto base_x = touch_state.base_x;
    auto base_y = touch_state.base_y;
    auto distance_x = touch_state.distanceX();
    auto distance_y = touch_state.distanceY();

    // if (!p) {
    //   base_x = 0;
    //   base_y = 0;
    // }

    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextSize(1);

    M5.Display.setCursor(10, 100);
    M5.Display.printf("Press:%d", press);

    M5.Display.setCursor(10, 150);
    M5.Display.printf("Base_X:%4d  Base_Y:%4d", base_x, base_y);

    M5.Display.setCursor(10, 200);
    M5.Display.printf("Dist_X:%4d  Dist_Y:%4d", distance_x, distance_y);
  }
  M5.Display.endWrite();

  delay(10);
}