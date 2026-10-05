#pragma once

#include <Arduino.h>

void initUART();

// Devuelve true cuando se ha recibido una línea completa.
bool leerMensajeUART(String& mensaje);

void escribirMensajeUART(const String& mensaje);