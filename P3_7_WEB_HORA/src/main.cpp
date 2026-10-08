#include <Arduino.h>
#include "hal/uart.h"
#include "hal/wifi.h"
#include "network/web.h"
#include "app/mef.h"

void setup() {
    initUART();
    delay(1000);

    if (!init_wifi()) {
        escribirMensajeUART("Revisa el punto de acceso y reinicia el ESP32.");
        return;
    }

    initMEF();
    initWeb();
}

void loop() {
    actualizarWeb();
    delay(1);
}
