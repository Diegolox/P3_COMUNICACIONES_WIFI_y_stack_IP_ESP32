#include "network/NTP.h"
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

String obtenerHoraMadrid() {
    if (WiFi.status() != WL_CONNECTED) {
        return "";
    }

    // Configura NTP y la zona horaria una sola vez.
    static bool configurado = false;
    if (!configurado) {
        // CET en invierno y CEST en verano.
        configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org");
        configurado = true;
    }

    struct tm fechaHora;

    // Espera hasta 10 segundos si el reloj todavia no tiene una hora valida.
    if (!getLocalTime(&fechaHora, 10000)) {
        return "";
    }

    // 19 caracteres de fecha y hora, mas el terminador '\0'.
    char texto[20];
    if (strftime(texto, sizeof(texto), "%d/%m/%Y %H:%M:%S", &fechaHora) == 0) {
        return "";
    }

    return String(texto);
}
