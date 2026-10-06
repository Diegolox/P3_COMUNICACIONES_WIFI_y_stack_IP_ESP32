#include <Arduino.h>
#include "protocol/coms.h"
#include "hal/uart.h"
#include "hal/wifi.h"

// Estas variables pertenecen al chat y solo se usan en este archivo.
static String mensajePC;
static bool chatActivo = false;

// Lee una linea por UART y la envia al PC por TCP.
static void enviarDesdeSerie() {
    String mensaje;

    if (leerMensajeUART(mensaje)) {
        cliente.println(mensaje);
        escribirMensajeUART("ESP32: " + mensaje);
    }
}

// Acumula los bytes TCP hasta completar una linea y la muestra por UART.
static void recibirDesdePC() {
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

// Abre el socket TCP cuando la conexion Wi-Fi ya esta disponible.
void initComs() {
    chatActivo = false;
    mensajePC = "";

    if (!abrirConexionTCP()) {
        escribirMensajeUART("Abre el servidor TCP del PC y reinicia el ESP32.");
        return;
    }

    chatActivo = true;
    escribirMensajeUART("Chat listo. Escribe un mensaje y pulsa Enter.");
}

// Atiende las dos direcciones y detecta el cierre de la conexion.
void actualizarComs() {
    if (!chatActivo) {
        delay(10);
        return;
    }

    if (cliente.connected()) {
        enviarDesdeSerie();
    }

    // Lee tambien los ultimos bytes si el PC acaba de cerrar el socket.
    recibirDesdePC();

    if (!cliente.connected()) {
        cliente.stop();
        mensajePC = "";
        chatActivo = false;
        escribirMensajeUART("Conexion TCP cerrada. Reinicia para reconectar.");
    }
}
