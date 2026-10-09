#ifndef UART_H
#define UART_H

#include <Arduino.h>

void initUART();
bool leerMensajeUART(String& mensaje);
void escribirMensajeUART(const String& mensaje);

// Obtiene la hora de Madrid y la muestra usando escribirMensajeUART().
void printHoraMadrid();

#endif
