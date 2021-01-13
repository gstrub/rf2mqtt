#ifndef _BIT_FUNCS
#define _BIT_FUNCS

#include <stdint.h>
void set_bit(uint8_t *buffer, uint16_t bit_index, uint8_t lsb_first = false);
void clr_bit(uint8_t *buffer, uint16_t bit_index, uint8_t lsb_first = false);
void def_bit(uint8_t *buffer, uint16_t bit_index, uint8_t value, uint8_t lsb_first = false);
uint8_t get_bit(uint8_t *buffer, uint16_t bit_index, uint8_t lsb_first = false);

void setNibble(uint8_t *buffer, uint16_t nib_index, uint8_t value, uint8_t lsn_first = false);
uint8_t getNibble(uint8_t *buffer, uint16_t nib_index, uint8_t value, uint8_t lsn_first = false);

#endif
