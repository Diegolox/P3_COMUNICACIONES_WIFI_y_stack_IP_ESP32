## Introducción

Esta práctica aborda la conexión del ESP32 a una red WiFi y los fundamentos de las comunicaciones basadas en el stack TCP/IP. Se comienza identificando la dirección MAC del dispositivo, estableciendo la conexión con un punto de acceso y consultando la dirección IP asignada.


## Conexión Wi-Fi
### Obtención de la IP del ESP32 y escaneo de redes disponibles
```` wifi.cpp
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
````
