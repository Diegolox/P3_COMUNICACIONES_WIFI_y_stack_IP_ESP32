#include <Arduino.h>
#include <ESPping.h>

bool ping_Google(){
    
    Serial.println("Haciendo ping a www.google.com...");

    if (Ping.ping("www.google.com", 4)) {
        Serial.print("Ping correcto. Tiempo medio: ");
        Serial.print(Ping.averageTime());
        Serial.println(" ms");
        return true;
    } else {
        Serial.println("Ping fallido.");
        return false;
    }
}