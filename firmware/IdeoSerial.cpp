#include <Arduino.h>
#include "IdeoSerial.h"

using namespace Ideo;

static const char *delims = ",";

bool SerialParser::parseMessage(char *message, TxPacketData *tx)
{
    uint8_t ntok = 0;
    char *token = strtok(message, delims);
    while (token != NULL)
    {
        switch (ntok++)
        {
        case 0:
            if (strlen(token) != 1)
            {
                Serial.print("Device ID must be a 1-byte token. Got: ");
                Serial.println(token);
                return false;
            }
            tx->device = token[0];
            break;

        case 1:
            if (strlen(token) != 2)
            {
                Serial.print("Command must be a 2-byte token. Got: ");
                Serial.println(token);
                return false;
            }
            tx->command = (Ideo::parseNibble(token[0]) << 4) | Ideo::parseNibble(token[1]);
            break;

        case 2:
            if (strlen(token) != 8)
            {
                Serial.print("Command parameters must be a 8-byte token. Got:");
                Serial.println(token);
                return false;
            }
            memcpy(tx->params, token, 8);
            break;
        }
        token = strtok(NULL, delims);
    }
    if (ntok >= 3)
    {
        return true;
    }
    else
    {
        Serial.print("Not enough data, got tokens: ");
        Serial.println(ntok);
    }
    return false;
}

void SerialParser::print(const RxPacketData *rx)
{
    Serial.print(rx->device);
    Serial.print(',');
    Serial.print(rx->command >> 4, HEX);
    Serial.print(rx->command & 0xF, HEX);
    Serial.print(',');
    Ideo::printParams(rx->params);
    Serial.println();
}