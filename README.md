## Introducción

Esta práctica aborda la conexión del ESP32 a una red WiFi y los fundamentos de las comunicaciones basadas en el stack TCP/IP. Se comienza identificando la dirección MAC del dispositivo, estableciendo la conexión con un punto de acceso y consultando la dirección IP asignada.


## Conexión Wi-Fi
### Obtención de la MAC e IP
```` wifi.cpp
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
````
