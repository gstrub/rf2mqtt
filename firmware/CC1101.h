#ifndef _CC1101_H
#define _CC1101_H

#include <stdint.h>

namespace CC1101
{

    typedef struct
    {
        uint8_t iocfg2;   // GDO2 Output Pin Configuration
        uint8_t iocfg1;   // GDO1 Output Pin Configuration
        uint8_t iocfg0;   // GDO0 Output Pin Configuration
        uint8_t fifothr;  // RX FIFO and TX FIFO Thresholds
        uint8_t sync1;    // Sync Word, High Byte
        uint8_t sync0;    // Sync Word, Low Byte
        uint8_t pktlen;   // Packet Length
        uint8_t pktctrl1; // Packet Automation Control
        uint8_t pktctrl0; // Packet Automation Control
        uint8_t addr;     // Device Address
        uint8_t channr;   // Channel Number
        uint8_t fsctrl1;  // Frequency Synthesizer Control
        uint8_t fsctrl0;  // Frequency Synthesizer Control
        uint8_t freq2;    // Frequency Control Word, High Byte
        uint8_t freq1;    // Frequency Control Word, Middle Byte
        uint8_t freq0;    // Frequency Control Word, Low Byte
        uint8_t mdmcfg4;  // Modem Configuration
        uint8_t mdmcfg3;  // Modem Configuration
        uint8_t mdmcfg2;  // Modem Configuration
        uint8_t mdmcfg1;  // Modem Configuration
        uint8_t mdmcfg0;  // Modem Configuration
        uint8_t deviatn;  // Modem Deviation Setting
        uint8_t mcsm2;    // Main Radio Control State Machine Configuration
        uint8_t mcsm1;    // Main Radio Control State Machine Configuration
        uint8_t mcsm0;    // Main Radio Control State Machine Configuration
        uint8_t foccfg;   // Frequency Offset Compensation Configuration
        uint8_t bscfg;    // Bit Synchronization Configuration
        uint8_t agcctrl2; // AGC Control
        uint8_t agcctrl1; // AGC Control
        uint8_t agcctrl0; // AGC Control
        uint8_t worevt1;  // High Byte Event0 Timeout
        uint8_t worevt0;  // Low Byte Event0 Timeout
        uint8_t worctrl;  // Wake On Radio Control
        uint8_t frend1;   // Front End RX Configuration
        uint8_t frend0;   // Front End TX Configuration
        uint8_t fscal3;   // Frequency Synthesizer Calibration
        uint8_t fscal2;   // Frequency Synthesizer Calibration
        uint8_t fscal1;   // Frequency Synthesizer Calibration
        uint8_t fscal0;   // Frequency Synthesizer Calibration
        uint8_t rcctrl1;  // RC Oscillator Configuration
        uint8_t rcctrl0;  // RC Oscillator Configuration
        uint8_t fstest;   // Frequency Synthesizer Calibration Control
        uint8_t ptest;    // Production Test
        uint8_t agctest;  // AGC Test
        uint8_t test2;    // Various Test Settings
        uint8_t test1;    // Various Test Settings
        uint8_t test0;    // Various Test Settings
    } Configuration;

    enum class Register : uint8_t
    {
        IOCFG2 = 0x00,   // GDO2 output pin configuration
        IOCFG1 = 0x01,   // GDO1 output pin configuration
        IOCFG0 = 0x02,   // GDO0 output pin configuration
        FIFOTHR = 0x03,  // RX FIFO and TX FIFO thresholds
        SYNC1 = 0x04,    // Sync word, high byte
        SYNC0 = 0x05,    // Sync word, low byte
        PKTLEN = 0x06,   // Packet length
        PKTCTRL1 = 0x07, // Packet automation control
        PKTCTRL0 = 0x08, // Packet automation control
        ADDR = 0x09,     // Device address
        CHANNR = 0x0A,   // Channel number
        FSCTRL1 = 0x0B,  // Frequency synthesizer control
        FSCTRL0 = 0x0C,  // Frequency synthesizer control
        FREQ2 = 0x0D,    // Frequency control word, high byte
        FREQ1 = 0x0E,    // Frequency control word, middle byte
        FREQ0 = 0x0F,    // Frequency control word, low byte
        MDMCFG4 = 0x10,  // Modem configuration
        MDMCFG3 = 0x11,  // Modem configuration
        MDMCFG2 = 0x12,  // Modem configuration
        MDMCFG1 = 0x13,  // Modem configuration
        MDMCFG0 = 0x14,  // Modem configuration
        DEVIATN = 0x15,  // Modem deviation setting
        MCSM2 = 0x16,    // Main Radio Cntrl State Machine config
        MCSM1 = 0x17,    // Main Radio Cntrl State Machine config
        MCSM0 = 0x18,    // Main Radio Cntrl State Machine config
        FOCCFG = 0x19,   // Frequency Offset Compensation config
        BSCFG = 0x1A,    // Bit Synchronization configuration
        AGCCTRL2 = 0x1B, // AGC control
        AGCCTRL1 = 0x1C, // AGC control
        AGCCTRL0 = 0x1D, // AGC control
        WOREVT1 = 0x1E,  // High byte Event 0 timeout
        WOREVT0 = 0x1F,  // Low byte Event 0 timeout
        WORCTRL = 0x20,  // Wake On Radio control
        FREND1 = 0x21,   // Front end RX configuration
        FREND0 = 0x22,   // Front end TX configuration
        FSCAL3 = 0x23,   // Frequency synthesizer calibration
        FSCAL2 = 0x24,   // Frequency synthesizer calibration
        FSCAL1 = 0x25,   // Frequency synthesizer calibration
        FSCAL0 = 0x26,   // Frequency synthesizer calibration
        RCCTRL1 = 0x27,  // RC oscillator configuration
        RCCTRL0 = 0x28,  // RC oscillator configuration
        FSTEST = 0x29,   // Frequency synthesizer cal control
        PTEST = 0x2A,    // Production test
        AGCTEST = 0x2B,  // AGC test
        TEST2 = 0x2C,    // Various test settings
        TEST1 = 0x2D,    // Various test settings
        TEST0 = 0x2E,    // Various test settings
    };

