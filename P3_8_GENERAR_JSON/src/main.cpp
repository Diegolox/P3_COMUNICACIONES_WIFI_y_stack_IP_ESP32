#include <Arduino.h>
#include "hal/uart.h"
#include "sensors/temp.h"


void setup() {
    initUART();
    delay(1000);

}

void loop() {
    Serial.println(generar_temp());
    delay(1000);
}
