#include "AsArduino.h"

#define LED_IS_UAW PB3
#define LED_NO_UAW PB4

#define ledIsUavOn() digitalWrite(LED_IS_UAW, HIGH)
#define ledIsUavOff() digitalWrite(LED_IS_UAW, LOW)

#define ledNoUavOn() digitalWrite(LED_NO_UAW, HIGH)
#define ledNoUavOff() digitalWrite(LED_NO_UAW, LOW)
