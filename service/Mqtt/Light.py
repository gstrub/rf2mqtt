from InOne import Switch, State
import InOne

# TODO decouple Mqtt functionality from Iobl classes
"""
MqttLight
A standard on/off light controlled via MQTT
"""
class Light(InOne.Light):
    def __init__(self, topic, client):
        super().__init__()
        self.__topic = topic
        self.__client = client
        client.message_callback_add(topic + "/#", self.on_message)

    def on_message(self, client, userdata, msg):
        if not msg.topic.startswith(self.__topic):
            raise ValueError("Callback topic " + msg.topic + " does not match topic " + self.__topic)
        subtopic = msg.topic[len(self.__topic):]
        print("Got message! Topic: " + msg.topic + ": " + str(msg.payload))

        if subtopic == "/command":
            if msg.payload == b"on":
                self.turnOn()
            if msg.payload == b"off":
                self.turnOff()
    
    def setState(self, state: State):
        print("MqttLight.setState")
        InOne.Light.setState(self, state)
        try:
            self.__client.publish(self.__topic + "/state", state.name.lower(), 0, True)
        except Exception as e:
            print("Exception trying to publish state: " + str(e))

    def topic(self):
        return self.__topic

class TimerLight(InOne.TimerLight, Light):
    def __init__(self, topic, client, duration = 0):
        InOne.TimerLight.__init__(self, duration)
        Light.__init__(self, topic, client)
        
    def on_message(self, client, userdata, msg):
        super().on_message(client, userdata, msg)
        subtopic = msg.topic[len(self.topic()):]

        if subtopic == "/timer":
            try:
                duration = int(msg.payload)
                self.setDuration(duration)
            except Exception as e:
                print("Exception while parsing duration: " + str(e))

        if subtopic == "/command":
            if msg.payload == b"hold":
                self.holdOn()

    def setState(self, state: Switch):
        InOne.TimerLight.setState(self, state)        
        Light.setState(self, state)
        