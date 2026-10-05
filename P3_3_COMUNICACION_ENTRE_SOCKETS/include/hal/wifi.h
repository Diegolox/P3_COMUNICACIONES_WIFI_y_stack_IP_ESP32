#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include <WiFi.h>

// El objeto se define una sola vez en wifi.cpp.
extern WiFiClient cliente;

bool init_wifi();
int escanearWiFi();
bool conectarWiFi(const char* ssid, const char* password,
                  unsigned long timeoutMs = 15000);
String obtenerIP();
bool abrirConexionTCP();

#endif
