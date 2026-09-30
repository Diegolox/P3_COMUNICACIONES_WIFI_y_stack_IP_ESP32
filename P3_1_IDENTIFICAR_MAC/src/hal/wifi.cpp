#include "hal/wifi.h"
#include <WiFi.h>

static const char* SSID = "telekino";
static const char* PASSWORD = "LeonardoTQ1852";

// Inicializa el puerto serie, muestra la MAC e intenta conectarse al WiFi.
void init_Wifi() {
    Serial.begin(115200);
    delay(1000);

    Serial.print("MAC del ESP32: ");
    Serial.println(obtenerMAC()); // OBTIENE LA MAC DEL ESP32

    Serial.print("Conectando a ");
    Serial.println(SSID);

    if (conectarWiFi(SSID, PASSWORD)) { // SI SE CONECTA AL WIFI CON UN SSID Y CONTRASEÑA
        Serial.println("WiFi conectado"); 
        Serial.print("IP del ESP32: ");
        Serial.println(obtenerIP()); // OBTIENE LA IP DEL ESP32
    } else {
        Serial.println("No se pudo conectar al WiFi");
    }
}

// Inicia la conexión y espera hasta conectarse o agotar el tiempo máximo.
bool conectarWiFi(const char* ssid, const char* password,
                  unsigned long timeoutMs) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    const unsigned long inicio = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - inicio < timeoutMs) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    return WiFi.status() == WL_CONNECTED;
}

// Comprueba el estado actual de la conexión.
bool wifiConectado() {
    return WiFi.status() == WL_CONNECTED;
}

// Activa el modo cliente y consulta su MAC; no necesita conexión.
String obtenerMAC() {
    WiFi.mode(WIFI_STA);
    return WiFi.macAddress();
}

// Consulta la IP que ha recibido el ESP32 al conectarse.
String obtenerIP() {
    if (!wifiConectado()) {
        return "";
    }

    return WiFi.localIP().toString();
}