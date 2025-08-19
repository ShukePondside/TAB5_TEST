#include <M5Unified.h>

void setup() {
  auto cfg = M5.config();  // M5Stack初期設定用の構造体を代入
  M5.begin(cfg);           // M5デバイスの初期化

  M5.Display.setRotation(2);
  M5.Display.setFont(&fonts::FreeMonoBold24pt7b);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setCursor(10, 10);
  M5.Display.print("Hello World!!");  // 画面にHello World!!と1行表示
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

    M5.Display.setCursor(10, 60);
    M5.Display.printf("Press:%d", press);

    M5.Display.setCursor(10, 110);
    M5.Display.printf("Base_X:%4d, Base_Y:%4d", base_x, base_y);

    M5.Display.setCursor(10, 160);
    M5.Display.printf("Dist_X:%4d, Dist_Y:%4d", distance_x, distance_y);
  }
  M5.Display.endWrite();

  delay(1);
}