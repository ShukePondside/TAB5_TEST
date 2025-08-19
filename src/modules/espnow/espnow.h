#pragma once

#include <M5Unified.h>
#include <WiFi.h>
#include <esp_now.h>
#include <unordered_map>
#include <array>

#define PEER_MAX 19

namespace EspNow {

enum TerminalKind : int8_t {
  KIND_MASTER = 0,
  KIND_SLAVE = 1,
  KIND_BGM = 2
};

// 通信コマンド
enum Command : int8_t {
  CMD_NONE = 0,
  CMD_PEERING = 1,
  CMD_PARAM_SET = 2,
  CMD_MODE_CHANGE = 3,
  CMD_DATA_UPDATE = 4,
  CMD_RESET = -1
};

// モード
enum Mode : int8_t {
  MODE_NONE = 0,
  MODE_BOOT = 1,
  MODE_TITLE = 2,
  MODE_OPENING = 3,
  MODE_GAME_FIRST = 4,
  MODE_GAME_INTERVAL = 5,
  MODE_GAME_SECOND = 6,
  MODE_ENDING = 7,
};

enum EnemyKind {
  ENEMY_WHITE = 0,
  ENEMY_RED = 2,
  ENEMY_REINBOW = 4
};

struct GameData {
  bool game_round = false;
  bool bonus_time = false;
  // bool is_reinbow = false;  // 虹倒したフラグ(再出現禁止)
  EnemyKind enemy_kind = ENEMY_WHITE;
  uint16_t defert_count = 0;
};

struct GameParam {
  uint8_t volume;
  uint16_t lux_border;
  uint16_t first_arrival_min;
  uint16_t first_arrival_max;
  uint16_t first_escape_min;
  uint16_t first_escape_max;
  uint16_t second_arrival_min;
  uint16_t second_arrival_max;
  uint16_t second_escape_min;
  uint16_t second_escape_max;
  uint16_t red_ratio;
  uint16_t reinbow_ratio;
};

struct GameInfo {
  GameData data;
  GameParam param;
  SemaphoreHandle_t mutex_game_data;
};

struct Message {
  char magic[6];
  std::array<uint8_t, 6> mac;
  TerminalKind terminal_kind;
  Command command;
  Mode mode;
  GameInfo game_info;
};

void Initialize();
std::unordered_map<std::string, std::array<uint8_t, 6>> GetPeerInfo();
bool AddPeer(const std::string &key, const std::array<uint8_t, 6> &mac);
bool DeletePeer(const std::string &key);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac, const uint8_t *data, int data_len);
void tRecvTask(void *pvParameters);
QueueHandle_t GetAppQueue();

}  // namespace EspNow
