#pragma once
#include <Arduino.h>

// Inicializa el puerto serie, muestra la MAC y conecta el ESP32 al WiFi.
void init_Wifi();

// Conecta el ESP32 a la red indicada.
// Devuelve true si se conecta antes de que transcurra timeoutMs.
bool conectarWiFi(const char* ssid, const char* password,
                  unsigned long timeoutMs = 15000);

// Devuelve true si el ESP32 está conectado al WiFi.
bool wifiConectado();

// Devuelve la dirección MAC de la interfaz WiFi del ESP32.
String obtenerMAC();

// Devuelve la IP local del ESP32, o una cadena vacía si no está conectado.
String obtenerIP();