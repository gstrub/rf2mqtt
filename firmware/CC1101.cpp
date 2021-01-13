/***
* CC1101 Arduino driver
* Based on the CC1101 library by SpaceTeddy (https://github.com/SpaceTeddy/CC1101) and others
**/

#include <Arduino.h>
#include "cc1101.h"

using namespace CC1101;

/**** SPI bus functions ****/
#define SCK_PIN 13
#define MISO_PIN 12
#define MOSI_PIN 11
#define DEFAULT_SS_PIN 10

void spiOpen()
{
    pinMode(SCK_PIN, OUTPUT);
    pinMode(MOSI_PIN, OUTPUT);
    pinMode(MISO_PIN, INPUT);
    /* Pin 10 on an Uno *must* be high when starting the SPI module 
    so that we start in master mode. Otherwise we are in slave mode
    This is a limitation of the hardware */
    pinMode(DEFAULT_SS_PIN, OUTPUT);
    digitalWrite(DEFAULT_SS_PIN, HIGH);

    SPCR = ((1 << SPE) |                // SPI Enable
            (0 << SPIE) |               // SPI Interupt Enable
            (0 << DORD) |               // Data Order (0:MSB first / 1:LSB first)
            (1 << MSTR) |               // Master/Slave select
            (0 << SPR1) | (0 << SPR0) | // SPI Clock Rate
            (0 << CPOL) |               // Clock Polarity (0:SCK low / 1:SCK hi when idle)
            (0 << CPHA));               // Clock Phase (0:leading / 1:trailing edge sampling)

    //  SPSR =  (1<<SPI2X);                  // Double Clock Rate
}

uint8_t spiTransfer(uint8_t outData)
{
    SPDR = outData;
    while (!(SPSR & (1 << SPIF)))
        ;
    return SPDR;
}

void spiWaitReady()
{
    while (digitalRead(MOSI_PIN) == 0)
        ;
}

/**** Helper class for SPI transactions ****/
/* When instanciated, driver ssPin low and waits for the SPI bus to be ready */
/* When going out of scope, releases the ssPin */
class SpiTransaction
{
public:
    SpiTransaction(uint8_t ssPin)
    {
        this->m_ssPin = ssPin;
        digitalWrite(this->m_ssPin, LOW);
        spiWaitReady();
    }

    ~SpiTransaction()
    {
        digitalWrite(this->m_ssPin, HIGH);
    }

private:
    uint8_t m_ssPin;
};

/**** Radio class implementation ****/

/*---------------------------[CC1100 - R/W offsets]---------------------------*/
#define WRITE_SINGLE_BYTE 0x00
#define WRITE_BURST 0x40
#define READ_SINGLE_BYTE 0x80
#define READ_BURST 0xC0
/*---------------------------[END R/W offsets]--------------------------------*/

/*------------------------[CC1100 - FIFO commands]----------------------------*/
#define TXFIFO_BURST 0x7F        //write burst only
#define TXFIFO_SINGLE_BYTE 0x3F  //write single only
#define RXFIFO_BURST 0xFF        //read burst only
#define RXFIFO_SINGLE_BYTE 0xBF  //read single only
#define PATABLE_BURST 0x7E       //power control read/write
#define PATABLE_SINGLE_BYTE 0xFE //power control read/write
/*---------------------------[END FIFO commands]------------------------------*/

#define RSSI_OFFSET 74

Radio::Radio(uint8_t ssPin, uint8_t gdo0Pin, uint8_t gdo2Pin)
{
    this->m_ssPin = ssPin;
    this->m_gdo0Pin = gdo0Pin;
    this->m_gdo2Pin = gdo2Pin;
}

bool Radio::begin()
{
    pinMode(this->m_ssPin, OUTPUT);
    digitalWrite(this->m_ssPin, HIGH);

    /* Necessary to handle this here ? */
    pinMode(this->m_gdo0Pin, INPUT);
    pinMode(this->m_gdo2Pin, INPUT);

    spiOpen();
    this->reset();

    this->writeStrobe(StrobeCommand::SFTX);
    delayMicroseconds(100);
    this->writeStrobe(StrobeCommand::SFRX);
    delayMicroseconds(100);

    this->goIdle();
    return true;
}

ControlState Radio::getState()
{
    return (ControlState)this->readStatus(StatusRegister::MARCSTATE);
}

uint8_t Radio::getChipPartNumber()
{
    return this->readStatus(StatusRegister::PARTNUM);
}

uint8_t Radio::getChipVersion()
{
    return this->readStatus(StatusRegister::VERSION);
}

