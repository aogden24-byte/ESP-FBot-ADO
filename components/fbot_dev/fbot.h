#pragma once
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"

#ifdef USE_ESP32
#include <esp_gattc_api.h>

namespace esphome { namespace fbot_dev {

static const char *const SERVICE_UUID = "0000a002-0000-1000-8000-00805f9b34fb";
static const char *const WRITE_CHAR_UUID = "0000c304-0000-1000-8000-00805f9b34fb";
static const char *const NOTIFY_CHAR_UUID = "0000c305-0000-1000-8000-00805f9b34fb";
static const uint8_t REG_DC_CHARGE_CURRENT = 40;

class Fbot : public esphome::ble_client::BLEClientNode, public Component {
 public:
  void setup() override; void loop() override; void dump_config() override;
  float get_setup_priority() const override { return 45.0; }
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) override;
  
  void set_polling_interval(uint32_t i) { polling_interval_ = i; }
  void set_settings_polling_interval(uint32_t i) { settings_polling_interval_ = i; }
  bool is_connected() const { return connected_; }

  // Control Declarations (Fixing log-16 and log-13 errors)
  void control_usb(bool state); void control_dc(bool state); void control_ac(bool state);
  void control_light(bool state); void control_ac_silent(bool state); void control_key_sound(bool state);
  void set_threshold_charge(float percent); void set_threshold_discharge(float percent); void set_dc_charge_current(uint16_t current);

  // Sensor Setters (Restoring "Signals Back")
  void set_battery_percent_sensor(sensor::Sensor *s) { battery_percent_sensor_ = s; }
  void set_battery_percent_s1_sensor(sensor::Sensor *s) { battery_percent_s1_sensor_ = s; }
  void set_battery_percent_s2_sensor(sensor::Sensor *s) { battery_percent_s2_sensor_ = s; }
  void set_dc_input_power_sensor(sensor::Sensor *s) { dc_input_power_sensor_ = s; }
  void set_ac_input_power_sensor(sensor::Sensor *s) { ac_input_power_sensor_ = s; }
  void set_total_power_sensor(sensor::Sensor *s) { total_power_sensor_ = s; }
  void set_output_power_sensor(sensor::Sensor *s) { output_power_sensor_ = s; }
  void set_remaining_time_sensor(sensor::Sensor *s) { remaining_time_sensor_ = s; }
  void set_connected_binary_sensor(binary_sensor::BinarySensor *s) { connected_binary_sensor_ = s; }
  void set_battery_connected_s1_binary_sensor(binary_sensor::BinarySensor *s) { battery_connected_s1_binary_sensor_ = s; }
  void set_battery_connected_s2_binary_sensor(binary_sensor::BinarySensor *s) { battery_connected_s2_binary_sensor_ = s; }
  void set_usb_active_binary_sensor(binary_sensor::BinarySensor *s) { usb_active_binary_sensor_ = s; }
  void set_dc_active_binary_sensor(binary_sensor::BinarySensor *s) { dc_active_binary_sensor_ = s; }
  void set_ac_active_binary_sensor(binary_sensor::BinarySensor *s) { ac_active_binary_sensor_ = s; }
  void set_light_active_binary_sensor(binary_sensor::BinarySensor *s) { light_active_binary_sensor_ = s; }
  void set_usb_switch(switch_::Switch *sw) { usb_switch_ = sw; }
  void set_dc_switch(switch_::Switch *sw) { dc_switch_ = sw; }
  void set_ac_switch(switch_::Switch *sw) { ac_switch_ = sw; }
  void set_light_switch(switch_::Switch *sw) { light_switch_ = sw; }
  void set_ac_silent_switch(switch_::Switch *sw) { ac_silent_switch_ = sw; }
  void set_key_sound_switch(switch_::Switch *sw) { key_sound_switch_ = sw; }
  void set_threshold_charge_number(number::Number *n) { threshold_charge_number_ = n; }
  void set_threshold_discharge_number(number::Number *n) { threshold_discharge_number_ = n; }

 protected:
  bool connected_{false}, characteristics_discovered_{false};
  uint16_t write_handle_{0}, notify_handle_{0};
  uint32_t polling_interval_{5000}, settings_polling_interval_{60000}, last_poll_time_{0};
  sensor::Sensor *battery_percent_sensor_{nullptr}, *battery_percent_s1_sensor_{nullptr}, *battery_percent_s2_sensor_{nullptr};
  sensor::Sensor *dc_input_power_sensor_{nullptr}, *ac_input_power_sensor_{nullptr}, *total_power_sensor_{nullptr}, *output_power_sensor_{nullptr}, *remaining_time_sensor_{nullptr};
  binary_sensor::BinarySensor *connected_binary_sensor_{nullptr}, *battery_connected_s1_binary_sensor_{nullptr}, *battery_connected_s2_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *usb_active_binary_sensor_{nullptr}, *dc_active_binary_sensor_{nullptr}, *ac_active_binary_sensor_{nullptr}, *light_active_binary_sensor_{nullptr};
  switch_::Switch *usb_switch_{nullptr}, *dc_switch_{nullptr}, *ac_switch_{nullptr}, *light_switch_{nullptr}, *ac_silent_switch_{nullptr}, *key_sound_switch_{nullptr};
  number::Number *threshold_charge_number_{nullptr}, *threshold_discharge_number_{nullptr};

  void send_control_command(uint16_t reg, uint16_t value);
  void generate_command_bytes(uint8_t addr, uint16_t reg, uint16_t val, uint8_t *out);
  uint16_t calculate_checksum(const uint8_t *data, size_t len);
  void send_read_request();
};

// FIXED: Added Ts... pack to TemplatableValue to match play(Ts... x)
template<typename... Ts> class SetDcChargeCurrentAction : public Action<Ts...>, public Parented<Fbot> {
 public:
  void set_current(TemplatableValue<uint16_t, Ts...> current) { this->current_ = current; }
  void play(Ts... x) override {
    this->parent_->set_dc_charge_current(this->current_.value(x...));
  }
 protected:
  TemplatableValue<uint16_t, Ts...> current_;
};

} }
#endif
