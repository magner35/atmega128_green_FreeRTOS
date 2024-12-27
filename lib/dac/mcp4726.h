#ifndef _MCP4726_H_
#define _MCP4726_H_

#include "FreeRTOS.h"

#define MCP4726_I2C_ADDRESS 0xC0 // (0x60 unshifted)

void MCP4726_SetOutput(uint16_t data);

// Function that works without waiting for the device to be ready
void DAC_SetOutput(uint16_t data);

#endif // !_MCP4726_H_
