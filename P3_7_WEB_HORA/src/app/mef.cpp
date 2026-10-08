#include "app/mef.h"

enum Estado {
    SIN_HORA,
    EN_HORA
};

// Solo este archivo puede modificar el estado de la MEF.
static Estado estado = SIN_HORA;

void initMEF() {
    estado = SIN_HORA;
}


void actualizarMEF(bool reset, bool ponerEnHora) {
    switch (estado) {

        case SIN_HORA:
            if (reset) {
                // Poner el reloj a 00:00:00.
            } else if (ponerEnHora) {
                // Sincronizar la hora.
                estado = EN_HORA;
            }
            break;

        case EN_HORA:
            if (reset) {
                // Poner el reloj a 00:00:00.
                estado = SIN_HORA;
            } else if (ponerEnHora) {
                // Volver a sincronizar la hora.
            }
            break;
    }
}
