#ifndef _INONEMANAGER_H
#define _INONEMANAGER_H

#include "InOne.h"
#include "CC1101.h"

namespace InOne
{

  const uint8_t c_rfRxPacketSize = 60;

  class Manager
  {
  public:
    Manager(uint8_t ssPin, uint8_t irqPin);

    void begin();

    bool isPacketAvailable();
    void getLastPacket(Packet *packet);
    void sendPacket(Packet *packet);

    void rfRxCallback();

    void detachRadio();
    void attachRadio();

    CC1101::Radio *radio() { return &this->m_radio; };

  protected:
    CC1101::Radio m_radio;
    uint8_t m_irqPin;
    Packet m_lastRxPacket;
    bool m_isPacketAvailable;
    uint8_t m_rxBuffer[c_rfRxPacketSize];
    uint8_t m_rxBufferCount;
    bool m_isRawDataAvailable;
    uint32_t m_lastRxTime;
    uint8_t m_debugLevel;
  };

} // namespace InOne

#endif //_INONEMANAGER_H
