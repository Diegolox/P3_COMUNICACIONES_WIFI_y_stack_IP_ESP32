#include "app/mef.h"
#include "network/NTP.h"

enum Estado {
    SIN_HORA,
    EN_HORA
};

static Estado estado = SIN_HORA;

void initMEF() {
    estado = SIN_HORA;
}

void actualizarMEF(bool reset, bool ponerEnHora) {
    // Si ambos argumentos son true, Reset tiene prioridad.
    switch (estado) {
        case SIN_HORA:
            if (reset) {
                estado = SIN_HORA;
            } else if (ponerEnHora) {
                initNTP();
                estado = EN_HORA;
            }
            break;

        case EN_HORA:
            if (reset) {
                estado = SIN_HORA;
            } else if (ponerEnHora) {
                initNTP(); // Solicita de nuevo la sincronización.
            }
            break;
    }
}

String obtenerHoraMEF() {
    if (estado == SIN_HORA) return "00:00:00";

    const String hora = obtenerHoraReloj();
    // Mientras llega la primera sincronización, mostrar ceros.
    if (hora.length() == 0) return "00:00:00";
    return hora;
}
