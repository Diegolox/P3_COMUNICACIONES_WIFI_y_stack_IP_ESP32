#include <Arduino.h>
#include "hal/uart.h"
#include "hal/wifi.h"

// Acumula lo recibido por TCP hasta encontrar un salto de linea.
static String mensajePC;
static bool chatActivo = false;

// Une la entrada UART con la salida TCP.
void enviarDesdeSerie() {
    String mensaje;

    if (leerMensajeUART(mensaje)) {
        cliente.println(mensaje);
        escribirMensajeUART("ESP32: " + mensaje);
    }
}

// Une la entrada TCP con la salida UART, sin esperar datos bloqueando.
void recibirDesdePC() {
    while (cliente.available() > 0) {
        char caracter = cliente.read();

        if (caracter == '\r') {
            continue;
        }

        if (caracter == '\n') {
            if (mensajePC.length() > 0) {
                escribirMensajeUART("PC: " + mensajePC);
                mensajePC = "";
            }
        } else {
            mensajePC += caracter;
        }
    }
}

void setup() {
    // 1. Inicializa el terminal serie.
    initUART();
    delay(1000);

    // 2. Escanea las redes, conecta al AP del PC y muestra la IP.
    if (!init_wifi()) {
        escribirMensajeUART("Revisa el punto de acceso y reinicia el ESP32.");
        return;
    }

    // 3. Abre la conexion TCP con el servidor del PC.
    if (!abrirConexionTCP()) {
        escribirMensajeUART("Abre el servidor TCP del PC y reinicia el ESP32.");
        return;
    }

    chatActivo = true;
    escribirMensajeUART("Chat listo. Escribe un mensaje y pulsa Enter.");
}

void loop() {
    if (!chatActivo) {
        delay(10);
        return;
    }

    // 4. Atiende el envio mientras la conexion sigue abierta.
    if (cliente.connected()) {
        enviarDesdeSerie();
    }

    // 5. Lee tambien los ultimos bytes si el PC acaba de cerrar la conexion.
    recibirDesdePC();

    if (!cliente.connected()) {
        cliente.stop();
        mensajePC = "";
        chatActivo = false;
        escribirMensajeUART("Conexion TCP cerrada. Reinicia para reconectar.");
    }
}
