from enum import Enum

class Command(Enum):
    Learn = 0
    On = 1
    Off = 2
    Hold = 3
    Release = 6

class State(Enum):
    Off = 'on'
    On = 'off'
    Hold = 'hold'

class Channel(Enum):
    Learn = 0
    Left = 1
    Right = 2
    Any = 'any'
