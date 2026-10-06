#pragma once

#include <Arduino.h>

void initComs();
void actualizarComs();

// Entrega una linea completa del PC. Devuelve false si aun no hay ninguna.
bool leerMensajePC(String& mensaje);

bool conexionTCPActiva();

// Se llama repetidamente; internamente limita el envio a una vez por segundo.
void enviarHoraPeriodicamente();
