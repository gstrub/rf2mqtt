from InOne import Channel

class Receiver:
    def __init__(self):
        pass
    def onEvent(self, event):
        pass

class FilterReceiver(Receiver):
    def __init__(self):
        self.__bindings = []
    def bind(self, id: int, channel = Channel.Any):
        self.__bindings.append({"id": id, "channel": channel})
    def onFilteredEvent(self, event):
        pass
    def onEvent(self, event):
        for b in self.__bindings:
            if b["id"] == event.id and \
                (b["channel"] == Channel.Any or b["channel"] ==  event.channel):
                self.onFilteredEvent(event)
                return
