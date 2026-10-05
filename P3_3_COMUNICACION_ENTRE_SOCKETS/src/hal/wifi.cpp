#include "hal/wifi.h"
#include <WiFi.h>

bool init_wifi() {
    Serial.begin(115200);
    delay(1000);

    // STA: el ESP32 se conecta a un punto de acceso.
    WiFi.mode(WIFI_STA);

    // Primero escanea y después intenta conectar al AP configurado.
    escanearWiFi();

    if (!conectarWiFi(SSID, PASSWORD)) {
        Serial.println("No se pudo conectar al AP.");
        return false;
    }

    Serial.println("WiFi conectado.");
    Serial.print("IP del ESP32: ");
    Serial.println(obtenerIP());

    return true;
}

int escanearWiFi() {
    WiFi.mode(WIFI_STA);

    Serial.println("\nEscaneando redes WiFi...");

    // Escaneo síncrono: espera hasta que termina.
    const int numeroRedes = WiFi.scanNetworks();

    if (numeroRedes < 0) {
        Serial.println("Error al escanear las redes.");
    } else if (numeroRedes == 0) {
        Serial.println("No se encontraron redes.");
    } else {
        Serial.printf("Redes encontradas: %d\n", numeroRedes);
        Serial.println("N | SSID | RSSI (dBm) | Canal | Seguridad");

        for (int i = 0; i < numeroRedes; i++) {
            Serial.printf(
                "%d | %s | %d | %d | %s\n",
                i + 1,
                WiFi.SSID(i).c_str(),
                WiFi.RSSI(i),
                WiFi.channel(i),
                WiFi.encryptionType(i) == WIFI_AUTH_OPEN
                    ? "Abierta"
                    : "Protegida"
            );
        }
    }

    // Libera la memoria ocupada por los resultados del escaneo.
    WiFi.scanDelete();

    return numeroRedes;
}

bool conectarWiFi(const char* ssid, const char* password,
                  unsigned long timeoutMs) {
    WiFi.mode(WIFI_STA);

    Serial.print("\nConectando a ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    const unsigned long inicio = millis();

    // La resta permite gestionar el desbordamiento de millis().
    while (WiFi.status() != WL_CONNECTED &&
           millis() - inicio < timeoutMs) {
        delay(250);
        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() != WL_CONNECTED) {
        // Cancela el intento al alcanzar el tiempo máximo.
        WiFi.disconnect();
        return false;
    }

    return true;
}

String obtenerIP() {
    if (WiFi.status() != WL_CONNECTED) {
        return "0.0.0.0";
    }

    return WiFi.localIP().toString();
}

// Abre una conexión con el servidor TCP del PC o dispositivo remoto.
// Devuelve true si la conexión se establece correctamente.
bool abrirConexionTCP() {
    Serial.println("Conectando al servidor TCP...");

    if (!cliente.connect(WiFi.gatewayIP(), PUERTO)) {
        Serial.println("No se pudo abrir la conexion TCP.");
        return false;
    }

    Serial.println("Conexion TCP abierta.");
    return true;
}




