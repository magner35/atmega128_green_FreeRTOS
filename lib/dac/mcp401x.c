/* FreeRTOS driver for 7-Bit Single Digital POT: MCP4017, MCP4018, MCP2019
 * by magner 2024
 */

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* i2c Interface include file. */
#include "i2cMultiMaster.h"

#include "mcp401x.h"

uint8_t MCP401x_GetWiperValue(void)
{
    uint8_t I2C_command_buf[2];
    if (xI2CSemaphore != NULL) // use the xI2CSemaphore if you're sharing the I2C bus.
    {
        // See if we can obtain the semaphore.  If the semaphore is not available
        // wait 10 ticks to see if it becomes free.
        if (xSemaphoreTake(xI2CSemaphore, (TickType_t)10) == pdTRUE)
        {
            I2C_command_buf[0] = MCP401x_I2C_ADDRESS + I2C_READ;
            I2C_Master_Start_Transceiver_With_Data((uint8_t *)&I2C_command_buf, 1);
            I2C_Master_Get_Data_From_Transceiver((uint8_t *)&I2C_command_buf, 2);
            xSemaphoreGive(xI2CSemaphore);
            return I2C_command_buf[1];
        }
    }
    return pdTRUE;
}

void MCP401x_SetWiperValue(uint8_t data)
{
    uint8_t I2C_command_buf[2];
    if (xI2CSemaphore != NULL) // use the xI2CSemaphore if you're sharing the I2C bus.
    {
        // See if we can obtain the semaphore.  If the semaphore is not available
        // wait 10 ticks to see if it becomes free.
        if (xSemaphoreTake(xI2CSemaphore, (TickType_t)10) == pdTRUE)
        {
            I2C_command_buf[0] = MCP401x_I2C_ADDRESS + I2C_WRITE;
            I2C_command_buf[1] = data;
            I2C_Master_Start_Transceiver_With_Data((uint8_t *)&I2C_command_buf, 2);
            xSemaphoreGive(xI2CSemaphore);
        }
    }
    return pdTRUE;
}
