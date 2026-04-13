#include "fbot.h"
#include "esphome/core/log.h"

#ifdef USE_ESP32
namespace esphome { namespace fbot_dev {

static const char *TAG = "fbot_dev";

void Fbot::setup() { this->update_connected_state(false); }

void Fbot::loop() {
  if (this->connected_ && this->characteristics_discovered_) {
    uint32_t now = millis();
    if (now - this->last_poll_time_ >= this->polling_interval_) {
      this->send_read_request();
      this->last_poll_time_ = now;
    }
  }
}

void Fbot::set_dc_charge_current(uint16_t current) {
  ESP_LOGI(TAG, "Commanding DC Charge Limit: %u Amps", current);
  this->send_control_command(REG_DC_CHARGE_CURRENT, current);
}

void Fbot::send_control_command(uint16_t reg, uint16_t value) {
  if (!this->connected_) return;
  uint8_t cmd[8];
  this->generate_command_bytes(0x11, reg, value, cmd);
  esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(), this->write_handle_, 8, cmd, ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
}

void Fbot::send_read_request() {
  uint8_t payload[6] = {0x11, 0x04, 0x00, 0x00, 0x00, 0x50};
  uint16_t crc = this->calculate_checksum(payload, 6);
  uint8_t cmd[8]; memcpy(cmd, payload, 6);
  cmd[6] = (crc >> 8) & 0xFF; cmd[7] = crc & 0xFF;
  esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(), this->write_handle_, 8, cmd, ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
}

uint16_t Fbot::calculate_checksum(const uint8_t *data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 1) crc = (crc >> 1) ^ 0xA001;
      else crc >>= 1;
    }
  }
  return crc;
}

void Fbot::generate_command_bytes(uint8_t addr, uint16_t reg, uint16_t val, uint8_t *out) {
  out[0] = addr; out[1] = 0x06; out[2] = (reg >> 8); out[3] = reg; out[4] = (val >> 8); out[5] = val;
  uint16_t crc = calculate_checksum(out, 6);
  out[6] = (crc >> 8); out[7] = crc;
}

void Fbot::update_connected_state(bool state) {
  this->connected_ = state;
  if (this->connected_binary_sensor_) this->connected_binary_sensor_->publish_state(state);
}

void Fbot::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {
  if (event == ESP_GATTC_OPEN_EVT && param->open.status == ESP_GATT_OK) {
    this->connected_ = true; this->update_connected_state(true);
  } else if (event == ESP_GATTC_SEARCH_CMPL_EVT) {
    auto *w = this->parent()->get_characteristic(esp32_ble_tracker::ESPBTUUID::from_raw(SERVICE_UUID), esp32_ble_tracker::ESPBTUUID::from_raw(WRITE_CHAR_UUID));
    if (w) this->write_handle_ = w->handle;
    this->characteristics_discovered_ = true;
    this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;
  }
}

void Fbot::dump_config() { ESP_LOGCONFIG(TAG, "Fbot Unit (ADO Dev)"); }

} }
#endif
