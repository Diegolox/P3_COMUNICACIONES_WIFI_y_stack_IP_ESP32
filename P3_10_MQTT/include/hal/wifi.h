#pragma one

#include <Arduino.h>
#include <WiFi.h>

// Edita estos datos con los del punto de acceso del PC.
static const char* SSID = "PCDEDIEGO 7252";
static const char* PASSWORD = "5245R@3d";
static const uint16_t PUERTO = 5000;

// El objeto se define una sola vez en wifi.cpp.
extern WiFiClient cliente;

bool init_wifi();
int escanearWiFi();
bool conectarWiFi(const char* ssid, const char* password,
                  unsigned long timeoutMs = 15000);
String obtenerIP();
bool abrirConexionTCP();

