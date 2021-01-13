#include <Arduino.h>
#include "InOneManager.h"
#include "bit_funcs.h"

using namespace InOne;

// CC1101 Rf settings for Legrand InOne protocol
CC1101::Configuration inOneRfSettings = {
    0x01, // IOCFG2        GDO2 Output Pin Configuration
    0x2E, // IOCFG1        GDO1 Output Pin Configuration
    0x80, // IOCFG0        GDO0 Output Pin Configuration
    0x07, // FIFOTHR       RX FIFO and TX FIFO Thresholds

    // Set sync word to 0x83E0 as in the InOne system
    0x83,             // SYNC1         Sync Word, High Byte
    0xE0,             // SYNC0         Sync Word, Low Byte
    c_rfRxPacketSize, // PKTLEN        Packet Length
    0x00,             // PKTCTRL1      Packet Automation Control
    0x00,             // PKTCTRL0      Packet Automation Control
    0x00,             // ADDR          Device Address
    0x00,             // CHANNR        Channel Number
    0x06,             // FSCTRL1       Frequency Synthesizer Control
    0x00,             // FSCTRL0       Frequency Synthesizer Control

    // Set frequency to 868.3 MHz
    0x21, // FREQ2         Frequency Control Word, High Byte
    0x65, // FREQ1         Frequency Control Word, Middle Byte
    0x6A, // FREQ0         Frequency Control Word, Low Byte

    // Set data rate to ~19200 baud, channel bandwidth 101kHz
    0xC9, // MDMCFG4       Modem Configuration
    0x83, // MDMCFG3       Modem Configuration
    // Use preamble + sync, 2-FSK modulation
    0x06, // MDMCFG2       Modem Configuration
    0x20, // MDMCFG1       Modem Configuration
    0xF8, // MDMCFG0       Modem Configuration

    // Set deviation to 25.4kHz
    0x40, // DEVIATN       Modem Deviation Setting
    0x07, // MCSM2         Main Radio Control State Machine Configuration
    0x0C, // MCSM1         Main Radio Control State Machine Configuration
    0x18, // MCSM0         Main Radio Control State Machine Configuration
    0x16, // FOCCFG        Frequency Offset Compensation Configuration
    0x6C, // BSCFG         Bit Synchronization Configuration
    0x03, // AGCCTRL2      AGC Control
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

namespace Manchester
{
  /* Decode an inverted manchester-encoded signal
   *  <length> is the length of the *decoded* signal, int *bits*
   *  in_offset is an offset in the *input* buffer, specificed in *decoded bits*
   */
  void Decode(uint8_t *in_buffer, uint8_t *out_buffer, uint16_t length, uint16_t in_offset, uint8_t *n_errors)
  {
    uint8_t _n_errors = 0;
    for (uint16_t b_ptr = 0; b_ptr < length; b_ptr++)
    {
      // Get two bits from input buffer
      uint8_t in_ptr = (b_ptr + in_offset) * 2;
      uint8_t in_data = get_bit(in_buffer, in_ptr, true) << 1 |
                        get_bit(in_buffer, in_ptr + 1, true);
      // Decode manchester-encoded bit
      uint8_t out_data = 0;
      if (in_data == 0b01)
        out_data = 0;
      else if (in_data == 0b10)
        out_data = 1;
      else
        _n_errors++;
      // Set decoded bit in output buffer
      def_bit(out_buffer, b_ptr, out_data);
    }
    if (n_errors != 0)
      *n_errors = _n_errors;
  }

  /** Encode a message using Manchester encoding
   *  <length> is the length of the message to be encoded, in *bits*
   *  <out_offset> is the offset to write to in the output buffer, in *bits*
   */

  void Encode(uint8_t *in_buffer, uint8_t *out_buffer, uint16_t length, uint16_t out_offset)
  {
    uint16_t out_ptr = out_offset;
    for (uint16_t i = 0; i < length; i++)
    {
      if (get_bit(in_buffer, i))
      {
        set_bit(out_buffer, out_ptr++, true);
        clr_bit(out_buffer, out_ptr++, true);
      }
      else
      {
        clr_bit(out_buffer, out_ptr++, true);
        set_bit(out_buffer, out_ptr++, true);
      }
    }
  }
} // namespace Manchester

/**
 * The Protocol used over the Manchester layer
 * It consists in framing each nibble with a high bit
 */
namespace LegrandProtocol
{
  /* Extract bytes from in_buffer (contains nibbles framed by '1' bits)
 *  length is in *bytes* to be decoded
 *  out_buffer only contains bytes !
 */
  void Decode(uint8_t *in_buffer, uint8_t *out_buffer, uint8_t length, uint8_t *n_errors)
  {
    uint8_t _n_errors = 0;
    for (uint8_t n_ptr = 0; n_ptr < length * 2; n_ptr++)
    {
      uint8_t in_ptr = n_ptr * 5;
      for (uint8_t i = 0; i < 5; i++)
      {
        uint8_t in_data = get_bit(in_buffer, (in_ptr + i));
        // First bit should always be 1
        if (i == 0)
        {
          if (in_data != 1)
            _n_errors++;
        }
        else
        {
          def_bit(out_buffer, n_ptr * 4 + (i - 1), in_data);
        }
      }
    }
    if (n_errors != 0)
      *n_errors = _n_errors;
  }

  void Encode(uint8_t *in_buffer, uint8_t *out_buffer, uint8_t length)
  {
    uint8_t out_ptr = 0;
    for (uint8_t i = 0; i < length * 8; i++)
    {
      // First bit before a nibble needs to be a '1'
      if (i % 4 == 0)
      {
        set_bit(out_buffer, out_ptr++);
      }
      def_bit(out_buffer, out_ptr++, get_bit(in_buffer, i));
    }
    // Set last bit
    set_bit(out_buffer, out_ptr++);
  }
} // namespace LegrandProtocol

Manager::Manager(uint8_t ssPin, uint8_t irqPin) : m_radio(ssPin, 255, irqPin), // Not using GDO0
                                                  m_isPacketAvailable(false),
                                                  m_irqPin(irqPin),
                                                  m_rxBufferCount(0),
                                                  m_isRawDataAvailable(false),
                                                  m_debugLevel(0)
{
}

void Manager::begin()
{
  // Initialize CC1101 module
  this->m_radio.begin();
  this->m_radio.writeConfiguration(&inOneRfSettings);
  uint8_t patable_arr[8] = {0xC0, 0, 0, 0, 0, 0, 0, 0};
  this->m_radio.writePaTable(patable_arr, 8);
  // Put radio in receive mode
  this->m_radio.goReceive();
}

void Manager::rfRxCallback()
{
  this->m_isRawDataAvailable = false;
  uint8_t count = this->m_radio.getNumRxBytes();
  this->m_radio.readRxFifo(this->m_rxBuffer + this->m_rxBufferCount, count);
  this->m_rxBufferCount += count;

  if (this->m_rxBufferCount >= c_rfRxPacketSize)
  {
    this->m_isRawDataAvailable = true;
    this->m_rxBufferCount = 0;
    this->m_radio.writeStrobe(CC1101::StrobeCommand::SFRX);
    this->m_radio.goReceive();
  }

  this->m_lastRxTime = millis();
}

bool Manager::isPacketAvailable()
{
  //return false;
  bool returnValue = false;
  if (this->m_isPacketAvailable)
    returnValue = true;
  else if (this->m_isRawDataAvailable)
  {
    this->m_isRawDataAvailable = false;
    /** A raw packet has been received, try to decode it to check for validity
     *  This function will be called from the main loop before accessing the packet data
     *  and it is preferable to perform decoding here than in the interrupt handler  */
    uint8_t manDecBuffer[12];
    uint8_t decodeErrorCount = 0;

    // Decode the manchester-encoded data stream
    Manchester::Decode(this->m_rxBuffer, manDecBuffer, 60, 2, &decodeErrorCount);
    if (decodeErrorCount != 0)
    {
      if (m_debugLevel)
      {
        Serial.print(F("Manchester decoding errors: "));
        Serial.println(decodeErrorCount);
      }
      goto Epilogue;
    }

    uint8_t legDecBuffer[9];
    uint8_t length = 6;
    // Try to decode the first 6 bytes of the InOne message (we do not know the actual length yet !)
    LegrandProtocol::Decode(manDecBuffer, legDecBuffer, length, &decodeErrorCount);
    if (decodeErrorCount != 0)
    {
      if (m_debugLevel)
      {
        Serial.print(F("Framing errors: "));
        Serial.println(decodeErrorCount);
      }
      goto Epilogue;
    }

    // Compute the number of extra bytes
    uint8_t extraByteCount = 0;
    switch ((legDecBuffer[4] & 0xC0) >> 6)
    {
    case 0:
      break;
    case 1:
      extraByteCount = 1;
      break;
    case 2:
      extraByteCount = 3;
      break;
    default:
      if (m_debugLevel)
      {
        Serial.println(F("Incorrect extra byte count !"));
      }
      goto Epilogue;
    }
    if (extraByteCount)
    {
      // Decode the extra message bytes
      Manchester::Decode(this->m_rxBuffer, manDecBuffer, 10 * extraByteCount, 62, &decodeErrorCount);
      if (decodeErrorCount != 0)
      {
        if (m_debugLevel)
        {
          Serial.print(F("Manchester decoding errors in extra byte processing: "));
          Serial.println(decodeErrorCount);
        }
        goto Epilogue;
      }
      LegrandProtocol::Decode(manDecBuffer, &legDecBuffer[6], extraByteCount, &decodeErrorCount);
      if (decodeErrorCount != 0)
      {
        if (m_debugLevel)
        {
          Serial.print(F("Framing errors in extra byte processing: "));
          Serial.println(decodeErrorCount);
        }
        goto Epilogue;
      }
      length += extraByteCount;
    }

    // Compute and check the packet checksum
    uint8_t checksum = Packet::checksum(legDecBuffer, length - 1);
    if (checksum != legDecBuffer[length - 1])
    {
      if (m_debugLevel)
      {
        Serial.print("Checksum fail. RX: ");
        Serial.print(legDecBuffer[length - 1], HEX);
        Serial.print(", calc: ");
        Serial.println(checksum, HEX);
      }
      goto Epilogue;
    }
    if (m_debugLevel > 1)
    {
      Serial.print("Raw Data: ");
      for (uint8_t i = 0; i < length; i++)
      {
        Serial.print(legDecBuffer[i], HEX);
        Serial.print(' ');
      }
      Serial.println();
    }
    // Convert raw data to packet
    Packet::fromRaw(&this->m_lastRxPacket, legDecBuffer, length);
    this->m_isPacketAvailable = true;
    returnValue = true;
  }

Epilogue:
  // If last received data was more than 600ms ago, reset the packet receiver
  if (millis() - this->m_lastRxTime > 600)
  {
    this->m_rxBufferCount = 0;
    this->m_radio.writeStrobe(CC1101::StrobeCommand::SFRX);
    this->m_radio.goReceive();
    this->m_lastRxTime = millis();
  }

  return returnValue;
}

void Manager::getLastPacket(Packet *packet)
{
  memcpy(packet, &this->m_lastRxPacket, sizeof(Packet));
  this->m_isPacketAvailable = false;
}

void Manager::sendPacket(Packet *packet)
{
  if (m_debugLevel > 1)
    packet->print();

  /* Messages to be transmitted are between 6 and 9 bytes long */
  uint8_t rawData[9];
  uint8_t length = packet->toRaw(rawData);

  if (m_debugLevel > 1)
  {
    Serial.print("Raw Data: ");
    for (uint8_t i = 0; i < length; i++)
    {
      Serial.print(rawData[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }

  // Frame each nibble in the packet with high bits
  uint8_t legEncData[12];
  LegrandProtocol::Encode(rawData, legEncData, length);

  if (m_debugLevel > 2)
  {
    Serial.print("Framed Data: ");
    for (uint8_t i = 0; i < length * 10 / 8; i++)
    {
      Serial.print(legEncData[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }

  // Encode the radio data with Manchester encoding
  uint8_t manEncData[64];
  memset(manEncData, 0, 64);
  // Position the first nibble of the manchester-encoded data to the 5th nibble of the sync word 83E0F
  manEncData[0] = 0xF0;
  // Manchester-encode the framed data, start outputting at the 5th bit (because of the sync nibble)
  Manchester::Encode(legEncData, manEncData, length * 10 + 1, 4);

  /** In order to improve link relability, the message is transmitted multiple times over the air
   *  Because the nibble count is odd, we cannot just repeat the previous message bytes, so we insert
   *  a second sync word and manchester-encoded data */
  uint8_t nibbleCount = (length * 10 + 1) / 2 + 1;

  // Insert sync word
  setNibble(manEncData, nibbleCount + 0, 0x8, true);
  setNibble(manEncData, nibbleCount + 1, 0x3, true);
  setNibble(manEncData, nibbleCount + 2, 0xE, true);
  setNibble(manEncData, nibbleCount + 3, 0x0, true);
  setNibble(manEncData, nibbleCount + 4, 0xF, true);
  nibbleCount += 5;
  // Insert manchester-encoded data again
  Manchester::Encode(legEncData, manEncData, length * 10 + 1, nibbleCount * 4);
  nibbleCount += (length * 10 + 1) / 2;

  if (m_debugLevel > 2)
  {
    Serial.println("Data to send: ");
    for (uint8_t i = 0; i < nibbleCount / 2; i++)
    {
      Serial.print(manEncData[i], HEX);
      Serial.print(' ');
    }
  }

  // Send packet using the CC1101 radio
  this->m_radio.writeRegister(CC1101::Register::MDMCFG2, 0x2);
  this->m_radio.writeRegister(CC1101::Register::PKTLEN, nibbleCount);
  this->m_radio.writeTxFifo((uint8_t *)&manEncData, nibbleCount / 2);
  this->m_radio.writeTxFifo((uint8_t *)&manEncData, nibbleCount / 2);
  this->m_radio.goTransmit();

  // Restore RX mode settings and go to Receive mode
  this->m_radio.writeRegister(CC1101::Register::MDMCFG2, 0x6);
  this->m_radio.writeRegister(CC1101::Register::PKTLEN, c_rfRxPacketSize);
  this->m_radio.goReceive();
}

void Manager::detachRadio()
{
}

void Manager::attachRadio()
{
  this->begin();
}