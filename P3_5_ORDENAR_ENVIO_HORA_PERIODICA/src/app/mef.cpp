#include <Arduino.h>
#include "app/mef.h"
#include "protocol/coms.h"
#include "hal/uart.h"

// La MEF tiene dos estados y comienza sin enviar la hora.
enum EstadoMEF { PARADO, ENVIANDO };
static EstadoMEF estado = PARADO;

// Los mensajes distintos de start y stop se muestran, pero no cambian el estado.
static void procesarComando(const String& comando) {
    if (comando == "start") {
        estado = ENVIANDO;
        escribirMensajeUART("Envio de hora activado.");
    } else if (comando == "stop") {
        estado = PARADO;
        escribirMensajeUART("Envio de hora detenido.");
    }
}

void initMEF() {
    estado = PARADO;
    escribirMensajeUART("Control listo. Envia start o stop desde el PC.");
}

void actualizarMEF() {
    String mensaje;

    // Procesa todas las lineas completas recibidas en esta vuelta.
    // Asi un stop pendiente se atiende antes de enviar otra hora.
    while (leerMensajePC(mensaje)) {
        escribirMensajeUART("PC: " + mensaje);
        procesarComando(mensaje);
    }

    if (!conexionTCPActiva()) {
        estado = PARADO;
        return;
    }

    if (estado == ENVIANDO) {
        enviarHoraPeriodicamente();
    }
}
