#include <Arduino.h>
#include "IdeoManager.h"

using namespace Ideo;

// Rf settings for CC1101
CC1101::Configuration ideoRfSettings = {
    0x01, // IOCFG2        GDO2 Output Pin Configuration
    0x2E, // IOCFG1        GDO1 Output Pin Configuration
    0x80, // IOCFG0        GDO0 Output Pin Configuration
    0x07, // FIFOTHR       RX FIFO and TX FIFO Thresholds

    // Set sync word to 0x2d00 as in the CMV system
    0x2D, // SYNC1         Sync Word, High Byte
    0x00, // SYNC0         Sync Word, Low Byte
    0x10, // PKTLEN        Packet Length
    0x04, // PKTCTRL1      Packet Automation Control
    0x00, // PKTCTRL0      Packet Automation Control
    0x00, // ADDR          Device Address
    0x00, // CHANNR        Channel Number
    0x06, // FSCTRL1       Frequency Synthesizer Control
    0x00, // FSCTRL0       Frequency Synthesizer Control

    // Set frequency to 868.3352 MHz
    0x21, // FREQ2         Frequency Control Word, High Byte
    0x65, // FREQ1         Frequency Control Word, Middle Byte
    0xC3, // FREQ0         Frequency Control Word, Low Byte

    // Set data rate to ~9600 baud, channel bandwidth 406.25kHz
    0x48, // MDMCFG4       Modem Configuration
    0x83, // MDMCFG3       Modem Configuration
    // Use preamble + sync, 2-FSK modulation
    0x01, // MDMCFG2       Modem Configuration
    0x02, // MDMCFG1       Modem Configuration
    0xF8, // MDMCFG0       Modem Configuration

    // Set deviation to 76.171875kHz (req: 75kHz)
    0x54, // DEVIATN       Modem Deviation Setting
    0x07, // MCSM2         Main Radio Control State Machine Configuration
    0x0F, // MCSM1         Main Radio Control State Machine Configuration
    0x18, // MCSM0         Main Radio Control State Machine Configuration
    0x16, // FOCCFG        Frequency Offset Compensation Configuration
    0x6C, // BSCFG         Bit Synchronization Configuration
    0x07, // AGCCTRL2      AGC Control
    0x40, // AGCCTRL1      AGC Control
    0x91, // AGCCTRL0      AGC Control
    0x87, // WOREVT1       High Byte Event0 Timeout
    0x6B, // WOREVT0       Low Byte Event0 Timeout
    0x09, // WORCTRL       Wake On Radio Control
    0x56, // FREND1        Front End RX Configuration
    0x10, // FREND0        Front End TX Configuration
    0xE9, // FSCAL3        Frequency Synthesizer Calibration
    0x2A, // FSCAL2        Frequency Synthesizer Calibration
    0x00, // FSCAL1        Frequency Synthesizer Calibration
    0x1F, // FSCAL0        Frequency Synthesizer Calibration
    0x41, // RCCTRL1       RC Oscillator Configuration
    0x00, // RCCTRL0       RC Oscillator Configuration
    0x59, // FSTEST        Frequency Synthesizer Calibration Control
    0x7F, // PTEST         Production Test
    0x3F, // AGCTEST       AGC Test
    0x81, // TEST2         Various Test Settings
    0x35, // TEST1         Various Test Settings
    0x09, // TEST0         Various Test Settings
};

struct RawPacket
{
  uint16_t header;
  char device;
  uint8_t command;
  char params[8];
  uint16_t checksum;
  uint16_t footer;
};

struct RawRxPacket
{
  uint16_t header;
  char device;
  uint8_t command;
  char params[8];
  uint16_t checksum;
  uint16_t footer;
  int8_t rssi;
  uint8_t lqi;
};

static const char nibbleLut[] = "0123456789ABCDEF";

uint16_t computeChecksum(const RawPacket *pkt)
{
  uint8_t chk = 0;
  chk += pkt->header >> 8;
  chk += pkt->header & 0xff;
  chk += pkt->device;
  chk += pkt->command;
  for (uint8_t i = 0; i < 8; i++)
    chk += pkt->params[i];
  return (nibbleLut[chk >> 4]) | (nibbleLut[chk & 0xF] << 8);
}

Manager::Manager(uint8_t ssPin, uint8_t irqPin) : m_radio(ssPin, 255, irqPin), // Not using GDO0
                                                  m_commandResponseTimeout(250)
{
}

void Manager::begin(uint8_t channel)
{
  // Initialize CC1101 module
  this->m_radio.begin();
  this->m_radio.writeConfiguration(&ideoRfSettings);
  // Communication channel (as set on the devices) is the low sync byte
  this->m_radio.writeRegister(CC1101::Register::SYNC0, channel);
  // Put radio in receive mode
  this->m_radio.goReceive();
}

void Manager::getLastPacket(RxPacketData *packet)
{
  memcpy(packet, &this->m_lastRxPacket, sizeof(RxPacketData));
  this->m_isPacketAvailable = false;
}

void Manager::rfRxCallback()
{
  RawRxPacket rxPacket;
  this->m_radio.readRxFifo((uint8_t *)&rxPacket, sizeof(RawRxPacket));

  if (rxPacket.header == 0x3001 &&
      rxPacket.footer == 0x0003 &&
      rxPacket.checksum == computeChecksum((RawPacket *)&rxPacket))
  {
    this->m_isPacketAvailable = true;
    memcpy(&this->m_lastRxPacket, &rxPacket.device, 10);
    this->m_lastRxPacket.rssi = CC1101::rssiToDbm(rxPacket.rssi);
    this->m_lastRxPacket.lqi = rxPacket.lqi & 0x7F;
  }

  if (this->m_radio.isRxOverflow())
    this->m_radio.writeStrobe(CC1101::StrobeCommand::SFRX);

  this->m_radio.goReceive();
}

