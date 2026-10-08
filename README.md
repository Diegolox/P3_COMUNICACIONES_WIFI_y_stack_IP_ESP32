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
### Conexión a servidor NTP
Se ha desarrollado este código para conectarse a un servidor NTP. Un servidor NTP es un equipo de la red que proporciona la hora para sincronizar el reloj de otros dispositivos, nos servirá en este caso para saber la hora actual en Madrid.

```NTP.cpp
bool mostrarHoraMadrid() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("NTP: el ESP32 no esta conectado al WiFi.");
        return false;
    }

    // CET en invierno y CEST en verano.
    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org");

    struct tm fechaHora;

    // Espera hasta 10 segundos a que llegue la hora por NTP.
    if (!getLocalTime(&fechaHora, 10000)) {
        Serial.println("NTP: no se pudo obtener la hora.");
        return false;
    }

    Serial.print("Fecha y hora de Madrid: ");
    Serial.println(&fechaHora, "%d/%m/%Y %H:%M:%S");

    return true;
}
```
```text
1:53:23.941 > WiFi conectado.
21:53:23.953 > IP del ESP32: 192.168.1.155
21:53:26.789 > Fecha y hora de Madrid: 30/09/2026 21:53:26
```



### Chat entre el ESP32 y el PC mediante sockets TCP

Se ha desarrollado un chat para intercambiar mensajes entre el ESP32 y un PC. En este caso, el PC genera la red Wi-Fi y ejecuta un servidor TCP en Python en el puerto `5000`. El ESP32 se conecta a esa red y abre la conexión con el servidor mediante la siguiente llamada:

```cpp
cliente.connect(WiFi.gatewayIP(), PUERTO);
```

Una vez establecida la conexión, los mensajes escritos en el monitor serie del ESP32 se envían al PC. Para ello, se utiliza la siguiente función dentro de `coms.cpp`:

```cpp
static void enviarDesdeSerie() {
    String mensaje;

    if (leerMensajeUART(mensaje)) {
        cliente.println(mensaje);
        escribirMensajeUART("ESP32: " + mensaje);
    }
}
```

También se reciben los mensajes enviados desde el terminal del PC y se muestran en el monitor serie del ESP32. En ambos sentidos se utiliza un salto de línea para indicar el final de cada mensaje.

El código se ha separado en los módulos `uart`, `wifi` y `coms`, manteniendo la estructura de capas de los apartados anteriores. Desde el `loop()` se llama continuamente a `actualizarComs()`, que gestiona el envío, la recepción y el cierre de la conexión. Para utilizar el chat, primero se ejecuta el servidor Python y después se arranca el ESP32.



### Envío periódico de datos del IMU por Wi-Fi

En este caso, el ESP32 lee el sensor **BNO055 mediante I²C** y envía sus datos al ordenador a través de una **conexión TCP sobre Wi-Fi**. Ambos dispositivos están conectados a la misma red: el ordenador actúa como servidor TCP y el ESP32 se conecta como cliente.

El ESP32 envía **una muestra cada segundo**, con las aceleraciones lineales de los tres ejes, en m/s² y sin la componente de gravedad. Cada mensaje utiliza el formato `accX;accY;accZ` y termina con un salto de línea, que permite al programa Python identificar cada muestra recibida.

El código se organiza en módulos: `hal/wifi` gestiona la conexión de red, `sensors/BNO055` realiza las lecturas y `protocol/coms` prepara y transmite los mensajes. La función de envío utiliza `millis()` para controlar el intervalo entre muestras.

Los datos recibidos pueden visualizarse desde el ordenador mediante una aplicación en Python, como se muestra en el siguiente vídeo.

**Vídeo de demostración**

👉 **[Añadir aquí el vídeo del programa Python mostrando los datos enviados por el ESP32]**
