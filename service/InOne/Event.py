from InOne import Channel, Command

class Event:
    def __init__(self, id, channel: Channel):
        self.id = id
        self.channel = Channel(channel)
    def __eq__(self, other):
        return type(self) == type(other) and \
            self.id == other.id and \
            self.channel == other.channel

class TurnEvent(Event):
    def __init__(self, id, channel: Channel, command: Command):
        super().__init__(id, channel)
        self.command = Command(command)
    def __eq__(self, other):
        return super().__eq__(other) and \
            self.command == other.command

class HoldEvent(TurnEvent):
    pass

class ReleaseEvent(Event):
    pass
