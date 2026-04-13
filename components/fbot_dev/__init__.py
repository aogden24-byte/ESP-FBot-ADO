import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client
from esphome.const import CONF_ID

AUTO_LOAD = ["ble_client", "sensor", "binary_sensor", "switch", "number"]
CONF_FBOT_ID = "fbot_id"

fbot_ns = cg.esphome_ns.namespace("fbot_dev")
Fbot = fbot_ns.class_("Fbot", cg.Component, ble_client.BLEClientNode)

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.declare_id(Fbot),
        cv.Optional("polling_interval", default="5s"): cv.positive_time_period_milliseconds,
        cv.Optional("settings_polling_interval", default="60s"): cv.positive_time_period_milliseconds,
    }).extend(cv.COMPONENT_SCHEMA).extend(ble_client.BLE_CLIENT_SCHEMA),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)
    if "polling_interval" in config:
        cg.add(var.set_polling_interval(config["polling_interval"]))
    if "settings_polling_interval" in config:
        cg.add(var.set_settings_polling_interval(config["settings_polling_interval"]))
