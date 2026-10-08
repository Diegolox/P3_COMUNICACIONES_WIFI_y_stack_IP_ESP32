#pragma once
#include <Arduino.h>

void initMEF();
void actualizarMEF(bool reset, bool ponerEnHora);

// La MEF decide qué hora se muestra según su estado.
String obtenerHoraMEF();
