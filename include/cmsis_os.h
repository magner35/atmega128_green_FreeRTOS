#ifndef _CMSIS_OS_H
#define _CMSIS_OS_H

/// Mail ID identifies the mail queue (pointer to a mail queue control block).
/// \note CAN BE CHANGED: \b os_mailQ_cb is implementation specific in every CMSIS-RTOS.
typedef struct os_mailQ_cb *osMailQId;

/// Status code values returned by CMSIS-RTOS functions.
/// \note MUST REMAIN UNCHANGED: \b osStatus shall be consistent in every CMSIS-RTOS.
typedef enum
{
    osOK = 0,                       ///< function completed; no error or event occurred.
    osEventSignal = 0x08,           ///< function completed; signal event occurred.
    osEventMessage = 0x10,          ///< function completed; message event occurred.
    osEventMail = 0x20,             ///< function completed; mail event occurred.
    osEventTimeout = 0x40,          ///< function completed; timeout occurred.
    osErrorParameter = 0x80,        ///< parameter error: a mandatory parameter was missing or specified an incorrect object.
    osErrorResource = 0x81,         ///< resource not available: a specified resource was not available.
    osErrorTimeoutResource = 0xC1,  ///< resource not available within given time: a specified resource was not available within the timeout period.
    osErrorISR = 0x82,              ///< not allowed in ISR context: the function cannot be called from interrupt service routines.
    osErrorISRRecursive = 0x83,     ///< function called multiple times from ISR with same object.
    osErrorPriority = 0x84,         ///< system cannot determine priority or thread has illegal priority.
    osErrorNoMemory = 0x85,         ///< system is out of memory: it was impossible to allocate or reserve memory for the operation.
    osErrorValue = 0x86,            ///< value of a parameter is out of range.
    osErrorOS = 0xFF,               ///< unspecified RTOS error: run-time error but no other error message fits.
    os_status_reserved = 0x7FFFFFFF ///< prevent from enum down-size compiler optimization.
} osStatus;

/// Event structure contains detailed information about an event.
/// \note MUST REMAIN UNCHANGED: \b os_event shall be consistent in every CMSIS-RTOS.
///       However the struct may be extended at the end.
typedef struct
{
    osStatus status; ///< status code: event or error information
    union
    {
        uint32_t v;      ///< message as 32-bit value
        void *p;         ///< message or mail as void pointer
        int32_t signals; ///< signal flags
    } value;             ///< event value
    union
    {
        osMailQId mail_id;        ///< mail id obtained by \ref osMailCreate
        QueueHandle_t message_id; ///< message id obtained by \ref osMessageCreate
    } def;                        ///< event definition
} osEvent;

/// Timeout value.
/// \note MUST REMAIN UNCHANGED: \b osWaitForever shall be consistent in every CMSIS-RTOS.
#define osWaitForever 0xFFFFFFFF ///< wait forever timeout value

osEvent osMessageGet(QueueHandle_t queue_id, uint32_t millisec);
osStatus osMessagePut(QueueHandle_t queue_id, uint32_t info, uint32_t millisec);

#endif // _CMSIS_OS_H
