#ifndef UART_H
#define UART_H

#include <Arduino.h>

void initUART();
bool leerMensajeUART(String& mensaje);
void escribirMensajeUART(const String& mensaje);

#endif
