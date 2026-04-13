#pragma once
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/switch/switch.h"

namespace esphome { namespace fbot_dev {

class Fbot : public esphome::ble_client::BLEClientNode, public Component {
 public:
  void setup() override; void loop() override; void dump_config() override;
  float get_setup_priority() const override { return 45.0; }
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) override;

  void set_dc_charge_current(uint16_t current);
  
  // Setters for YAML
  void set_battery_percent_sensor(sensor::Sensor *s) { battery_percent_sensor_ = s; }
  void set_dc_input_power_sensor(sensor::Sensor *s) { dc_input_power_sensor_ = s; }
  void set_total_power_sensor(sensor::Sensor *s) { total_power_sensor_ = s; }
  void set_connected_binary_sensor(binary_sensor::BinarySensor *s) { connected_binary_sensor_ = s; }

 protected:
  sensor::Sensor *battery_percent_sensor_{nullptr}, *dc_input_power_sensor_{nullptr}, *total_power_sensor_{nullptr};
  binary_sensor::BinarySensor *connected_binary_sensor_{nullptr};
};

template<typename... Ts> class SetDcChargeCurrentAction : public Action<Ts...>, public Parented<Fbot> {
 public:
  TEMPLATABLE_VALUE(uint16_t, current)
  // FIXED: Added 'const' and '&' to match the base class signature exactly
  void play(const Ts &...x) override {
    this->parent_->set_dc_charge_current(this->current.value(x...));
  }
};

} }
