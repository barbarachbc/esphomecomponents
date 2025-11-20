import esphome.codegen as cg
from esphome.components import light
import esphome.config_validation as cv
from esphome.const import CONF_CHANNEL, CONF_OUTPUT_ID

from .. import CONF_CAP1166_ID, CAP1166Component, cap1166_ns

DEPENDENCIES = ["cap1166"]

# Constants for LED behavior configuration
CONF_LED_BEHAVIOR = "led_behavior"

CAP1166LedBehavior = cap1166_ns.enum("CAP1166LedBehavior")
LED_BEHAVIORS = {
    "DIRECT": CAP1166LedBehavior.LED_BEHAVIOR_DIRECT,
    "PULSE1": CAP1166LedBehavior.LED_BEHAVIOR_PULSE1,
    "PULSE2": CAP1166LedBehavior.LED_BEHAVIOR_PULSE2,
    "BREATHE": CAP1166LedBehavior.LED_BEHAVIOR_BREATHE,
}

CAP1166Light = cap1166_ns.class_("CAP1166Light", light.LightOutput)

CONFIG_SCHEMA = light.BINARY_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_CAP1166_ID): cv.use_id(CAP1166Component),
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(CAP1166Light),
        cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=5),
        cv.Optional(CONF_LED_BEHAVIOR, default="DIRECT"): cv.enum(LED_BEHAVIORS, upper=True),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await light.register_light(var, config)
    
    parent = await cg.get_variable(config[CONF_CAP1166_ID])
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    cg.add(var.set_led_behavior(config[CONF_LED_BEHAVIOR]))
    cg.add(parent.register_channel(var))
