#include "network/NTP.h"
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

bool mostrarHoraMadrid() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("NTP: el ESP32 no esta conectado al WiFi.");
        return false;
    }

    // CET en invierno y CEST en verano.
    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org");

    struct tm fechaHora;

    // Espera hasta 10 segundos a que llegue la hora por NTP.
    if (!getLocalTime(&fechaHora, 10000)) {
        Serial.println("NTP: no se pudo obtener la hora.");
        return false;
    }

    Serial.print("Fecha y hora de Madrid: ");
    Serial.println(&fechaHora, "%d/%m/%Y %H:%M:%S");

    return true;
}