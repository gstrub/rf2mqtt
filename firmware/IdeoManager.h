#ifndef _IDEOMANAGER_H
#define _IDEOMANAGER_H

#include "CC1101.h"

namespace Ideo
{

  struct TxPacketData
  {
    char device;
    uint8_t command;
    char params[8];
  };

  struct RxPacketData
  {
    char device;
    uint8_t command;
    char params[8];
    int8_t rssi;
    uint8_t lqi;
  };

  class Manager
  {
  public:
    Manager(uint8_t ssPin, uint8_t irqPin);

    void begin(uint8_t channel = 0);

    bool isPacketAvailable() { return this->m_isPacketAvailable; }
    void getLastPacket(RxPacketData *packet);
    void sendPacket(TxPacketData *packet);

    bool commandResponse(TxPacketData *tx, RxPacketData *rx);

    void rfRxCallback();

    void detachRadio();
    void attachRadio();

    CC1101::Radio *radio() { return &m_radio; };

  protected:
    CC1101::Radio m_radio;
    uint8_t m_irqPin;
    RxPacketData m_lastRxPacket;
    bool m_isPacketAvailable;
    uint32_t m_commandResponseTimeout;
  };

  void printParams(const char *params);
  void printPacket(const RxPacketData *packet);
  uint8_t parseNibble(char param);
  uint16_t parseUint16(const char *param);
  void buildParams(char *params, uint32_t data);

  class Remote
  {
  public:
    Remote(Manager &manager);
  };

}; // namespace Ideo

#endif