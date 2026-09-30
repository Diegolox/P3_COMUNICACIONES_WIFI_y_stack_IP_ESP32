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

Y se puede ver en la siguiente imagen como se ha desarrollado el código para mostrar las redes disponibles y el resultado obtenido.
```` wifi.cpp
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
````

### Ping a google.com
Uno de los objetivos era comprobar la correcta conexión a internet mediante un ping a google. Se ha desarrollado  la siguiente función que se llama de forma periódica:
```` ping.cpp
#include <Arduino.h>
#include <ESPping.h>

bool ping_Google(){
    
    Serial.println("Haciendo ping a www.google.com...");

    if (Ping.ping("www.google.com", 4)) {
        Serial.print("Ping correcto. Tiempo medio: ");
        Serial.print(Ping.averageTime());
        Serial.println(" ms");
        return true;
    } else {
        Serial.println("Ping fallido.");
        return false;
    }
}
````

