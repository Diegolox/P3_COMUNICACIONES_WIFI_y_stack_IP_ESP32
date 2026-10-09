#include <Arduino.h>
#include "sensors/temp.h"

String generar_temp(){
    float temp = random(0, 50 + 1) + random(0, 50 + 1) / 100.0;

    String json_temp = generarJSON(temp);
    return json_temp;
}



String generarJSON(float dato) {
    String json = "[{\"n\":\"temp\",\"u\":\"Cel\",\"v\":";
    json += String(dato, 2);  // Valor con 2 decimales.
    json += "}]";
    return json;
}