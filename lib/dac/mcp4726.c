/* FreeRTOS driver for 12-Bit Single DAC: MCP4726
 * by magner 2024
 */

#include <avr/delay.h>
/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* i2c Interface include file. */
#include "i2cMultiMaster.h"

#include "mcp4726.h"

void MCP4726_SetOutput(uint16_t data // 0...4095
)
{
    uint8_t I2C_command_buf[4];
    if (xI2CSemaphore != NULL) // use the xI2CSemaphore if you're sharing the I2C bus.
    {
        // See if we can obtain the semaphore.  If the semaphore is not available
        // wait 10 ticks to see if it becomes free.
        if (xSemaphoreTake(xI2CSemaphore, (TickType_t)10) == pdTRUE)
        {
            I2C_command_buf[0] = MCP4726_I2C_ADDRESS + I2C_WRITE;
            I2C_command_buf[1] = 0b01011000; // Mode setup
            I2C_command_buf[2] = (uint8_t)(data >> 4);
            I2C_command_buf[3] = (uint8_t)(data << 4);
            I2C_Master_Start_Transceiver_With_Data((uint8_t *)&I2C_command_buf, 4);
            xSemaphoreGive(xI2CSemaphore);
        }
    }
    return pdTRUE;
}

static void I2C_Start()
{
    TWCR = ((1 << TWINT) | (1 << TWSTA) | (1 << TWEN));
    while (!(TWCR & (1 << TWINT)))
        ;
}

static void I2C_Stop(void)
{
    TWCR = ((1 << TWINT) | (1 << TWEN) | (1 << TWSTO));
    _delay_us(50); // wait for a short time
}

static void I2C_Write(uint8_t data)
{
    TWDR = data;
    TWCR = ((1 << TWINT) | (1 << TWEN));
    while (!(TWCR & (1 << TWINT)))
        ;
}

// mcp4726
void DAC_SetOutput(uint16_t data)
{
    if (xI2CSemaphore != NULL) // use the xI2CSemaphore if you're sharing the I2C bus.
    {
        // See if we can obtain the semaphore.  If the semaphore is not available
        // wait 10 ticks to see if it becomes free.
        if (xSemaphoreTake(xI2CSemaphore, (TickType_t)10) == pdTRUE)
        {
            I2C_Start();
            I2C_Write(MCP4726_I2C_ADDRESS + I2C_WRITE);
            I2C_Write(0b01011000); // Mode setup
            I2C_Write((uint8_t)(data >> 4));
            I2C_Write((uint8_t)(data << 4));
            I2C_Stop();
            xSemaphoreGive(xI2CSemaphore);
        }
    }
    return pdTRUE;
}
