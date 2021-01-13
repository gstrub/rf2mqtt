class Dispatcher:
    def __init__(self):
        self.__receivers = []
    def register(self, receiver):
        self.__receivers.append(receiver)
    def onEvent(self, event):
        for r in self.__receivers:
            r.onEvent(event)