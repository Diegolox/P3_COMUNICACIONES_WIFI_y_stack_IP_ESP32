#include <Arduino.h>
#include "hal/uart.h"
#include "sensors/temp.h"
#include "hal/wifi.h"
#include "network/MQTT.h"

int i = 0;

void setup() {
    initUART();
    delay(1000);
    if (!init_wifi()) {
        Serial.println("Error al conectar el Wi-Fi");
        return;
    }
    initMQTT();
}

void loop() {
    String dato = recibirMQTT();

    if (!dato.isEmpty()) {
        Serial.print("MQTT: dato recibido: ");
        Serial.println(dato);
    }
    delay(500);
}
