#include <Arduino.h>
#include "protocol/coms.h"
#include "hal/uart.h"
#include "hal/wifi.h"
#include "network/NTP.h"
#include <time.h>

// Estas variables pertenecen al chat y solo se usan en este archivo.
static String mensajePC;
static bool chatActivo = false;
static unsigned long ultimoEnvioHora = 0;

// Lee una linea por UART y la envia al PC por TCP.
static void enviarDesdeSerie() {
    String mensaje;

    if (leerMensajeUART(mensaje)) {
        cliente.println(mensaje);
        escribirMensajeUART("ESP32: " + mensaje);
    }
}

// Conserva los fragmentos TCP hasta encontrar el fin de un mensaje.
// No interpreta comandos: esa decision pertenece a la capa de control.
bool leerMensajePC(String& mensaje) {
    if (!chatActivo) {
        return false;
    }

    while (cliente.available() > 0) {
        char caracter = cliente.read();

        if (caracter == '\r') {
            continue;
        }

        if (caracter == '\n') {
            if (mensajePC.length() > 0) {
                mensaje = mensajePC;
                mensajePC = "";
                return true;
            }
        } else {
            mensajePC += caracter;
        }
    }

    return false;
}

bool conexionTCPActiva() {
    return chatActivo && cliente.connected();
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

    // Configura NTP y obtiene la hora inicial una vez al arrancar.
    printHoraMadrid();
    ultimoEnvioHora = millis();

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

    // La recepcion de comandos se atiende desde actualizarMEF().

    if (!cliente.connected()) {
        cliente.stop();
        mensajePC = "";
        chatActivo = false;
        escribirMensajeUART("Conexion TCP cerrada. Reinicia para reconectar.");
    }
}

// Se llama continuamente, pero solo envia la hora una vez por segundo.
void enviarHoraPeriodicamente() {
    if (!chatActivo || !cliente.connected()) {
        return;
    }

    const unsigned long ahora = millis();
    if (ahora - ultimoEnvioHora < 1000) {
        return;
    }
    ultimoEnvioHora = ahora;

    // Evita esperar 10 segundos si el reloj aun no se ha sincronizado.
    struct tm fechaHora;
    if (!getLocalTime(&fechaHora, 0)) {
        return;
    }

    String hora = obtenerHoraMadrid();
    if (hora.length() > 0) {
        cliente.println(hora);
        escribirMensajeUART("Hora enviada al PC: " + hora);
    }
}
