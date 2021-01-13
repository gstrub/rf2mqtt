from threading import Timer
#https://stackoverflow.com/questions/12435211/python-threading-timer-repeat-function-every-n-seconds

class RepeatTimer(Timer):
    def run(self):
        while not self.finished.wait(self.interval):
            self.function(*self.args, **self.kwargs)

class Ideo:
    def __init__(self, ideo, client, topic):
        self.__client = client
        self.__topic = topic
        self.__ideo = ideo
        client.message_callback_add(topic + "/#", self.on_message)
        self.__timer = RepeatTimer(60, self._requestAll)
        self.__timer.start()

    def __del__(self):
        self.__timer.cancel()

    def _requestAll(self):
        self.__ideo.requestInsideTemp()
        self.__ideo.requestOutsideTemp()
        self.__ideo.requestStatus()

    def stopTimer(self):
        self.__timer.cancel()
        
    def on_message(self, client, userdata, msg):
        try:
            if not msg.topic.startswith(self.__topic):
                raise ValueError("Callback topic " + msg.topic + " does not match topic " + self.__topic)
            subtopic = msg.topic[len(self.__topic):]
            print("Got message! Topic: " + msg.topic + ": " + str(msg.payload))

            if subtopic == "/airflow" and msg.payload == b"high":
                self.__ideo.boost()
            if subtopic == "/airflow/low":
                self.__ideo.setMinAirflow(int(float(msg.payload)))
            if subtopic == "/airflow/high":
                self.__ideo.setMaxAirflow(int(float(msg.payload)))
            if subtopic == "/config/dirty_filter_rpm":
                self.__ideo.setDirtyFilterRpmThreshold(int(float(msg.payload)))

            if not subtopic.startswith("/state"):
                self.__ideo.requestInsideTemp()
                self.__ideo.requestOutsideTemp()
                self.__ideo.requestStatus()
        except e:
            print("Exception while parsing MQTT payload: " + str(s))

    def onInsideTemperatureUpdate(self, inlet_temp, outlet_temp):
        self.__client.publish(self.__topic + "/state/inside_inlet", str(inlet_temp))
        self.__client.publish(self.__topic + "/state/outlet", str(outlet_temp))
        pass

    def onOutsideTemperatureUpdate(self, inlet_temp, outlet_temp):
        self.__client.publish(self.__topic + "/state/outside_inlet", str(inlet_temp))
        self.__client.publish(self.__topic + "/state/outlet", str(outlet_temp))
        pass

    def onStatusUpdate(self, airflow, bypass):
        self.__client.publish(self.__topic + "/state/airflow", airflow.name.lower())
        self.__client.publish(self.__topic + "/state/bypass", bypass.name.lower())
        pass
    
    def topic(self):
        return self.__topic
