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

// FULL CONTROL IMPLEMENTATION (Restored from stable repo review)
void Fbot::control_usb(bool state) { this->send_control_command(REG_USB_CONTROL, state ? 1 : 0); }
void Fbot::control_dc(bool state) { this->send_control_command(REG_DC_CONTROL, state ? 1 : 0); }
void Fbot::control_ac(bool state) { this->send_control_command(REG_AC_CONTROL, state ? 1 : 0); }
void Fbot::control_light(bool state) { this->send_control_command(REG_LIGHT_CONTROL, state ? 1 : 0); }
void Fbot::control_ac_silent(bool state) { this->send_control_command(REG_AC_SILENT_CONTROL, state ? 1 : 0); }
void Fbot::control_key_sound(bool state) { this->send_control_command(REG_KEY_SOUND, state ? 1 : 0); }
void Fbot::control_light_mode(const std::string &value) {
  uint16_t mode = 0;
  if (value == "On") mode = 1; else if (value == "SOS") mode = 2; else if (value == "Flashing") mode = 3;
  this->send_control_command(REG_LIGHT_CONTROL, mode);
}
void Fbot::set_threshold_charge(float percent) { this->send_control_command(REG_THRESHOLD_CHARGE, (uint16_t)(percent * 10)); }
void Fbot::set_threshold_discharge(float percent) { this->send_control_command(REG_THRESHOLD_DISCHARGE, (uint16_t)(percent * 10)); }
void Fbot::set_dc_charge_current(uint16_t current) {
  ESP_LOGI(TAG, "Commanding DC Charge Limit: %u A", current);
  this->send_control_command(REG_DC_CHARGE_CURRENT, current);
}

void Fbot::send_control_command(uint16_t reg, uint16_t value) {
  if (!this->connected_) return;
  uint8_t cmd[8]; this->generate_command_bytes(0x11, reg, value, cmd);
  esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(), this->write_handle_, 8, cmd, ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
}

void Fbot::send_read_request() {
  uint8_t payload[6] = {0x11, 0x04, 0x00, 0x00, 0x00, 0x50};
  uint16_t crc = calculate_checksum(payload, 6);
  uint8_t cmd[8]; memcpy(cmd, payload, 6); cmd[6] = (crc >> 8); cmd[7] = crc;
  esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(), this->write_handle_, 8, cmd, ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
}

uint16_t Fbot::calculate_checksum(const uint8_t *data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) { if (crc & 1) crc = (crc >> 1) ^ 0xA001; else crc >>= 1; }
  }
  return crc;
}

void Fbot::generate_command_bytes(uint8_t addr, uint16_t reg, uint16_t val, uint8_t *out) {
  out[0] = addr; out[1] = 0x06; out[2] = (reg >> 8); out[3] = reg; out[4] = (val >> 8); out[5] = val;
  uint16_t crc = calculate_checksum(out, 6); out[6] = (crc >> 8); out[7] = crc;
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
    auto *n = this->parent()->get_characteristic(esp32_ble_tracker::ESPBTUUID::from_raw(SERVICE_UUID), esp32_ble_tracker::ESPBTUUID::from_raw(NOTIFY_CHAR_UUID));
    if (n) {
      this->notify_handle_ = n->handle;
      esp_ble_gattc_register_for_notify(gattc_if, this->parent()->get_remote_bda(), this->notify_handle_);
    }
    this->characteristics_discovered_ = true;
    this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;
  }
}

void Fbot::parse_notification(const uint8_t *data, uint16_t length) {
  if (length < 6 || data[0] != 0x11 || data[1] != 0x04) return;
  uint16_t offset = 6;
  auto get_reg = [&](uint16_t reg) { return (uint16_t)(data[offset + (reg*2)] << 8 | data[offset + (reg*2) + 1]); };
  
  if (this->battery_percent_sensor_) this->battery_percent_sensor_->publish_state(get_reg(56) / 10.0f);
  if (this->battery_percent_s1_sensor_) this->battery_percent_s1_sensor_->publish_state(get_reg(53) / 10.0f);
  if (this->battery_percent_s2_sensor_) this->battery_percent_s2_sensor_->publish_state(get_reg(55) / 10.0f);
  if (this->dc_input_power_sensor_) this->dc_input_power_sensor_->publish_state(get_reg(4));
  if (this->total_power_sensor_) this->total_power_sensor_->publish_state(get_reg(20));
}

void Fbot::dump_config() { ESP_LOGCONFIG(TAG, "Fbot Unit (Unified Master)"); }

} }
#endif
