#include "hal/uart.h"

// Conserva lo escrito entre llamadas hasta recibir un salto de línea.
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

// Escribe una línea en el monitor serie.
void escribirMensajeUART(const String& mensaje) {
    Serial.println(mensaje);
}
