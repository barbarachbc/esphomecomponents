import esphome.codegen as cg
from esphome.components import light
import esphome.config_validation as cv
from esphome.const import CONF_CHANNEL, CONF_OUTPUT_ID, CONF_INTERNAL

from .. import CONF_CAP1166_ID, CAP1166Component, cap1166_ns, LED_BEHAVIORS

DEPENDENCIES = ["cap1166"]

# Constants for LED behavior configuration
CONF_LED_BEHAVIOR = "led_behavior"
CONF_LINKED_TO_TOUCH = "linked"

def check_linked(obj):
    if CONF_LINKED_TO_TOUCH not in obj or not obj[CONF_LINKED_TO_TOUCH]:
        return obj
    if CONF_INTERNAL not in obj or not obj[CONF_INTERNAL]:
        msg = "If linked to touch, has to be internal"
        raise cv.Invalid(msg)
    return obj

CAP1166Light = cap1166_ns.class_("CAP1166Light", light.LightOutput)

CONFIG_SCHEMA = cv.All(
    light.BINARY_LIGHT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_CAP1166_ID): cv.use_id(CAP1166Component),
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(CAP1166Light),
            cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=5),
            cv.Optional(CONF_LED_BEHAVIOR, default="DIRECT"): cv.enum(LED_BEHAVIORS, upper=True),
            cv.Optional(CONF_LINKED_TO_TOUCH, default=False): cv.boolean,
        }
    ).extend(cv.COMPONENT_SCHEMA),
    check_linked,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await light.register_light(var, config)
    
    parent = await cg.get_variable(config[CONF_CAP1166_ID])
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    cg.add(var.set_led_behavior(config[CONF_LED_BEHAVIOR]))
    cg.add(var.set_link_to_touch(config[CONF_LINKED_TO_TOUCH]))
    cg.add(parent.register_channel(var))
