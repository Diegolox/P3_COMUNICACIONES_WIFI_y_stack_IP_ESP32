#include <Arduino.h>
#include "hal/wifi.h"
#include "network/NTP.h"

// Variables globales
bool flag_init_wifi;


void setup() {
    flag_init_wifi = init_wifi();
}

void loop() {
    if (!flag_init_wifi) {
        Serial.println("Error al inicializar el WiFi.");
        delay(1000);
        return;
    }
    mostrarHoraMadrid();
    delay(1000);
}