from InOne.Enums import Command, Channel
from InOne.Event import TurnEvent, HoldEvent, ReleaseEvent

class MessageParser:
    def __init__(self, dispatcher):
        self.__dispatcher = dispatcher
    def parse(self, message: str):
        message = message.strip(" \r\n")
        tokens = message.split(",")
        #tokens = re.split('[\W]+', message)
        if len(tokens) >= 4:
            id = int(tokens[1])
            channel = Channel(int(tokens[2]))
            command = Command(int(tokens[3]))
        if len(tokens) == 4:
            if (command == Command.Release):
                self.__dispatcher.onEvent(ReleaseEvent(id, channel))
            else:
                self.__dispatcher.onEvent(TurnEvent(id, channel, command))
        if len(tokens) == 8:
            if command == Command.Hold:
                if tokens[5] == '127':
                    holdCommand = Command.On
                if tokens[5] == '128':
                    holdCommand = Command.Off
                self.__dispatcher.onEvent(HoldEvent(id, channel, holdCommand))
