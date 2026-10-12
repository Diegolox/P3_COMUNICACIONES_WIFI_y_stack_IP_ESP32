#include <Arduino.h>
#include "hal/uart.h"
#include "hal/wifi.h"
#include "protocol/coms.h"

void setup() {
    initUART();
    delay(1000);

    if (!init_wifi()) {
        escribirMensajeUART("Revisa el punto de acceso y reinicia el ESP32.");
        return;
    }

    initComs();
}

void loop() {
    enviarHoraPeriodicamente();
}
