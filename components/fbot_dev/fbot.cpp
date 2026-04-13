#include "fbot.h"
#include "esphome/core/log.h"

namespace esphome { namespace fbot_dev {
void Fbot::setup() {}
void Fbot::loop() {}
void Fbot::dump_config() {}
void Fbot::set_dc_charge_current(uint16_t current) { ESP_LOGI("fbot", "Setting current: %u", current); }
void Fbot::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {}
} }
