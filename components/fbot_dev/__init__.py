import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import ble_client
from esphome.const import CONF_ID

AUTO_LOAD = ["ble_client", "sensor", "binary_sensor"]
CONF_FBOT_ID = "fbot_id"

fbot_ns = cg.esphome_ns.namespace("fbot_dev")
Fbot = fbot_ns.class_("Fbot", cg.Component, ble_client.BLEClientNode)
SetDcChargeCurrentAction = fbot_ns.class_("SetDcChargeCurrentAction", automation.Action)

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.declare_id(Fbot),
    }).extend(cv.COMPONENT_SCHEMA).extend(ble_client.BLE_CLIENT_SCHEMA),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)

@automation.register_action(
    "fbot_dev.set_dc_charge_current",
    SetDcChargeCurrentAction,
    automation.maybe_simple_id({
        cv.GenerateID(): cv.use_id(Fbot),
        cv.Required("current"): cv.templatable(cv.uint16_t),
    }),
)
async def fbot_set_dc_charge_current_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config["current"], args, cg.uint16)
    cg.add(var.set_current(template_))
    return var
