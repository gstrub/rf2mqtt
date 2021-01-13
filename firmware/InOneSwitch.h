#ifndef _INONESWITCH_H
#define _INONESWITCH_H

#include "InOne.h"
#include "InOneManager.h"

namespace InOne
{

  class Switch
  {
  public:
    Switch(uint32_t id, Manager *manager);

    void turnOn(Channel channel);
    void turnOff(Channel channel);

    void startLearn();
    void stopLearn();

    bool isLearnMode() { return this->m_packet.isLearnMode; };

    void startDim(Channel channel, int8_t value);
    void stopDim(Channel channel, int8_t value);

  private:
    void updateSequence();

    void channelShortPress(Channel channel, Command command);
    void shortMessage(Channel channel, Command command);
    void mediumMessage(Channel channel, Command command, uint8_t data);
    void longMessage(Channel channel, Command command, uint8_t data0, uint8_t data1, uint8_t data2);

    bool isLearning;
    Packet m_packet;
    uint8_t m_sequence;
    Channel m_learnChannel;
    Manager *m_manager;
  };

} // namespace InOne

#endif //_INONESWITCH_H
