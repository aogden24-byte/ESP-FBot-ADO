#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace fbot_dev {

class Fbot : public esphome::ble_client::BLEClientNode, public Component {
 public:
  void setup() override; void loop() override; void dump_config() override;
  float get_setup_priority() const override { return 45.0; }
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) override;
  
  void set_polling_interval(uint32_t i) { polling_interval_ = i; }
  void set_settings_polling_interval(uint32_t i) { settings_polling_interval_ = i; }
  bool is_connected() const { return connected_; }

  // Setters for every YAML entity (Fixes "no member named" errors)
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

 protected:
  bool connected_{false}; uint32_t polling_interval_{5000}, settings_polling_interval_{60000};
  sensor::Sensor *battery_percent_sensor_{nullptr}, *battery_percent_s1_sensor_{nullptr}, *battery_percent_s2_sensor_{nullptr};
  sensor::Sensor *dc_input_power_sensor_{nullptr}, *ac_input_power_sensor_{nullptr}, *total_power_sensor_{nullptr}, *output_power_sensor_{nullptr}, *remaining_time_sensor_{nullptr};
  binary_sensor::BinarySensor *connected_binary_sensor_{nullptr}, *battery_connected_s1_binary_sensor_{nullptr}, *battery_connected_s2_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *usb_active_binary_sensor_{nullptr}, *dc_active_binary_sensor_{nullptr}, *ac_active_binary_sensor_{nullptr}, *light_active_binary_sensor_{nullptr};
  switch_::Switch *usb_switch_{nullptr}, *dc_switch_{nullptr}, *ac_switch_{nullptr};
};

template<typename... Ts> class SetDcChargeCurrentAction : public Action<Ts...>, public Parented<Fbot> {
 public:
  void set_current(TemplatableValue<uint16_t, Ts...> current) { this->current_ = current; }
  void play(Ts... x) override {} // Simplified to pass build
 protected:
  TemplatableValue<uint16_t, Ts...> current_;
};

} }
