#include <Arduino.h>
#include "protocol/coms.h"
#include "hal/uart.h"
#include "hal/wifi.h"
#include "network/NTP.h"
#include <time.h>

/* Estado interno del módulo: static limita estas variables a este archivo.
 * mensajePC acumula los caracteres recibidos hasta completar una línea.
 * chatActivo indica si se abrió el chat; connected() comprueba la conexión.
 * ultimoEnvioHora guarda la marca de millis() del último intento de envío.
 */
static String mensajePC;
static bool chatActivo = false;
static unsigned long ultimoEnvioHora = 0;

/* Si UART entrega un mensaje completo, lo envía al PC y muestra una copia
 * en el terminal serie. println() añade el fin de línea para que el receptor
 * pueda identificar dónde termina el mensaje dentro del flujo TCP.
 * static hace que esta función solo se pueda usar desde este archivo.
 */
static void enviarDesdeSerie() {
    String mensaje;

    if (leerMensajeUART(mensaje)) {
        cliente.println(mensaje);
        escribirMensajeUART("ESP32: " + mensaje);
    }
}

/* Lee los bytes disponibles del PC y devuelve, como máximo, una línea.
 * TCP transporta un flujo de bytes: una línea puede llegar en varios fragmentos
 * o junto con otras líneas. Por eso mensajePC conserva lo recibido entre llamadas.
 *
 * Devuelve true al completar una línea no vacía y la copia en mensaje, pasado
 * por referencia. Devuelve false si el chat está inactivo o falta una línea
 * completa; en ese caso no modifica el argumento mensaje.
 * La interpretación de comandos corresponde a la capa de control.
 */
bool leerMensajePC(String& mensaje) {
    if (!chatActivo) {
        return false;
    }

    // Procesa únicamente los bytes que ya están disponibles para leer.
    while (cliente.available() > 0) {
        char caracter = cliente.read();

        // Ignora el retorno de carro: admite líneas terminadas en \n o \r\n.
        if (caracter == '\r') {
            continue;
        }

        if (caracter == '\n') {
            // Una línea vacía se ignora. Una no vacía se entrega al llamador.
            if (mensajePC.length() > 0) {
                mensaje = mensajePC;
                mensajePC = "";
                // Las siguientes líneas quedan pendientes de otra llamada.
                return true;
            }
        } else {
            mensajePC += caracter;
        }
    }

    // El fragmento incompleto permanece en mensajePC para la próxima llamada.
    return false;
}

// La conexión se considera activa si el chat está habilitado y el cliente conectado.
bool conexionTCPActiva() {
    return chatActivo && cliente.connected();
}

/* Inicializa el chat y abre la conexión con el servidor TCP del PC.
 * Se llama una vez que la conexión Wi-Fi está disponible.
 * Si falla la apertura, deja el chat inactivo y termina la inicialización.
 */
void initComs() {
    chatActivo = false;
    mensajePC = "";

    if (!abrirConexionTCP()) {
        escribirMensajeUART("Abre el servidor TCP del PC y reinicia el ESP32.");
        return;
    }

    chatActivo = true;

    // Llama a la rutina de hora inicial; su implementación está en el módulo NTP.
    printHoraMadrid();
    // Inicia la temporización: el primer intento periódico será tras un segundo.
    ultimoEnvioHora = millis();

    escribirMensajeUART("Chat listo. Escribe un mensaje y pulsa Enter.");
}

/* Atiende el envío UART -> TCP y detecta el cierre de la conexión.
 * Debe llamarse repetidamente desde el bucle principal.
 * La recepción se atiende por separado: actualizarMEF() llama a leerMensajePC().
 * Esta función no realiza una reconexión automática.
 */
void actualizarComs() {
    if (!chatActivo) {
        // Introduce una pequeña pausa cuando no hay un chat habilitado.
        delay(10);
        return;
    }

    if (cliente.connected()) {
        enviarDesdeSerie();
    }

    if (!cliente.connected()) {
        // Libera el cliente y descarta cualquier línea que haya quedado incompleta.
        cliente.stop();
        mensajePC = "";
        chatActivo = false;
        escribirMensajeUART("Conexion TCP cerrada. Reinicia para reconectar.");
    }
}

/* Intenta enviar la hora como máximo una vez por segundo mientras hay conexión.
 * Debe llamarse repetidamente; millis() permite temporizar sin un delay(1000).
 * Se consulta el reloj local: esta función no envía una petición NTP cada segundo.
 */
void enviarHoraPeriodicamente() {
    if (!chatActivo || !cliente.connected()) {
        return;
    }

    const unsigned long ahora = millis();
    // La resta sin signo permite comprobar el intervalo aunque millis() desborde.
    if (ahora - ultimoEnvioHora < 1000) {
        return;
    }
    // Registra el intento, aunque después no se pueda obtener o enviar la hora.
    ultimoEnvioHora = ahora;

    // Timeout 0: comprueba la disponibilidad de la hora sin esperar a sincronizar.
    struct tm fechaHora;
    if (!getLocalTime(&fechaHora, 0)) {
        return;
    }

    // Obtiene el texto de la hora y solo lo envía si no está vacío.
    String hora = obtenerHoraMadrid();
    if (hora.length() > 0) {
        cliente.println(hora);
        escribirMensajeUART("Hora enviada al PC: " + hora);
    }
}
