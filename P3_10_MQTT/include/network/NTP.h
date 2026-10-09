#pragma once
#include <Arduino.h>

// Inicia la sincronización en segundo plano; llamar con WiFi conectado.
void initNTP();

// Fecha y hora completas, conservadas para el resto del proyecto.
String obtenerHoraMadrid();

// Solo HH:MM:SS. Devuelve "" si todavía no hay hora disponible.
String obtenerHoraReloj();
