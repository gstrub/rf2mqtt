/*---------------------------------------------------------------------------
 * CC1101 Controller for Legrand InOne RF switches
 */
#include <avr/sleep.h>
#include "cc1101.h"
#include <EnableInterrupt.h>
#include "bit_funcs.h"
#include "InOneManager.h"
#include "InOneSwitch.h"
#include "IdeoManager.h"
#include "IdeoSerial.h"
#include <LiquidCrystal.h>

// Initialize LiquidCrystal library with DFRobot LCD-keypad shield pin assignments
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

using namespace CC1101;

// Pin assignment for the CC1101 TRX dedicated to the IOBL protocol
#define IOBL_SS_PIN 3
#define IOBL_INT_PIN 2

InOne::Manager inOneManager(IOBL_SS_PIN, IOBL_INT_PIN);
InOne::Switch sw(0x1CAFE, &inOneManager);

Ideo::Manager ideoManager(IOBL_SS_PIN, IOBL_INT_PIN);
bool isIdeoMode = false;

// Interrupt callback that will be called on incoming RX packet
void rfCallback()
{
  disableInterrupt(IOBL_INT_PIN);

  // if we are using Ideo command/response, forward to ideoManager
  if (isIdeoMode)
    ideoManager.rfRxCallback();
  // otherwise (listening mode), forward to inOneManager
  else
    inOneManager.rfRxCallback();

  enableInterrupt(IOBL_INT_PIN, rfCallback, RISING);
}

/** Énumération des boutons utilisables */
enum
{
  BUTTON_NONE,  /*!< Pas de bouton appuyé */
  BUTTON_UP,    /*!< Bouton UP (haut) */
  BUTTON_DOWN,  /*!< Bouton DOWN (bas) */
  BUTTON_LEFT,  /*!< Bouton LEFT (gauche) */
  BUTTON_RIGHT, /*!< Bouton RIGHT (droite) */
  BUTTON_SELECT /*!< Bouton SELECT */
};

/** Retourne le bouton appuyé (si il y en a un) */
byte getPressedButton()
{

  /* Lit l'état des boutons */
  int value = analogRead(A0);

  /* Calcul l'état des boutons */
  if (value < 50)
    return BUTTON_RIGHT;
  else if (value < 200)
    return BUTTON_UP;
  else if (value < 350)
    return BUTTON_DOWN;
  else if (value < 500)
    return BUTTON_LEFT;
  else if (value < 850)
    return BUTTON_SELECT;
  else
    return BUTTON_NONE;
}

//---------------------------------[SETUP]-----------------------------------
void setup()
{
  // init serial Port for debugging
  Serial.begin(115200);
  Serial.println(F("Begin CC1101 setup"));

  // Start Legrand IOBL Manager
  inOneManager.begin();

  enableInterrupt(IOBL_INT_PIN, rfCallback, RISING);

  lcd.begin(16, 2);
  lcd.print(F("IOBL Manager"));

  Serial.println(F("CC1101 TX Demo")); //welcome message
}

