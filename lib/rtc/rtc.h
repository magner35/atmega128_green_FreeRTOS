/*
 * rtc.h
 *
 *  Created on: 16/02/2012
 *      Author: Phillip Stevens
 *      modified : magner 2024
 */

#ifndef RTC_H_
#define RTC_H_

#include <time.h>
#include "FreeRTOS.h"

// Definitions for the DS1307 RTC
#define DS1307 0xD0 // device address of DS1307 Real Time Clock is 0xD0 (see data sheet)
/* definitions for use in the DS1307 Control byte */
#define SQWDISABLEOUT 0b10000000 //  level SQW should take if DISABLED
#define SQWENABLE 0b00010000     //  set to 1 to enable SQW output
#define SQWRS1 0b00000010        //  RS1 & RS0 set to 0 for 1Hz output
#define SQWRS0 0b00000001

/*----------------------------------------------------------------*/
uint8_t getDateTimeDS1307(struct tm *timeDate);    // Get the date & time as needed.
uint8_t setDateTimeDS1307(struct tm *timeDateSet); // Set the date & time initially & as needed.
/*----------------------------------------------------------------*/

#endif /* RTC_H_ */
