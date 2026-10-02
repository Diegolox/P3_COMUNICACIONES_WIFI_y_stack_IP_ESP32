#include <Arduino.h>
#include "hal/wifi.h"
#include "network/NTP.h"

void setup() {
    init_wifi();
}

void loop() {
    mostrarHoraMadrid();
    delay(1000);
}