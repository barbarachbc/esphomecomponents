import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_CHANNEL

from . import CONF_CAP1166_ID, CAP1166Component, cap1166_ns

DEPENDENCIES = ["cap1166"]
CAP1166Channel = cap1166_ns.class_("CAP1166Channel", binary_sensor.BinarySensor)

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(CAP1166Channel).extend(
    {
        cv.GenerateID(CONF_CAP1166_ID): cv.use_id(CAP1166Component),
        cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=5),
    }
)


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    hub = await cg.get_variable(config[CONF_CAP1166_ID])
    cg.add(var.set_channel(config[CONF_CHANNEL]))

    cg.add(hub.register_channel(var))
