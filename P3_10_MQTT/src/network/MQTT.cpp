#include "network/MQTT.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

// ---------- CONFIGURACIÓN: cambia estos tres valores ----------
#define AIO_USERNAME "Diegolox"
#define AIO_KEY      "aio_Snkj82CQloZhQlTxgRax8631HYVX"
#define AIO_FEED_KEY "Temperatura"

// Esta versión usa MQTT sin TLS: el puerto 1883 no cifra los datos
// ni las credenciales. No publicar una AIO Key real en GitHub.

namespace {

// Transporte TCP a través del WiFi que ya conecta tu proyecto.
WiFiClient clienteWiFi;

// Cliente MQTT: servidor, puerto, usuario y contraseña (AIO Key).
Adafruit_MQTT_Client mqtt(
    &clienteWiFi, "io.adafruit.com", 1883, AIO_USERNAME, AIO_KEY
);

// El topic se construye automáticamente: TU_USUARIO/feeds/temperatura.
// Los objetos quedan privados dentro de este archivo.
Adafruit_MQTT_Publish feed(
    &mqtt, AIO_USERNAME "/feeds/" AIO_FEED_KEY
);

}  // namespace

bool initMQTT() {
    // MQTT necesita una conexión de red previa.
    if (WiFi.status() != WL_CONNECTED) {
        mqtt.disconnect();
        Serial.println("MQTT: no hay conexión WiFi");
        return false;
    }

    // Conserva la conexión existente.
    if (mqtt.connected()) {
        return true;
    }

    Serial.println("MQTT: conectando con Adafruit IO...");

    // Adafruit MQTT devuelve 0 cuando el broker acepta la conexión.
    const int8_t resultado = mqtt.connect();

    if (resultado != 0) {
        Serial.print("MQTT: error de conexión: ");
        Serial.println(mqtt.connectErrorString(resultado));
        mqtt.disconnect();
        return false;
    }

    Serial.println("MQTT: conectado");
    return true;
}

bool envioMQTT(float dato) {
    // Comprueba WiFi y MQTT; reconecta si hace falta.
    if (!initMQTT()) {
        return false;
    }

    // El objeto feed ya conoce el topic. Solo se le pasa el dato.
    // QoS 0: true indica que la biblioteca envió el paquete,
    // no una confirmación de que Adafruit haya guardado la medida.
    const bool enviado = feed.publish(static_cast<double>(dato));

    if (!enviado) {
        Serial.println("MQTT: error al publicar");
        // La próxima llamada intentará establecer una conexión nueva.
        mqtt.disconnect();
    }

    return enviado;
}

