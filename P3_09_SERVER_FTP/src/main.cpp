#include <Arduino.h>
#include "hal/uart.h"
#include "sensors/temp.h"
#include "hal/wifi.h"
#include "network/FTP.h"

int i = 0;

void setup() {
    initUART();
    delay(1000);
    if (!init_wifi()) {
        Serial.println("Error al conectar el Wi-Fi");
        return;
    }
}

void loop() {
    String temp = generar_temp();
    String nombre = "temperatura" + String(i) + ".json";

    Serial.println(nombre);
    Serial.println(temp);

    subirArchivoFTP(nombre.c_str(), temp);

    i++;
    delay(10000);
}