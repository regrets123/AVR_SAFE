#include "safe.h"
#include "servo.h"
#include "millis.h"

uint8_t inputLockState = 0; /* all locked by default */


uint8_t lightLed(uint8_t index)
{
    return 0;
}

uint8_t unlock()
{
    servo1_set_percentage(-60); //Full right
    return 0;
}

uint8_t lock()
{
    servo1_set_percentage(100); // Full left
    return 0;
}
