#ifndef _IDEOSERIAL_H
#define _IDEOSERIAL_H

#include "IdeoManager.h"

namespace Ideo
{
    class SerialParser
    {
    public:
        static bool parseMessage(char *message, TxPacketData *tx);
        static void print(const RxPacketData *rx);
    };
} // namespace Ideo

#endif