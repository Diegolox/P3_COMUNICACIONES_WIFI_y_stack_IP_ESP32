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
    static unsigned long ultimoEnvio = 0;
    const unsigned long ahora = millis();

    if (ahora - ultimoEnvio >= 5000) {
        ultimoEnvio = ahora;

        float temp = generar_temp_float();
        Serial.println(temp);

        if (!envioMQTT(temp)) {
            Serial.println("No se pudo enviar el dato");
        }
    }
}
