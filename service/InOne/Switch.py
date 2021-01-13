from InOne.Enums import Channel, Command
from InOne.Event import TurnEvent
from InOne.Dispatcher import Dispatcher

class OutputInterface:
    def __init__(self):
        pass
    def write(self, message):
        pass

class Switch:
    """
    InOne.Switch:
    Implement a virtual switch: generates RF output via interface 'out',
    notifies dispatcher of sent commands
    """
    def __init__(self, id, out=OutputInterface(), dispatcher=Dispatcher()): 
        if type(id) != int or id < 0 or id > 0xFFFFF:
            raise ValueError("'id' must be a positive integer in the [0, 0xFFFFFF] range.")
        self.__sequenceNumber = 0
        self.__learn = False
        self.__id = id
        self.__out = out
        self.__dispatcher = dispatcher

    def __shortPress(self, channel: Channel, command: Command):
        output = str(self.__sequenceNumber) + ',' + \
            str(self.__id) + ',' + str(channel.value) + ',' + str(command.value)
        self.__out.write(bytes(output, 'utf8'))
        self.__out.write(bytes(output, 'utf8'))
        self.__incrementSequenceCounter()

    def __incrementSequenceCounter(self):
        self.__sequenceNumber = (self.__sequenceNumber + 1) % 4

    def id(self):
        return self.__id

    def turnOn(self, channel: Channel):
        self.__shortPress(channel, Command.On)
        self.__dispatcher.onEvent(TurnEvent(self.__id, channel, Command.On))

    def turnOff(self, channel: Channel):
        self.__shortPress(channel, Command.Off)
        self.__dispatcher.onEvent(TurnEvent(self.__id, channel, Command.Off))