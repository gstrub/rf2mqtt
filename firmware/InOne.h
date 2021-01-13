#ifndef _INONE_H
#define _INONE_H

#include <stdint.h>

namespace InOne
{

  enum class Command : uint8_t
  {
    Learn = 0,
    On,
    Off,
    DimStart,
    DimStop = 6
  };

  enum class Channel : uint8_t
  {
    Learn = 0,
    Left = 1,
    Right = 2
  };

  enum class PacketType : uint8_t
  {
    Short = 0,
    Medium = 1,
    Long = 2
  };

  class Packet
  {
  public:
    uint8_t sequenceIndex;
    uint32_t id;
    PacketType type;
    Command command;
    Channel channel;
    uint8_t data[3];
    //uint8_t rssi;
    //uint8_t lqi;
    bool isLearnMode;

    static uint8_t checksum(uint8_t *rawData, uint8_t length);
    uint8_t toRaw(uint8_t *rawData);
    static void fromRaw(Packet *packet, uint8_t *rawData, uint8_t length);
    void print();
  };

} // namespace InOne

#endif //_INONE_H
