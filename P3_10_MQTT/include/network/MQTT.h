#pragma once
#include <Arduino.h>

// Requiere que el ESP32 ya tenga conexión WiFi.
// Devuelve true si MQTT está conectado; false si no puede conectar.
// Hace un intento de conexión por llamada.
// MQTT.h
bool initMQTT(bool suscriptor = false);

// Publica un número en el feed configurado en MQTT.cpp.
// Si MQTT está desconectado, intenta conectarlo primero.
// Devuelve true si publish() tiene éxito; false si falla.
// Para un único feed, llamar cada 5 segundos es un buen punto de partida.
bool envioMQTT(float dato);

String recibirMQTT();