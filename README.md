## Introducción

Esta práctica aborda la conexión del ESP32 a una red WiFi y los fundamentos de las comunicaciones basadas en el stack TCP/IP. Se comienza identificando la dirección MAC del dispositivo, estableciendo la conexión con un punto de acceso y consultando la dirección IP asignada.

### Conceptos fundamentales

| Concepto | Descripción |
|---|---|
| **WiFi** | Tecnología de comunicación inalámbrica que permite conectar el ESP32 a una red local. Estar conectado al WiFi no implica necesariamente tener acceso a Internet. |
| **SSID y contraseña** | El SSID identifica la red WiFi. La contraseña permite autenticarse en las redes protegidas. |
| **Estación (STA)** | Modo en el que el ESP32 se conecta a un punto de acceso existente, por ejemplo, el de un router. Es el modo utilizado en el primer ejercicio. |
| **Punto de acceso (AP)** | Modo en el que el ESP32 crea una red WiFi a la que pueden conectarse otros dispositivos. |
| **Dirección MAC** | Identificador de una interfaz de red, normalmente de 48 bits y representado mediante seis pares hexadecimales. Se utiliza en las comunicaciones dentro del enlace local. |
| **Dirección IP** | Dirección lógica de una interfaz dentro de una red IP. Permite dirigir paquetes hacia el dispositivo y puede cambiar según la configuración de la red. |
| **DHCP** | Protocolo que proporciona automáticamente una dirección IP y otros parámetros de red, como la máscara de subred y la puerta de enlace. |
| **Máscara de subred** | Permite determinar qué parte de una dirección IPv4 identifica la red y qué parte identifica al dispositivo dentro de ella. |
| **Puerta de enlace** | Dispositivo al que se envían los paquetes destinados a otras redes; habitualmente es el router. |
| **DNS** | Sistema que permite resolver nombres de dominio, como `example.com`, en direcciones IP. |
| **Stack TCP/IP** | Conjunto de protocolos organizado por capas: enlace, Internet, transporte y aplicación. WiFi proporciona el enlace inalámbrico; IP permite el direccionamiento y el encaminamiento. |
| **TCP y UDP** | TCP proporciona un flujo de bytes fiable y ordenado mediante una conexión. UDP envía datagramas sin garantizar su entrega ni su orden. |
| **Puertos y sockets** | Los puertos identifican servicios o aplicaciones dentro de un dispositivo. Los sockets permiten a los programas enviar y recibir datos a través de la red. |
| **Cliente y servidor** | El cliente inicia una comunicación o solicita un servicio; el servidor escucha y atiende las solicitudes. Estos roles son independientes de los modos WiFi STA y AP. |

### CONEXIÓN WIFI
#### OBTENCIÓN DE LA MAC E IP
````
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
