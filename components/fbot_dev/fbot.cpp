#include "fbot.h"
#include "esphome/core/log.h"

namespace esphome { namespace fbot_dev {
static const char *TAG = "fbot_dev";
void Fbot::setup() {}
void Fbot::loop() {}
void Fbot::dump_config() { ESP_LOGCONFIG(TAG, "Fbot Unit (Simplified)"); }
void Fbot::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {}
} }
