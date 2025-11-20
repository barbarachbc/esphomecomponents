import esphome.codegen as cg
from esphome.components import light
import esphome.config_validation as cv
from esphome.const import CONF_CHANNEL, CONF_OUTPUT_ID

from .. import CONF_CAP1166_ID, CAP1166Component, cap1166_ns

DEPENDENCIES = ["cap1166"]

CAP1166Light = cap1166_ns.class_("CAP1166Light", light.LightOutput)

CONFIG_SCHEMA = light.BINARY_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_CAP1166_ID): cv.use_id(CAP1166Component),
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(CAP1166Light),
        cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=5),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await light.register_light(var, config)
    
    parent = await cg.get_variable(config[CONF_CAP1166_ID])
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    cg.add(parent.register_channel(var))
