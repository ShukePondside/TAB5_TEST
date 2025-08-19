#include "EspNow.h"

namespace EspNow {

SemaphoreHandle_t peer_mutex = nullptr;
// SemaphoreHandle_t mac_mutex = nullptr;
// SemaphoreHandle_t message_mutex = nullptr;
SemaphoreHandle_t data_mutex = nullptr;

// uint8_t from_mac[6] = {0};
std::unordered_map<std::string, std::array<uint8_t, 6>> peer_map;

// static Message message;

QueueHandle_t recv_queue;  // 受信コールバック関数からのデータ
QueueHandle_t app_queue;   // アプリ側にESP-NOWの受信データを送る

// uint16_t defeat_count = 0;  // 討伐数

QueueHandle_t GetAppQueue() {
  return app_queue;
}

bool AddPeer(const std::string &key, const std::array<uint8_t, 6> &mac) {
  xSemaphoreTake(peer_mutex, portMAX_DELAY);

  if (peer_map.size() >= PEER_MAX) {
    log_e("ペア登録数が上限に達しました");
    xSemaphoreGive(peer_mutex);
    return false;
  }
  for (const auto &pair : peer_map) {
    if (pair.second == mac) {
      log_e("既に登録済みのペアです");
      xSemaphoreGive(peer_mutex);
      return false;
    }
  }
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, mac.data(), 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    log_d("add_peer_error");
    xSemaphoreGive(peer_mutex);
    return false;
  }
  peer_map[key] = mac;
  xSemaphoreGive(peer_mutex);
  return true;
}

bool DeletePeer(const std::string &key) {
  xSemaphoreTake(peer_mutex, portMAX_DELAY);
  if (esp_now_del_peer(peer_map.at(key).data()) != ESP_OK) {
    log_d("delete_peer_error");
    xSemaphoreGive(peer_mutex);
    return false;
  }
  if (peer_map.find("broad_cast") != peer_map.end()) {
    peer_map.erase("broad_cast");
  }
  xSemaphoreGive(peer_mutex);
  return true;
}

void Initialize() {
  peer_mutex = xSemaphoreCreateMutex();
  data_mutex = xSemaphoreCreateMutex();

  // ESP-NOW初期化//////////////////
  WiFi.mode(WIFI_STA);
  if (esp_now_init() == ESP_OK) {
    // log_d("EspNow Init Success");
  } else {
    log_d("EspNow Init Failed");
    ESP.restart();
  }
  //////////////////////////////////

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  recv_queue = xQueueCreate(20, sizeof(Message));
  app_queue = xQueueCreate(20, sizeof(Message));

  xTaskCreate(tRecvTask, "tRecvTask", 4096, NULL, 1, NULL);

  log_d("EspNow Init Success");
}

std::unordered_map<std::string, std::array<uint8_t, 6>> GetPeerInfo() {
  xSemaphoreTake(peer_mutex, portMAX_DELAY);
  std::unordered_map<std::string, std::array<uint8_t, 6>> val = peer_map;
  xSemaphoreGive(peer_mutex);
  return val;
}

// uint8_t GetData() {
//   xSemaphoreTake(data_mutex, portMAX_DELAY);
//   uint8_t val = defeat_count;
//   xSemaphoreGive(data_mutex);
//   return val;
// }
// void SetData(const uint8_t *recive_message) {
//   xSemaphoreTake(data_mutex, portMAX_DELAY);
//   defeat_count = *recive_message;
//   xSemaphoreGive(data_mutex);
// }

// 送信コールバック
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  const uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  if (memcmp(mac_addr, broadcast_mac, 6) == 0) {
    log_d("Broadcast packet sent (status: %s)", status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
  } else {
    log_d("Unicast packet sent to %02X:%02X:%02X:%02X:%02X:%02X - %s",
          mac_addr[0], mac_addr[1], mac_addr[2],
          mac_addr[3], mac_addr[4], mac_addr[5],
          status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  }
  // char macStr[18];
  // snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
  //          mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
}

// 受信コールバック
void OnDataRecv(const uint8_t *mac, const uint8_t *data, int data_len) {
  log_d("mac:%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Message recive_message;
  memcpy(&recive_message, data, sizeof(recive_message));
  memcpy(recive_message.mac.data(), mac, 6);
  if (xQueueSendFromISR(recv_queue, &recive_message, nullptr) != pdTRUE) {
    log_e("受信キューが満杯！データを破棄しました");
  }
}

void tRecvTask(void *pvParameters) {
  Message recive_message;
  while (true) {
    if (xQueueReceive(recv_queue, &recive_message, portMAX_DELAY)) {
      log_d("mac:%02X:%02X:%02X:%02X:%02X:%02X",
            recive_message.mac[0], recive_message.mac[1], recive_message.mac[2],
            recive_message.mac[3], recive_message.mac[4], recive_message.mac[5]);
      // 再起動処理
      if (strncmp(recive_message.magic, MAGIC, 5) == 0 &&
          recive_message.command == CMD_RESET) {
        log_d("ESP-RESTART");
        esp_restart();
      }
      if (xQueueSendFromISR(app_queue, &recive_message, nullptr) != pdTRUE) {
        log_e("アプリ側受信キューが満杯！データを破棄しました");
      }
    }
  }
}

}  // namespace EspNow