void Manager::sendPacket(TxPacketData *packet)
{
  RawPacket txPacket;
  txPacket.header = 0x3001;
  txPacket.footer = 0x0003;
  memcpy(&txPacket.device, packet, 10);
  txPacket.checksum = computeChecksum(&txPacket);

  if (this->m_radio.getNumTxBytes())
    this->m_radio.writeStrobe(CC1101::StrobeCommand::SFTX);

  this->m_radio.writeTxFifo((uint8_t *)&txPacket, sizeof(RawPacket));

  this->m_radio.writeStrobe(CC1101::StrobeCommand::STX);

  // Radio goes automatically in RX mode after transmitting
  // Ensure we have finished transmitting before returning
  while (this->m_radio.getState() != CC1101::ControlState::RX)
    ;
}

bool Manager::commandResponse(TxPacketData *tx, RxPacketData *rx)
{
  this->sendPacket(tx);
  uint32_t packetSendTime = millis();

  while (!this->isPacketAvailable() && millis() - packetSendTime < m_commandResponseTimeout)
    ;
  if (this->isPacketAvailable())
  {
    this->getLastPacket(rx);
    return true;
  }
  Serial.println("Wait for response timed out.");
  return false;
}

uint8_t Ideo::parseNibble(char param)
{
  uint8_t nib;
  if (param >= '0' && param <= '9')
    nib = param - '0';
  else if (param >= 'A' && param <= 'F')
    nib = param - 'A' + 10;
  else if (param >= 'a' && param <= 'f')
    nib = param - 'a' + 10;
  else
    nib = 0;
  return nib;
}

uint16_t Ideo::parseUint16(const char *param)
{
  uint16_t retval = 0;
  for (uint8_t i = 0; i < 4; i++)
  {
    uint8_t nib;
    nib = parseNibble(param[i]);
    retval |= nib << (12 - 4 * i);
  }
  return retval;
}

void Ideo::printParams(const char *params)
{
  for (int i = 0; i < 8; i++)
    Serial.print(params[i]);
}

void Ideo::buildParams(char *params, uint32_t data)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    params[i] = nibbleLut[(data >> (28 - 4 * i)) & 0xF];
  }
}

void Ideo::printPacket(const RxPacketData *packet)
{
  uint16_t param_1 = parseUint16(packet->params);
  uint16_t param_2 = parseUint16(&packet->params[4]);
  Serial.print("Device: ");
  Serial.println(packet->device);
  Serial.print("Command: ");
  Serial.println(packet->command, HEX);
  switch (packet->command)
  {
  case 0x31:
    Serial.println("Get Outlet? Temperature");
    break;

  case 0x32:
    Serial.println("Get Outside Temperature");
    break;

  case 0x33:
    Serial.println("Get Status");
    break;

  case 0x3A:
    Serial.println("Set Date/Time");
    if ((param_2 & 0xFF) != 0xFF)
    {
      Serial.print("Day ");
      Serial.print(param_2 & 0xff);
      Serial.print(' ');
      Serial.print(param_2 >> 8);
      Serial.print(':');
      Serial.println(param_1 & 0xff);
    }
    break;

  case 0x3B:
    Serial.println("Set Schedule");
    if (param_2 == 0)
    {
      Serial.print("Value: ");
      Serial.println(param_1);
    }
    break;

  case 0x3C:
    Serial.println("Set Low Fan Speed");
    if (param_2 == 0)
    {
      Serial.print("Value: ");
      Serial.println(param_1);
    }
    break;

  case 0x3D:
    Serial.println("Set High Fan Speed");
    if (param_2 == 0)
    {
      Serial.print("Value: ");
      Serial.println(param_1);
    }
    break;

  case 0x40:
    Serial.println("Set Force Bypass");
    if (param_2 < 25)
    {
      Serial.print("Value: ");
      Serial.println(param_1);
      Serial.print("Duration (h): ");
      Serial.print(param_2);
    }
    break;

  case 0x41:
    Serial.println("Set Holiday mode");
    if (param_2 == 0)
    {
      Serial.print("Value: ");
      Serial.println(param_1);
    }
    break;

  case 0x42:
    Serial.println("Set Bypass");
    if (param_2 <= 325)
    {
      Serial.print("Value: ");
      Serial.println(param_1);
      Serial.print("Fan speed: ");
      Serial.print(param_2);
    }
    break;

  case 0x58:
    Serial.println("Set Contact Polarity");
    break;

  case 0x59:
    Serial.println("Dirty filter alarm threshold");
    if (param_2 == 0)
    {
      Serial.print("Value (RPM): ");
      Serial.println(param_1);
    }
    break;

  default:

    break;
  }

  Serial.print("Params: ");
  Ideo::printParams(packet->params);
  Serial.println();

  Serial.print("RSSI: ");
  Serial.print(packet->rssi);
  Serial.println(" dBm");

  Serial.print("LQI: ");
  Serial.println(packet->lqi);
}

void Manager::attachRadio()
{
  // Re-Initialize CC1101 module via hot reset
  this->m_radio.writeStrobe(CC1101::StrobeCommand::SRES);
  while (this->m_radio.getState() != CC1101::ControlState::IDLE)
    ;

  this->m_radio.writeConfiguration(&ideoRfSettings);
  // Communication channel (as set on the devices) is the low sync byte
  this->m_radio.writeRegister(CC1101::Register::SYNC0, 0);
  // Put radio in receive mode
  this->m_radio.goReceive();
}

void Manager::detachRadio()
{
}