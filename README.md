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
```text
21:28:36.921 > Escaneando redes WiFi...
21:28:44.329 > Redes encontradas: 31
21:28:44.329 > N | SSID | RSSI (dBm) | Canal | Seguridad
21:28:44.344 > 1 | DIGIFIBRA-6CC1 | -68 | 4 | Protegida
21:28:44.348 > 2 | MOVISTAR-WIFI6-B3D0 | -75 | 1 | Protegida
21:28:44.351 > 3 | MOVISTAR-WIFI6-2EC0 | -77 | 6 | Protegida
21:28:44.356 > 4 | MOVISTAR_F9CD | -78 | 1 | Protegida
21:28:44.359 > 5 | MIWIFI_Dtvm | -79 | 11 | Protegida
21:28:44.364 > 6 | MOVISTAR-WIFI6-8858 | -80 | 1 | Protegida
21:28:44.367 > 7 | MOVISTAR-WIFI6-B878 | -81 | 8 | Protegida
21:28:44.370 > 8 | MiFibra-C086 | -82 | 1 | Protegida
21:28:44.372 > 9 | MOVISTAR_C6F0_Domotica | -82 | 2 | Protegida
21:28:44.377 > 10 | DIGI-6CC1_EXT | -82 | 4 | Protegida
21:28:44.383 > 11 | DIGIFIBRA-GkKK | -83 | 9 | Protegida
21:28:44.385 > 12 | MOVISTAR_C820 | -84 | 1 | Protegida
21:28:44.387 > 13 | HUAWEI-104N6B | -85 | 6 | Protegida
21:28:44.392 > 14 | DIGI-u4uY | -85 | 10 | Protegida
21:28:44.395 > 15 | DIGIFIBRA-95PH | -86 | 1 | Protegida
21:28:44.399 > 16 | MOVISTAR_C6F0 | -86 | 2 | Protegida
21:28:44.403 > 17 | HUAWEI-104N6B_Wi-Fi5 | -86 | 6 | Protegida
21:28:44.406 > 18 | MIWIFI_KPfG-2G | -87 | 6 | Protegida
21:28:44.410 > 19 | MOVISTAR-WIFI6-3468 | -87 | 6 | Protegida
21:28:44.414 > 20 | MOVISTAR_4E8F | -87 | 11 | Protegida
21:28:44.418 > 21 | DIGIFIBRA-9xPc | -88 | 3 | Protegida
21:28:44.422 > 22 | MIWIFI_4Fpy | -88 | 6 | Protegida
21:28:44.422 > 23 | MOVISTAR-WIFI6-0DB0 | -88 | 6 | Protegida
21:28:44.430 > 24 | MOVISTAR-WIFI6-B0F8 | -89 | 6 | Protegida
21:28:44.433 > 25 | DIRECT-94-HP ENVY 5000 series | -89 | 6 | Protegida
21:28:44.438 > 26 | MOVISTAR_0FC8 | -89 | 11 | Protegida
21:28:44.439 > 27 | Livebox7-381F-WiFi7 | -90 | 11 | Protegida
21:28:44.446 > 28 | Livebox7-381F | -90 | 11 | Protegida
21:28:44.450 > 29 | MOVISTAR_B3D0 | -91 | 1 | Protegida
21:28:44.453 > 30 | MOVISTAR_C6F0_Domotica | -93 | 2 | Protegida
21:28:44.454 > 31 | DIGIFIBRA-xuM4 | -94 | 10 | Protegida
21:28:44.462 > 
```

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

