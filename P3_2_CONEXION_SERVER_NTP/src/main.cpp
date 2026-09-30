#include <Arduino.h>
#include "hal/wifi.h"
#include "network/ping.h"

void setup() {
    init_wifi();
}

void loop() {
    ping_Google();
    delay(1000);
}