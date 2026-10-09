#include "network/NTP.h"
#include <WiFi.h>
#include <time.h>

static bool configurado = false;

void initNTP() {
    if (WiFi.status() != WL_CONNECTED) return;

    // Hora peninsular: CET en invierno y CEST en verano.
    // La sincronización se hace en segundo plano, sin esperar aquí.
    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org");
    configurado = true;
}

static String leerHora(const char* formato) {
    if (!configurado) initNTP();
    if (!configurado) return "";

    struct tm fechaHora;
    // No esperar 10 segundos en cada petición de la web.
    if (!getLocalTime(&fechaHora, 0)) return "";

    char texto[20];
    if (strftime(texto, sizeof(texto), formato, &fechaHora) == 0) return "";
    return String(texto);
}

String obtenerHoraMadrid() {
    return leerHora("%d/%m/%Y %H:%M:%S");
}

String obtenerHoraReloj() {
    return leerHora("%H:%M:%S");
}
