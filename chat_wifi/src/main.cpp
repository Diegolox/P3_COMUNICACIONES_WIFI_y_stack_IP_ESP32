#include <Arduino.h>
#include <WiFi.h>

// Editar con los datos del punto de acceso del portátil.
const char *SSID = "NOMBRE_DEL_AP";
const char *PASSWORD = "CONTRASENA_DEL_AP";
const uint16_t PORT = 5000;
const size_t MAX_LINE = 512;

WiFiClient cliente;
String entradaSerie;
String entradaTCP;
bool chatActivo = false;
bool descartarSerie = false;

void cerrarChat() {
    cliente.stop();
    chatActivo = false;
    Serial.println("\nChat terminado. Reinicia el ESP32 para volver a conectar.");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Conectando al AP");
    unsigned long inicio = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - inicio < 20000) {
        delay(250);
        Serial.print(".");
    }
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nNo se pudo conectar al WiFi. Reinicia para intentar otra vez.");
        WiFi.disconnect();
        return;
    }
    Serial.print("\nIP del ESP32: ");
    Serial.println(WiFi.localIP());
    // En el AP de Windows, la puerta de enlace es normalmente el portátil.
    // Si no coincide, sustituir por IPAddress(a, b, c, d) del adaptador AP.
    IPAddress servidor = WiFi.gatewayIP();
    Serial.print("Servidor TCP: ");
    Serial.println(servidor);
    if (!cliente.connect(servidor, PORT)) {
        Serial.println("No se pudo conectar al servidor. Reinicia para intentar otra vez.");
        return;
    }
    chatActivo = true;
    Serial.println("Chat conectado. Escribe con final de línea LF o CRLF. /salir cierra.");
}

void loop() {
    if (!chatActivo) {
        delay(20);
        return;
    }

    // Leer bytes y acumular hasta '\n': una lectura TCP no equivale
    // necesariamente a un mensaje completo.
    while (cliente.available()) {
        char c = cliente.read();
        if (c == '\n') {
            Serial.print("Portátil> ");
            Serial.println(entradaTCP);
            entradaTCP = "";
        } else {
            entradaTCP += c;
            if (entradaTCP.length() > MAX_LINE) {
                Serial.println("Mensaje TCP demasiado largo.");
                cerrarChat();
                return;
            }
        }
    }
    if (WiFi.status() != WL_CONNECTED || !cliente.connected()) {
        cerrarChat();
        return;
    }

    // Entrada desde el monitor serie, sin readStringUntil bloqueante.
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\r') continue;
        if (c == '\n') {
            if (!descartarSerie && entradaSerie.length()) {
                if (entradaSerie == "/salir") {
                    cerrarChat();
                    return;
                }
                String trama = entradaSerie + "\n";
                if (cliente.write((const uint8_t *)trama.c_str(), trama.length()) != trama.length()) {
                    cerrarChat();
                    return;
                }
                Serial.print("Tú> ");
                Serial.println(entradaSerie);
            }
            entradaSerie = "";
            descartarSerie = false;
        } else if (!descartarSerie) {
            entradaSerie += c;
            if (entradaSerie.length() > MAX_LINE) {
                Serial.println("Máximo 512 bytes. Se descarta esta línea.");
                entradaSerie = "";
                descartarSerie = true;
            }
        }
    }
    delay(1);
}
