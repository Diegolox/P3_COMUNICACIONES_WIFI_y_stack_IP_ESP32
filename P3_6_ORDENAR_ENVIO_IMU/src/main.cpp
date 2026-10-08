#include <Arduino.h>
#include "hal/uart.h"
#include "hal/wifi.h"
#include "protocol/coms.h"
#include "sensors/BNO055.h"

static bool imuListo = false;

void setup() {
    initUART();
    delay(1000);

    // Inicializa el sensor y comprueba si responde.
    imuListo = initBNO055();

    if (imuListo) {
        escribirMensajeUART("BNO055 conectado.");
    } else {
        escribirMensajeUART(
            "BNO055 no responde. Revisa cables, pines y direccion I2C."
        );
    }

    if (!init_wifi()) {
        escribirMensajeUART(
            "Revisa el punto de acceso y reinicia el ESP32."
        );
        return;
    }

    // El servidor Python debe estar escuchando antes de esta llamada.
    initComs();
}

void loop() {
    actualizarComs();

    // Solo lee y envia datos si el sensor se inicio correctamente.
    if (imuListo) {
        enviarIMUPeriodicamente(100);
    }
}