import configparser
import os.path
from InOne import Channel

class ConfigSwitch:
    def __init__(self, name, id):
        self.name = name
        self.id = id
    def __repr__(self):
        return self.name + " (" + str(self.id) + ")"

class ConfigSwitchChannel:
    def __init__(self, name, channel):
        self.name = name
        self.channel = channel
    def __repr__(self):
        return self.name + " (" + str(self.channel) + ")"

class ConfigLight:
    def __init__(self, name, topic, control, bound, timer):
        self.name = name
        self.topic = topic
        self.control = control
        self.bound = bound
        self.timer = timer
    def __repr__(self):
        return self.name + " (" + self.topic + ") control: " + str(self.control) + " bound: " + str(self.bound) + " timer: " + str(self.timer)

class ConfigIdeo:
    def __init__(self, minAirflow, maxAirflow):
        self.minAirflow = minAirflow
        self.maxAirflow = maxAirflow

class ConfigReader:
    def __init__(self, filename):
        self.__parser = configparser.ConfigParser()
        self.__parser.read(filename)
        self.__general = self.__parser["General"]

    def getSerialPort(self):
        return self.__general.get("SerialPort", "COM1")

    def getMqttHost(self):
        return self.__general.get("MqttHost", "127.0.0.1")

    def _getNamedSwitches(self, section):
        switches = []
        for name in self.__parser[section]:
            id = int(self.__parser[section][name])
            switches.append(ConfigSwitch(name, id))
        return switches

    def getSwitches(self):
        return self._getNamedSwitches('Switches')

    def getVirtualSwitches(self):
        return self._getNamedSwitches('VirtualSwitches')

    def ideo(self):
        return ConfigIdeo(
            int(self.__parser["Ideo"].get("AirflowLow", "90")),
            int(self.__parser["Ideo"].get("AirflowHigh", "325"))
        )

    def _parseSwitchChannel(self, str):
        tokens = str.strip().split(",")
        name = tokens[0].strip().lower()
        channel = tokens[1].strip()
        return ConfigSwitchChannel(name, Channel[channel])

    def getLights(self):
        lights = []
        for section in self.__parser.sections():
            # Lights are all sections beginning with "Light:"
            if not section.startswith("Light:"):
                continue
            # Strip the prefix to get the name
            name = section[len("Light:"):].lower()
            light = self.__parser[section]
            topic = light.get("Topic")
            timer = int(light.get("Timer", "0"))

            # Get the controlling switch for this light
            controlSwitch = self._parseSwitchChannel(light.get("Control"))

            # Get bound switches
            boundSwitches = []
            boundSwitchList = light.get("Bind").split("\n")
            for boundSwitchStr in boundSwitchList:
                boundSwitches.append(self._parseSwitchChannel(boundSwitchStr))

            lights.append(ConfigLight(name, topic, controlSwitch, boundSwitches, timer))
        return lights