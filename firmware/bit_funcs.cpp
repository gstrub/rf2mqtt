#include "bit_funcs.h"
#include <stdint.h>

void set_bit(uint8_t *buffer, uint16_t bit_index, uint8_t lsb_first)
{
  uint8_t byte_index = bit_index / 8;
  uint8_t bit_in_byte = bit_index % 8;
  if (lsb_first)
    buffer[byte_index] |= 1 << (7 - bit_in_byte);
  else
    buffer[byte_index] |= 1 << bit_in_byte;
}

void clr_bit(uint8_t *buffer, uint16_t bit_index, uint8_t lsb_first)
{
  uint8_t byte_index = bit_index / 8;
  uint8_t bit_in_byte = bit_index % 8;
  if (lsb_first)
    buffer[byte_index] &= ~(1 << (7 - bit_in_byte));
  else
    buffer[byte_index] &= ~(1 << bit_in_byte);
}

void def_bit(uint8_t *buffer, uint16_t bit_index, uint8_t value, uint8_t lsb_first)
{
  uint8_t byte_index = bit_index / 8;
  uint8_t bit_in_byte = bit_index % 8;
  value &= 1;
  if (lsb_first)
  {
    buffer[byte_index] &= ~(1 << (7 - bit_in_byte));
    buffer[byte_index] |= value << (7 - bit_in_byte);
  }
  else
  {
    buffer[byte_index] &= ~(1 << bit_in_byte);
    buffer[byte_index] |= value << bit_in_byte;
  }
}

uint8_t get_bit(uint8_t *buffer, uint16_t bit_index, uint8_t lsb_first)
{
  uint8_t byte_index = bit_index / 8;
  uint8_t bit_in_byte = bit_index % 8;
  if (lsb_first)
    return (buffer[byte_index] & (1 << (7 - bit_in_byte)) ? 1 : 0);
  else
    return (buffer[byte_index] & (1 << bit_in_byte) ? 1 : 0);
}

void setNibble(uint8_t *buffer, uint16_t nib_index, uint8_t value, uint8_t lsn_first)
{
  uint8_t byte_index = nib_index / 2;
  uint8_t bit_in_byte = (4 * (nib_index % 2));
  value &= 0xF;
  if (lsn_first)
  {
    buffer[byte_index] &= ~(0xF << (4 - bit_in_byte));
    buffer[byte_index] |= value << (4 - bit_in_byte);
  }
  else
  {
    buffer[byte_index] &= ~(0xF << bit_in_byte);
    buffer[byte_index] |= value << bit_in_byte;
  }
}

uint8_t getNibble(uint8_t *buffer, uint16_t nib_index, uint8_t value, uint8_t lsn_first)
{
  uint8_t byte_index = nib_index / 2;
  uint8_t bit_in_byte = (4 * (nib_index % 2));
  if (lsn_first)
    return (buffer[byte_index] & (0xF << (4 - bit_in_byte))) >> (4 - bit_in_byte);
  else
    return (buffer[byte_index] & (0xF << bit_in_byte)) >> bit_in_byte;
}