//---------------------------------[LOOP]-----------------------------------
void loop()
{
  static uint8_t prev_button = 0;
  static char serial_buffer[32];
  static uint8_t serial_ptr = 0;
  static const char delims[] = ",";

  uint8_t button = getPressedButton();
  if (button != BUTTON_NONE && prev_button == BUTTON_NONE)
  {
    switch (button)
    {
    case BUTTON_UP:
      sw.turnOn(InOne::Channel::Left);
      Serial.println("BUTTON_UP");
      break;
    case BUTTON_DOWN:
      sw.turnOff(InOne::Channel::Left);
      Serial.println("BUTTON_DOWN");
      break;
    case BUTTON_LEFT:
      sw.turnOn(InOne::Channel::Right);
      Serial.println("BUTTON_LEFT");
      break;
    case BUTTON_RIGHT:
      sw.turnOff(InOne::Channel::Right);
      Serial.println("BUTTON_RIGHT");
      break;
    case BUTTON_SELECT:
      if (sw.isLearnMode())
        sw.stopLearn();
      else
        sw.startLearn();
      Serial.println("BUTTON_SELECT");
      break;
    }
  }
  prev_button = button;

  if (Serial.available())
  {
    char c = Serial.read();
    if (c != '\r' && c != '\n' && serial_ptr < 32)
    {
      serial_buffer[serial_ptr++] = c;
    }
    if (c == '\n' || c == '\r')
    {
      serial_buffer[serial_ptr] = 0;
      if (serial_ptr == 32)
      {
        Serial.println("Serial RX buffer overrun");
      }
      else if (serial_buffer[0] == '0')
      {
        char *token = strtok(&serial_buffer[2], delims);
        InOne::Packet packet;
        packet.isLearnMode = false;
        packet.type = InOne::PacketType::Short;
        uint8_t ntok = 0;
        while (token != NULL)
        {
          switch (ntok++)
          {
          case 0:
            packet.sequenceIndex = atoi(token);
            break;
          case 1:
            packet.id = atol(token);
            break;
          case 2:
            packet.channel = (InOne::Channel)atoi(token);
            break;
          case 3:
            packet.command = (InOne::Command)atoi(token);
            break;
          case 4:
            packet.isLearnMode = strcmp(token, "L") == 0;
            break;
          case 5:
            packet.data[0] = atoi(token);
            break;
          case 6:
            packet.data[1] = atoi(token);
            break;
          case 7:
            packet.data[2] = atoi(token);
            break;
          }
          token = strtok(NULL, delims);
        }
        if (ntok > 3)
        {
          if (ntok > 5)
            packet.type = InOne::PacketType::Long;
          else if (ntok == 5)
            packet.type = InOne::PacketType::Medium;
          inOneManager.sendPacket(&packet);
        }
        else
        {
          Serial.print("Not enough data, got tokens: ");
          Serial.println(ntok);
        }
      }
      else if (serial_buffer[0] == '1')
      {
        disableInterrupt(IOBL_INT_PIN);
        isIdeoMode = true;
        inOneManager.detachRadio();
        ideoManager.attachRadio();
        enableInterrupt(IOBL_INT_PIN, rfCallback, RISING);

        Ideo::TxPacketData tx;
        Ideo::RxPacketData rx;

        if (Ideo::SerialParser::parseMessage(&serial_buffer[2], &tx))
        {
          ideoManager.commandResponse(&tx, &rx);
          Serial.print("1>");
          Ideo::SerialParser::print(&rx);
        }

        disableInterrupt(IOBL_INT_PIN);
        isIdeoMode = false;
        ideoManager.detachRadio();
        inOneManager.attachRadio();
        enableInterrupt(IOBL_INT_PIN, rfCallback, RISING);
      }
      serial_ptr = 0;
    }
  }

  if (inOneManager.isPacketAvailable())
  {
    InOne::Packet rxPacket;
    inOneManager.getLastPacket(&rxPacket);
    Serial.print("0>");
    Serial.print(rxPacket.sequenceIndex);
    Serial.print(',');
    Serial.print(rxPacket.id);
    Serial.print(',');
    Serial.print((uint8_t)rxPacket.channel);
    Serial.print(',');
    Serial.print((uint8_t)rxPacket.command);
    if (rxPacket.isLearnMode || rxPacket.type != InOne::PacketType::Short)
      Serial.print(',');
    if (rxPacket.isLearnMode)
      Serial.print('L');
    if (rxPacket.type != InOne::PacketType::Short)
    {
      Serial.print(',');
      Serial.print(rxPacket.data[0]);
    }
    if (rxPacket.type == InOne::PacketType::Long)
    {
      Serial.print(',');
      Serial.print(rxPacket.data[1]);
      Serial.print(',');
      Serial.print(rxPacket.data[2]);
    }
    Serial.println();
  }

  if (ideoManager.isPacketAvailable())
  {
    Ideo::RxPacketData rxPacket;
    ideoManager.getLastPacket(&rxPacket);
    Serial.print("1>");
    Serial.print(rxPacket.device);
    Serial.print(',');
    Serial.print(rxPacket.command >> 4, HEX);
    Serial.print(rxPacket.command & 0xF, HEX);
    Serial.print(',');
    Ideo::printParams(rxPacket.params);
    Serial.print(',');
    Serial.print(rxPacket.rssi);
    Serial.print(',');
    Serial.println(rxPacket.lqi);
  }
}
