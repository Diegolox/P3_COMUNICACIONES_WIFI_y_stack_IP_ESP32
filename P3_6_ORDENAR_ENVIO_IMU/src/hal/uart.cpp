#include "hal/uart.h"
#include "network/NTP.h"

// Conserva lo escrito entre llamadas hasta recibir un salto de linea.
static String entradaSerie;

void initUART() {
    Serial.begin(115200);
}

// Lee sin bloquear. El mensaje termina al recibir '\n'.
bool leerMensajeUART(String& mensaje) {
    while (Serial.available() > 0) {
        char caracter = Serial.read();

        if (caracter == '\r') {
            continue;
        }

        if (caracter == '\n') {
            if (entradaSerie.length() > 0) {
                mensaje = entradaSerie;
                entradaSerie = "";
                return true;
            }
        } else {
            entradaSerie += caracter;
        }
    }

    return false;
}

// Escribe una linea en el monitor serie.
void escribirMensajeUART(const String& mensaje) {
    Serial.println(mensaje);
}

// Reutiliza la salida UART para mostrar la hora devuelta por NTP.
void printHoraMadrid() {
    String hora = obtenerHoraMadrid();

    if (hora.length() == 0) {
        escribirMensajeUART("NTP: no se pudo obtener la hora. Revisa WiFi e Internet.");
        return;
    }

    escribirMensajeUART("Fecha y hora de Madrid: " + hora);
}
