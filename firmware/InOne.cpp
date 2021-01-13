#include <Arduino.h>
#include "InOne.h"

using namespace InOne;

//https://stackoverflow.com/questions/29214301/ios-how-to-calculate-crc-8-dallas-maxim-of-nsdata
uint8_t Packet::checksum(uint8_t *rawData, uint8_t length)
{
  uint8_t crc = 0;
  for (uint8_t i = 0; i < length; ++i)
  {
    uint8_t inbyte = rawData[i];
    for (uint8_t j = 0; j < 8; ++j)
    {
      uint8_t mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix)
        crc ^= 0x8C;
      inbyte >>= 1;
    }
  }
  return crc;
}

uint8_t Packet::toRaw(uint8_t *rawData)
{
  rawData[0] = 0x40 | this->sequenceIndex;
  rawData[1] = (this->id & 0xFF000) >> 12;
  rawData[2] = (this->id & 0xFF0) >> 4;
  rawData[3] = (this->id & 0xF) << 4;
  rawData[3] |= (uint8_t)this->channel;
  rawData[4] = (uint8_t)this->command;

  if (this->isLearnMode)
    rawData[4] |= 0x30;

  uint8_t length = 6;
  switch (this->type)
  {
  case PacketType::Medium:
    rawData[4] |= 0x40;
    rawData[5] = this->data[0];
    length = 7;
    break;

  case PacketType::Long:
    rawData[4] |= 0x80;
    memcpy(&rawData[5], this->data, 3);
    length = 9;
    break;
  }

  rawData[length - 1] = checksum(rawData, length - 1);
  return length;
}

void Packet::fromRaw(Packet *packet, uint8_t *rawData, uint8_t length)
{
  packet->sequenceIndex = rawData[0] & 0xF;
  packet->id = ((uint32_t)rawData[1] << 12) & 0xFF000 | (rawData[2] << 4) & 0xFF0 | (rawData[3] >> 4);
  packet->channel = (InOne::Channel)(rawData[3] & 0xF);
  packet->command = (InOne::Command)(rawData[4] & 0xF);
  switch (length)
  {
  case 6:
    packet->type = PacketType::Short;
    break;
  case 7:
    packet->type = PacketType::Medium;
    packet->data[0] = rawData[5];
    break;
  case 9:
    packet->type = PacketType::Long;
    memcpy(packet->data, &rawData[5], 3);
    break;
  }
  if (rawData[4] & 0x30)
    packet->isLearnMode = true;
  else
    packet->isLearnMode = false;
}

void Packet::print()
{
  Serial.print("Sequence index: ");
  Serial.println(this->sequenceIndex);
  Serial.print("Switch ID: ");
  Serial.println(this->id, DEC);
  Serial.print("Channel: ");
  switch (this->channel)
  {
  case Channel::Learn:
    Serial.println("LEARN ");
    break;
  case Channel::Left:
    Serial.println("LEFT ");
    break;
  case Channel::Right:
    Serial.println("RIGHT ");
    break;
  }
  Serial.print("Command: ");
  switch (this->command)
  {
  case Command::Learn:
    Serial.println("LEARN ");
    break;
  case Command::On:
    Serial.println("ON ");
    break;
  case Command::Off:
    Serial.println("OFF ");
    break;
  case Command::DimStart:
    Serial.println("STARTVAR ");
    break;
  case Command::DimStop:
    Serial.println("STOPVAR ");
    break;
  }
  Serial.print("Packet Type: ");
  switch (this->type)
  {
  case PacketType::Short:
    Serial.println("SHORT ");
    break;
  case PacketType::Medium:
    Serial.println("MEDIUM ");
    break;
  case PacketType::Long:
    Serial.println("LONG ");
    break;
  }

  if (this->isLearnMode)
  {
    Serial.print("Learning mode: ");
    if (this->data[0] == 0x7)
      Serial.println("exiting.");
    else if (this->data[0] == 0x6)
    {
      Serial.print("command ");
      Serial.println(this->data[1]);
    }
    else
      Serial.println("entering.");
  }

  switch (this->type)
  {
  case PacketType::Medium:
    Serial.print("Data: ");
    Serial.println(this->data[0]);
    break;
  case PacketType::Long:
    Serial.print("Data: ");
    Serial.print(this->data[0], DEC);
    Serial.print(' ');
    Serial.print(this->data[1], DEC);
    Serial.print(' ');
    Serial.println(this->data[2], DEC);
    break;
  }
}
