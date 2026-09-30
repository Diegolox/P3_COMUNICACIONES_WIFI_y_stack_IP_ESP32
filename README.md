## Introducción

Esta práctica aborda la conexión del ESP32 a una red WiFi y los fundamentos de las comunicaciones basadas en el stack TCP/IP. Se comienza identificando la dirección MAC del dispositivo, estableciendo la conexión con un punto de acceso y consultando la dirección IP asignada.


## Conexión Wi-Fi
### Obtención de la IP del ESP32 y escaneo de redes disponibles
Primeramente se ha realizado un codigo sencillo con la estructura de capas de los repositorios anteriores. En este caso el objetivo es configurar el ESP32 como STA y conectarlo a una wifi con un determinado SSID. Además realiza un escaneo de las redes disponibles y entre otros parámetros indica si están protegidas o no, la intensidad con la que llega la señal o el nombre del punto de acceso. 

Se puede ver el código completo en `P3_1_IDENTIFICAR_IP`, pero en el siguiente fragmento se muestran las funciones diseñadas para conectarse a la red Wi-Fi y para obtener la IP del ESP32.

```` wifi.cpp
/* Conecta al AP y espera hasta conectarse o agotar el tiempo máximo.
 * Para una red abierta, utiliza "" como contraseña.
 */
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

/* Devuelve la IP del ESP32 como texto.
 * Si no está conectado, devuelve "0.0.0.0".
 */
String obtenerIP() {
    if (WiFi.status() != WL_CONNECTED) {
        return "0.0.0.0";
    }

    return WiFi.localIP().toString();
}
````
