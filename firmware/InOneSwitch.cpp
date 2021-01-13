#include "InOneSwitch.h"

using namespace InOne;

Switch::Switch(uint32_t id, Manager *manager) : m_manager(manager),
                                                m_sequence(0),
                                                m_learnChannel(Channel::Learn)
{
  m_packet.id = id;
  m_packet.isLearnMode = false;
  isLearning = false;
}

void Switch::turnOn(Channel channel)
{
  channelShortPress(channel, Command::On);
}

void Switch::turnOff(Channel channel)
{
  channelShortPress(channel, Command::Off);
}

void Switch::channelShortPress(Channel channel, Command command)
{
  if (isLearning)
  {
    if (m_learnChannel == Channel::Learn || m_learnChannel == channel)
    {
      this->m_learnChannel = channel;
      this->longMessage(channel, Command::Learn, 6, (uint8_t)command, 0);
    }
    else
      this->mediumMessage(channel, Command::Learn, 0);
  }
  else
    this->shortMessage(channel, command);
}

void Switch::startLearn()
{
  if (!isLearning)
  {
    this->m_packet.isLearnMode = true;
    isLearning = true;
    m_learnChannel = Channel::Learn;
    mediumMessage(Channel::Learn, Command::Learn, 0);
  }
}

void Switch::stopLearn()
{
  // Get out of learning mode (if we were in learning mode)
  if (isLearning)
  {
    isLearning = false;
    // If at least one button was pressed, send the exit learning mode message
    if (m_learnChannel != Channel::Learn)
    {
      // This must be true for this command: we are still in learing mode when exiting this mode !
      this->m_packet.isLearnMode = true;
      mediumMessage(Channel::Learn, Command::Learn, 0x7);
    }
  }
  this->m_packet.isLearnMode = false;
}

void Switch::updateSequence()
{
  m_packet.sequenceIndex = (m_sequence >> 2 * (uint8_t)m_packet.channel) & 0x3;
  uint8_t sequenceIndex = (m_packet.sequenceIndex + 1) & 0x3;
  // Do not change sequence index for learning mode enter and exit packets
  if (m_packet.channel == Channel::Learn && m_packet.command == Command::Learn && m_packet.data[0] == 0)
    sequenceIndex = m_packet.sequenceIndex;
  m_sequence &= ~(0x3 << (2 * (uint8_t)m_packet.channel));
  m_sequence |= sequenceIndex << (2 * (uint8_t)m_packet.channel);
}

void Switch::shortMessage(Channel channel, Command command)
{
  this->m_packet.channel = channel;
  this->m_packet.command = command;
  this->m_packet.type = PacketType::Short;
  this->updateSequence();
  this->m_manager->sendPacket(&this->m_packet);
}

void Switch::mediumMessage(Channel channel, Command command, uint8_t data)
{
  this->m_packet.channel = channel;
  this->m_packet.command = command;
  this->m_packet.type = PacketType::Medium;
  this->m_packet.data[0] = data;
  this->updateSequence();
  this->m_manager->sendPacket(&this->m_packet);
}

void Switch::longMessage(Channel channel, Command command, uint8_t data0, uint8_t data1, uint8_t data2)
{
  this->m_packet.channel = channel;
  this->m_packet.command = command;
  this->m_packet.type = PacketType::Long;
  this->m_packet.data[0] = data0;
  this->m_packet.data[1] = data1;
  this->m_packet.data[2] = data2;
  this->updateSequence();
  this->m_manager->sendPacket(&this->m_packet);
}
