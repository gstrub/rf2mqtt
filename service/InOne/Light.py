from InOne.Enums import Channel, State, Command
from InOne.Event import TurnEvent, ReleaseEvent, HoldEvent
from InOne.Receiver import FilterReceiver
from InOne.Switch import Switch

import threading

"""
Light
A standard on/off light
"""
class Light(FilterReceiver):
    def __init__(self):
        super().__init__()
        self.__state = State.Off
        self.__switch = None
        self.__channel = None

    # Define the controlling switch for this Light
    # The controlling switch is used for sending commands via the interface
    def bindControl(self, switch: Switch, channel: Channel):
        self.__switch = switch
        self.__channel = channel
        self.bind(switch.id(), channel)

    def onFilteredEvent(self, event):
        # ReleaseEvent does nothing
        if type(event) is ReleaseEvent:
            return
        # Treat TurnEvent and HoldEvent similarly
        if event.command == Command.On:
            self.setState(State.On)
        if event.command == Command.Off:
            self.setState(State.Off)

    def state(self):
        return self.__state

    def setState(self, state: State):
        self.__state = state

    # Emit an 'On' command via the interface
    # TODO: should we crash / throw if there is no controlling switch ?
    def turnOn(self):
        if self.__switch is not None:
            self.__switch.turnOn(self.__channel)
        self.setState(State.On)

    # Emit an 'Off' command via the interface
    # TODO: should we crash / throw if there is no controlling switch ?
    def turnOff(self):
        if self.__switch is not None:
            self.__switch.turnOff(self.__channel)
        self.setState(State.Off)

    def switch(self):
        return self.__switch

    def channel(self):
        return self.__channel

"""
TimerLight
A standard on/off light with an auto-off timer
The duration of the timer is expressed in seconds
A TimerLight can be held on, bypassing the timer
"""
class TimerLight(Light):
    def __init__(self, duration = 0):
        Light.__init__(self)
        self.__duration = duration
        self.__timer = None

    # (override) Capture HoldEvents to set the light to Hold state
    def onFilteredEvent(self, event):
        # Capture HoldEvent with On command
        if type(event) is HoldEvent and event.command == Command.On:
            self.setState(State.Hold)
        else:
            # Pass all other events to Light
            super().onFilteredEvent(event)

    # Hold the light on
    def holdOn(self):
        if self.switch() is not None:
            self.switch().turnOn(self.channel())
        self.setState(State.Hold)

    # (override) If the light is held on, do nothing
    def turnOn(self):
        if self.state() != State.Hold:
            super().turnOn()

    def setDuration(self, duration):
        self.__duration = duration
        # If the light was on, cancel the existing timer
        # and restart a timer with the new duration
        if self.state() == State.On:
            self.__startTimer()

    def setState(self, state: State):
        # The light got an 'on' command
        # Create or restart the auto-off timer
        if state == State.On:
            self.__startTimer()
        
        # The light got an 'off' command
        # Cancel the timer
        if state == State.Off:
            self.__stopTimer()

        # Call parent class explicitly
        Light.setState(self, state)

    # (private) starts a timer, if duration > 0
    # If a timer was already running, it is stopped and restarted
    # This behaves like a retriggerable monostable
    def __startTimer(self):
        self.__stopTimer()
        if self.__duration > 0:
                print("Starting " + str(self.__duration) + "s timer...")
                self.__timer = threading.Timer(self.__duration, self.turnOff)
                self.__timer.start()

    def __stopTimer(self):
        if self.__timer is not None:
                print("Canceling timer")
                self.__timer.cancel()
        

