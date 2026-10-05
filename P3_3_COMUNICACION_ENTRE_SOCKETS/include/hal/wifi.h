#pragma once

#include <Arduino.h>

// Credenciales del punto de acceso.
constexpr const char* SSID = "PCDEDIEGO 7252";
constexpr const char* PASSWORD = "5245R@3d";

// Puerto TCP para la comunicación con el servidor.
const uint16_t PUERTO = 5000;

// Objeto que permite abrir la conexión TCP, enviar y recibir.
WiFiClient cliente;


// Configura el modo STA, escanea las redes y después se conecta.
// Devuelve true si la conexión se establece correctamente.

bool init_wifi();

// Escanea y muestra las redes por Serial.
// Devuelve el número de redes encontradas, o un valor negativo si falla.
 
int escanearWiFi();

// Conecta al AP y espera hasta conectarse o agotar el tiempo máximo.
// Para una red abierta, utiliza "" como contraseña.

bool conectarWiFi(const char* ssid, const char* password,
                  unsigned long timeoutMs = 15000);

// Devuelve la IP del ESP32 como texto.
// Si no está conectado, devuelve "0.0.0.0".

String obtenerIP();