#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/switch/switch.h"

#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif

#ifdef USE_ESP32
#include <esp_gattc_api.h>

namespace esphome {
namespace fbot_dev {

static const char *const SERVICE_UUID = "0000a002-0000-1000-8000-00805f9b34fb";
static const char *const WRITE_CHAR_UUID = "0000c304-0000-1000-8000-00805f9b34fb";
static const char *const NOTIFY_CHAR_UUID = "0000c305-0000-1000-8000-00805f9b34fb";

static const uint8_t REG_DC_CHARGE_CURRENT = 40; 

class Fbot : public esphome::ble_client::BLEClientNode, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) override;
  
  void set_polling_interval(uint32_t interval) { this->polling_interval_ = interval; }
  void set_settings_polling_interval(uint32_t interval) { this->settings_polling_interval_ = interval; }
  
  void set_battery_percent_sensor(sensor::Sensor *s) { battery_percent_sensor_ = s; }
  void set_dc_input_power_sensor(sensor::Sensor *s) { dc_input_power_sensor_ = s; }
  void set_total_power_sensor(sensor::Sensor *s) { total_power_sensor_ = s; }
  void set_connected_binary_sensor(binary_sensor::BinarySensor *s) { connected_binary_sensor_ = s; }

  void set_dc_charge_current(uint16_t current);
  void send_control_command(uint16_t reg, uint16_t value);

 protected:
  uint16_t write_handle_{0}, notify_handle_{0};
  uint32_t polling_interval_{2000}, settings_polling_interval_{60000}, last_poll_time_{0};
  bool connected_{false}, characteristics_discovered_{false};
  sensor::Sensor *battery_percent_sensor_{nullptr}, *dc_input_power_sensor_{nullptr}, *total_power_sensor_{nullptr};
  binary_sensor::BinarySensor *connected_binary_sensor_{nullptr};

  uint16_t calculate_checksum(const uint8_t *data, size_t len);
  void generate_command_bytes(uint8_t addr, uint16_t reg, uint16_t val, uint8_t *out);
  void send_read_request();
  void update_connected_state(bool state);
};

template<typename... Ts> class SetDcChargeCurrentAction : public Action<Ts...>, public Parented<Fbot> {
 public:
  // The Fix: Added set_current setter for Python and defined 'current' explicitly
  void set_current(TemplatableValue<uint16_t> current) { this->current = current; }

  void play(const Ts &...x) override {
    this->parent_->set_dc_charge_current(this->current.value(x...));
  }
 protected:
  TemplatableValue<uint16_t> current;
};

} }
#endif
