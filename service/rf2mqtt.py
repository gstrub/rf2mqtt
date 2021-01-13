import paho.mqtt.client as mqtt
import serial
import signal

from ConfigReader import ConfigReader

import InOne
import Ideo
import Mqtt

def on_connect(client, userfata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe("lights/#")
    client.subscribe("ideo/#")

def on_message(client, userdata, msg):
    print(msg.topic + ":" + str(msg.payload))

class PrintAndSerialOutInt:
    def __init__(self, index, serial):
        self.__serial = serial
        self.__index = index
    def write(self,message):
        msg = bytes("{0}>{1}\n".format(self.__index, message), 'utf8')
        try:
            self.__serial.write(msg)
            print("Serial < " + str(msg))
        except Exception as e:
            print("Exception trying to write to serial port: " + str(e))

class MqttMonitorSwitch(InOne.FilterReceiver):
    def __init__(self, id, topic, client):
        InOne.FilterReceiver.__init__(self)
        self.bind(id, InOne.Channel.Any)
        self.__topic = topic
        self.__client = client
    def onFilteredEvent(self, event):
        try:
            topic_str = self.__topic + "/channel/" + event.channel.name.lower() + "/event"
            if type(event) is InOne.ReleaseEvent:
                event_str = "release"
            else:
                event_str = str(type(event).__name__)[:-5].lower() + "_" + event.command.name.lower()
            self.__client.publish(topic_str, event_str)
        except Exception as e:
            print("Exception trying to publish state: " + str(e))

config = ConfigReader("config.ini")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message


ser = serial.Serial(config.getSerialPort(), 115200, timeout=0.5)
while (not ser.readline().decode("utf-8").startswith("CC1101 TX")): pass

client.connect(config.getMqttHost())

# The InOne event dispatcher
dispatcher = InOne.Dispatcher()
# The serial message parser, sending decoded events to the dispatcher
parser = InOne.MessageParser(dispatcher)
# The output interface on which the outgoing commands will be sent
outputInterface = PrintAndSerialOutInt(0, ser)

# Named object storage
virtualSwitches = {}
switches = {}
lights = {}

# Build objects according to the configuration file
print("Virtual switches")
for sw in config.getVirtualSwitches():
    virtualSwitches[sw.name] = InOne.Switch(sw.id, outputInterface)
    print("  " + sw.name + ": " + str(sw.id))
    

print("Switches")
for sw in config.getSwitches():
    switches[sw.name] = sw.id
    print("  " + sw.name + ": " + str(sw.id))
    dispatcher.register(MqttMonitorSwitch(sw.id, "switches/" + sw.name, client))

print("Lights")
for l in config.getLights():
    lights[l.name] = Mqtt.TimerLight(l.topic, client)
    lights[l.name].bindControl(virtualSwitches[l.control.name], l.control.channel)
    print("  " + l.name + ": " + l.topic)
    print("    control: " + l.control.name + ", " + str(l.control.channel))
    for bs in l.bound:
        lights[l.name].bind(switches[bs.name], bs.channel)
        print("    bind: " + bs.name + ", " + str(bs.channel))
    lights[l.name].setDuration(l.timer)
    print("    timer: " + str(l.timer))
    dispatcher.register(lights[l.name])

ideo = Ideo.Ideo(PrintAndSerialOutInt(1, ser))
ideoConfig = config.ideo()
ideo.setMaxAirflow(ideoConfig.maxAirflow)
ideo.setMinAirflow(ideoConfig.minAirflow)
mqttIdeo = Mqtt.Ideo(ideo, client, "ideo")
ideo.register(mqttIdeo)

# Start monitoring MQTT messages
client.loop_start()

class LoopManager:
    def __init__(self):
        self._run = True
        signal.signal(signal.SIGINT, self.handler)

    def handler(self, sig, frame):
        self._run = False

    def run(self):
        return self._run

loop = LoopManager()

while loop.run():
    # Read one line from the RF-to-serial interface
    #try:
    input_message = ser.readline()
   # except Exception as e:
   #     print("Exception trying to read from serial port: " + str(e))
    #    run = False
    if input_message != b'':
        # Strip all whitespace and newline from the received message
        input_string = input_message.decode('utf-8').rstrip()
        print("Serial > " + input_string)
        # Try parsing the InOne message
        try:
            if input_string.startswith("0>"):
                parser.parse(input_string[2:])
            if input_string.startswith("1>"):
                ideo.parseIncomingMessage(input_string[2:])
        except Exception as e:
            print("Exception while parsing the message:" + str(e))

mqttIdeo.stopTimer()
client.loop_stop()