uint8_t Radio::getNumRxBytes()
{
    return this->readStatus(StatusRegister::RXBYTES) & 0x7F;
}

uint8_t Radio::getNumTxBytes()
{
    return this->readStatus(StatusRegister::TXBYTES) & 0x7F;
}

bool Radio::isRxOverflow()
{
    return (this->readStatus(StatusRegister::RXBYTES) & 0x80) != 0;
}

bool Radio::isTxUnderflow()
{
    return (this->readStatus(StatusRegister::TXBYTES) & 0x80) != 0;
}

int8_t Radio::getRssi()
{
    return rssiToDbm(this->readStatus(StatusRegister::RSSI));
}

int8_t CC1101::rssiToDbm(uint8_t rawRssi)
{
    if (rawRssi >= 128)
    {
        return (rawRssi - 256) / 2 - RSSI_OFFSET;
    }
    else
    {
        return rawRssi / 2 - RSSI_OFFSET;
    }
}

void Radio::goIdle()
{
    this->writeStrobe(StrobeCommand::SIDLE);
    while (this->getState() != ControlState::IDLE)
        ;
}

void Radio::goReceive()
{
    this->goIdle();
    this->writeStrobe(StrobeCommand::SRX);
    while (this->getState() != ControlState::RX)
        ;
}

void Radio::goTransmit()
{
    this->goIdle();
    this->writeStrobe(StrobeCommand::STX);
    while (this->getState() != ControlState::IDLE)
        ;
}

void Radio::readBurst(Register address, uint8_t *data, uint8_t length)
{
    this->_readBurst((uint8_t)address, data, length);
}

void Radio::readConfiguration(Configuration *config)
{
    this->_readBurst(0, (uint8_t *)config, sizeof(Configuration));
}

uint8_t Radio::readRegister(Register address)
{
    return this->_readRegister((uint8_t)address | READ_SINGLE_BYTE);
}

uint8_t Radio::readStatus(StatusRegister address)
{
    return this->_readRegister((uint8_t)address);
}

void Radio::readRxFifo(uint8_t *data, uint8_t length)
{
    this->_readBurst(RXFIFO_BURST, data, length);
}

void Radio::readPaTable(uint8_t *data, uint8_t length)
{
    this->_readBurst(PATABLE_BURST, data, length);
}

void Radio::reset()
{
    /* Reset operation as defined in the Radio datasheet */
    digitalWrite(this->m_ssPin, LOW);
    delayMicroseconds(10);
    digitalWrite(this->m_ssPin, HIGH);
    delayMicroseconds(40);

    this->writeStrobe(StrobeCommand::SRES);
    delay(1);
}

void Radio::writeBurst(Register address, uint8_t *data, uint8_t length)
{
    this->_writeBurst((uint8_t)address, data, length);
}

void Radio::writeConfiguration(Configuration *config)
{
    this->_writeBurst(0, (uint8_t *)config, sizeof(Configuration));
}

void Radio::writeRegister(Register address, uint8_t data)
{
    SpiTransaction transaction(this->m_ssPin);
    spiTransfer((uint8_t)address | WRITE_SINGLE_BYTE);
    spiTransfer(data);
}

void Radio::writeStrobe(StrobeCommand command)
{
    SpiTransaction transaction(this->m_ssPin);
    spiTransfer((uint8_t)command);
}

void Radio::writeTxFifo(uint8_t *data, uint8_t length)
{
    this->_writeBurst(TXFIFO_BURST, data, length);
}

void Radio::writePaTable(uint8_t *data, uint8_t length)
{
    this->_writeBurst(PATABLE_BURST, data, length);
}

/**** Private functions, read/write without address type checking ****/
uint8_t Radio::_readRegister(uint8_t address)
{
    SpiTransaction transaction(this->m_ssPin);

    spiTransfer(address);
    return spiTransfer(0xFF);
}

uint8_t Radio::_readBurst(uint8_t address, uint8_t *data, uint8_t length)
{
    SpiTransaction transaction(this->m_ssPin);

    spiTransfer(address | READ_BURST);
    for (uint8_t i = 0; i < length; i++)
        data[i] = spiTransfer(0xFF);
}

uint8_t Radio::_writeBurst(uint8_t address, uint8_t *data, uint8_t length)
{
    SpiTransaction transaction(this->m_ssPin);

    spiTransfer(address | WRITE_BURST);
    for (uint8_t i = 0; i < length; i++)
        spiTransfer(data[i]);
}
