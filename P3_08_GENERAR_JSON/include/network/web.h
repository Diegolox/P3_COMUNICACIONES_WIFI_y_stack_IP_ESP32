#pragma once

// Arranca el servidor HTTP en el puerto 80 después de conectar el WiFi.
void initWeb();

// Atiende las peticiones del navegador. Llamar continuamente desde loop().
void actualizarWeb();

// Estas funciones se ejecutan cuando el ESP32 recibe cada pulsación.
// Por ahora están vacías: no reinician el ESP32 ni modifican la hora.
void alPulsarReset();
void alPulsarPonerEnHora();