    enum class StrobeCommand : uint8_t
    {
        SRES = 0x30,    // Reset chip
        SFSTXON = 0x31, // Enable/calibrate freq synthesizer
        SXOFF = 0x32,   // Turn off crystal oscillator.
        SCAL = 0x33,    // Calibrate freq synthesizer & disable
        SRX = 0x34,     // Enable RX.
        STX = 0x35,     // Enable TX.
        SIDLE = 0x36,   // Exit RX / TX
        SAFC = 0x37,    // AFC adjustment of freq synthesizer
        SWOR = 0x38,    // Start automatic RX polling sequence
        SPWD = 0x39,    // Enter pwr down mode when CSn goes hi
        SFRX = 0x3A,    // Flush the RX FIFO buffer.
        SFTX = 0x3B,    // Flush the TX FIFO buffer.
        SWORRST = 0x3C, // Reset real time clock.
        SNOP = 0x3D,    // No operation.
    };

    enum class StatusRegister : uint8_t
    {
        PARTNUM = 0xF0,        // Part number
        VERSION = 0xF1,        // Current version number
        FREQEST = 0xF2,        // Frequency offset estimate
        LQI = 0xF3,            // Demodulator estimate for link quality
        RSSI = 0xF4,           // Received signal strength indication
        MARCSTATE = 0xF5,      // Control state machine state
        WORTIME1 = 0xF6,       // High byte of WOR timer
        WORTIME0 = 0xF7,       // Low byte of WOR timer
        PKTSTATUS = 0xF8,      // Current GDOx status and packet status
        VCO_VC_DAC = 0xF9,     // Current setting from PLL cal module
        TXBYTES = 0xFA,        // Underflow and # of bytes in TXFIFO
        RXBYTES = 0xFB,        // Overflow and # of bytes in RXFIFO
        RCCTRL1_STATUS = 0xFC, // Last RC Oscillator Calibration Result
        RCCTRL0_STATUS = 0xFD, // Last RC Oscillator Calibration Result
    };

    enum class ControlState : uint8_t
    {
        SLEEP = 0x00,
        IDLE = 0x01,
        XOFF = 0x02,
        VCOON_MC = 0x03,
        REGON_MC = 0x04,
        MANCAL = 0x05,
        VCOON = 0x06,
        REGON = 0x07,
        STARTCAL = 0x08,
        BWBOOST = 0x09,
        FS_LOCK = 0x0A,
        IFADCON = 0x0B,
        ENDCAL = 0x0C,
        RX = 0x0D,
        RX_END = 0x0E,
        RX_RST = 0x0F,
        TXRX_SWITCH = 0x10,
        RXFIFO_OVERFLOW = 0x11,
        FSTXON = 0x12,
        TX = 0x13,
        TX_ON = 0x14,
        RXTX_SWITCH = 0x15,
        TXFIFO_UNDERFLOW = 0x16
    };

    class Radio
    {

    public:
        Radio(uint8_t ssPin, uint8_t gdo0Pin = 255, uint8_t gdo2Pin = 255);

        bool begin();

        ControlState getState();
        uint8_t getChipPartNumber();
        uint8_t getChipVersion();
        uint8_t getNumRxBytes();
        uint8_t getNumTxBytes();
        bool isRxOverflow();
        bool isTxUnderflow();
        int8_t getRssi();

        void goIdle();
        void goReceive();
        void goTransmit();

        void readBurst(Register address, uint8_t *data, uint8_t length);
        void readConfiguration(Configuration *config);
        uint8_t readRegister(Register address);
        uint8_t readStatus(StatusRegister address);
        void readRxFifo(uint8_t *data, uint8_t length);
        void readPaTable(uint8_t *data, uint8_t length);

        void reset();

        void writeBurst(Register address, uint8_t *data, uint8_t length);
        void writeConfiguration(Configuration *config);
        void writeRegister(Register address, uint8_t data);
        void writeStrobe(StrobeCommand command);
        void writeTxFifo(uint8_t *data, uint8_t length);
        void writePaTable(uint8_t *data, uint8_t length);

    private:
        uint8_t m_ssPin;
        uint8_t m_gdo0Pin;
        uint8_t m_gdo2Pin;

        uint8_t _readRegister(uint8_t address);
        uint8_t _readBurst(uint8_t address, uint8_t *data, uint8_t length);
        uint8_t _writeBurst(uint8_t address, uint8_t *data, uint8_t length);
    };

    int8_t rssiToDbm(uint8_t rawRssi);

}; // namespace CC1101

#endif
