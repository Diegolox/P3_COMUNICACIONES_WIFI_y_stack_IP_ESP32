#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "network/web.h"
#include "network/web_html.h"

// WebServer forma parte del framework Arduino para ESP32.
static WebServer servidor(80);
static bool webIniciada = false;

void alPulsarReset() {
    // Añade aquí lo que quieras hacer al pulsar Reset.
}

void alPulsarPonerEnHora() {
    // Añade aquí lo que quieras hacer al pulsar Poner en hora.
}

void initWeb() {
    if (webIniciada || WiFi.status() != WL_CONNECTED) {
        return;
    }

    servidor.on("/", HTTP_GET, []() {
        servidor.sendHeader("Cache-Control", "no-store");
        servidor.send_P(200, "text/html; charset=utf-8", PAGINA_WEB);
    });

    // El navegador envía una petición distinta por cada botón.
    servidor.on("/reset", HTTP_POST, []() {
        alPulsarReset();
        servidor.send(204); // Pulsación recibida; respuesta sin contenido.
    });

    servidor.on("/poner-hora", HTTP_POST, []() {
        alPulsarPonerEnHora();
        servidor.send(204);
    });

    servidor.onNotFound([]() {
        servidor.send(404, "text/plain; charset=utf-8", "Ruta no encontrada");
    });

    servidor.begin();
    webIniciada = true;
    Serial.print("Abre la web en: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
}

void actualizarWeb() {
    if (webIniciada) {
        servidor.handleClient();
    }
}
