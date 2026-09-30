## Introducción

Esta práctica aborda la conexión del ESP32 a una red WiFi y los fundamentos de las comunicaciones basadas en el stack TCP/IP. Se comienza identificando la dirección MAC del dispositivo, estableciendo la conexión con un punto de acceso y consultando la dirección IP asignada.


## Conexión Wi-Fi
### Obtención de la IP del ESP32 y escaneo de redes disponibles
Primeramente se ha realizado un codigo sencillo con la estructura de capas de los repositorios anteriores. En este caso el objetivo es configurar el ESP32 como STA y conectarlo a una wifi con un determinado SSID. Además realiza un escaneo de las redes disponibles y entre otros parámetros indica si están protegidas o no, la intensidad con la que llega la señal o el nombre del punto de acceso.

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
