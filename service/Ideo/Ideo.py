from enum import Enum

class AirflowState(Enum):
    Low = 1
    High = 0
    Away = 2

class BypassState(Enum):
    Off = 0
    On = 8
    Forced = 9

class Schedule(Enum):
    Auto1 = 2
    Auto2 = 3
    Manual = 4

class Ideo:
    def __init__(self, out):
        self.__out = out
        self._lowSpeed = 90
        self._deviceId = 0
        self._listeners = []

    def _send(self, command, params):
        output = "{0},{1:02X},{2:08X}".format(self._deviceId, command, params)
        self.__out.write(output)

    # Bouton boost cuisine (vitesse max pendant 30 minutes)
    def boost(self):
        self._send(0x94,0)

    def setAwayMode(self, mode):
        if mode:
            self._send(0x41,1 << 16)
        else:
            self._send(0x41,0)

    def setDateTime(self, dayOfWeek, hours, minutes):
        self._send(0x3A, dayOfWeek + (hours << 8) + (minutes << 16))

    def setMinAirflow(self, airflow):
        self._send(0x3c, airflow << 16)

    def setMaxAirflow(self, airflow):
        self._send(0x3d, airflow << 16)

    def setSchedule(self, mode):
        self._send(0x3B, mode.value)

    def setDirtyFilterRpmThreshold(self, rpm):
        self._send(0x59, rpm << 16)

    def requestInsideTemp(self):
        self._send(0x31,0)

    def requestOutsideTemp(self):
        self._send(0x32,0)

    def requestStatus(self):
        self._send(0x33,0)

    def register(self, listener):
        self._listeners.append(listener)
            
    def parseIncomingMessage(self, message):
        message = message.strip(" \r\n")
        tokens = message.split(",")

        if (len(tokens) < 3): return

        command = int(tokens[1], 16)
        params = int(tokens[2], 16)

        if (params==0): return
        
        # Inside inlet (inside dirty air) temperature topic
        if command == 0x31:
            inlet_temp = (params >> 16) / 10.0 
            outlet_temp = (params & 0xFFFF) / 10.0
            for r in self._listeners:
                r.onInsideTemperatureUpdate(inlet_temp, outlet_temp)

        # Outside inlet (fresh air) temperature topic
        if command == 0x32:
            inlet_temp = (params >> 16) / 10.0
            outlet_temp = (params & 0xFFFF) / 10.0
            for r in self._listeners:
                r.onOutsideTemperatureUpdate(inlet_temp, outlet_temp)

        # Status topic
        if command == 0x33:
            airflow = AirflowState ((params >> 16) & 0xf)
            bypass = BypassState ((params >> 12) & 0xf)
            for r in self._listeners:
                r.onStatusUpdate(airflow, bypass)