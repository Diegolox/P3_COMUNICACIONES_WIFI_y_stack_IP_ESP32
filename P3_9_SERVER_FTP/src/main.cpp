#include <Arduino.h>
#include "hal/uart.h"
#include "sensors/temp.h"
#include "hal/wifi.h"
#include "network/FTP.h"


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
    Serial.println(temp);
    subirArchivoFTP("temperatura.json", temp);
    delay(1000);

}
