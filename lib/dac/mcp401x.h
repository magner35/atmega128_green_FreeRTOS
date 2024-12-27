#ifndef _MCP401x_H_
#define _MCP401x_H_

#include "FreeRTOS.h"

#define MCP401x_I2C_ADDRESS 0x5E // (0x2F unshifted)

uint8_t MCP401x_GetWiperValue(void);      // get wiper value
void MCP401x_SetWiperValue(uint8_t data); // set resistance value

#endif // _MCP401x_H_