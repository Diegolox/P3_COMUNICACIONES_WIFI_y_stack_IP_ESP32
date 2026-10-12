#include "network/MQTT.h"
#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

// ---------- CONFIGURACIÓN: cambia estos tres valores ----------
#define AIO_USERNAME "Diegolox"
#define AIO_KEY      ""
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

// Escucha los datos publicados en el mismo feed.
Adafruit_MQTT_Subscribe suscripcion(
    &mqtt, AIO_USERNAME "/feeds/" AIO_FEED_KEY
);

}  // namespace

bool initMQTT(bool suscriptor) {
    static bool suscripcionRegistrada = false;

    // Si se activa la recepción, registra el feed una sola vez.
    if (suscriptor && !suscripcionRegistrada) {
        if (!mqtt.subscribe(&suscripcion)) {
            Serial.println("MQTT: no se pudo registrar la suscripcion");
            return false;
        }

        suscripcionRegistrada = true;

        // Si ya estaba conectado, reconecta para solicitar
        // al broker la nueva suscripción.
        mqtt.disconnect();
    }

    if (WiFi.status() != WL_CONNECTED) {
        mqtt.disconnect();
        Serial.println("MQTT: no hay conexion WiFi");
        return false;
    }

    if (mqtt.connected()) {
        return true;
    }

    Serial.println("MQTT: conectando con Adafruit IO...");

    const int8_t resultado = mqtt.connect();

    if (resultado != 0) {
        Serial.print("MQTT: error de conexion: ");
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

String recibirMQTT() {
    if (!initMQTT(true)) {
        return "";
    }

    String dato = "";
    Adafruit_MQTT_Subscribe* mensaje;

    while ((mensaje = mqtt.readSubscription(10)) != nullptr) {
        if (mensaje == &suscripcion) {
            dato = reinterpret_cast<const char*>(suscripcion.lastread);
        }
    }

    // Devuelve el último dato recibido o "" si no recibió ninguno.
    return dato;
}