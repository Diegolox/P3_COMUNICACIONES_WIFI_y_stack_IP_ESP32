<div align="justify">

# Práctica 3 · Comunicaciones Wi-Fi y stack IP con ESP32

<p align="center"><strong>ESP32 · Wi-Fi · TCP/IP · NTP · HTTP · SenML · FTP · MQTT</strong></p>

En esta práctica se trabaja con un ESP32 conectado a una red Wi-Fi y se desarrollan distintas aplicaciones sobre el stack IP: comprobación de conectividad mediante ping, sincronización de hora con NTP, comunicación bidireccional mediante sockets TCP, transmisión de datos de una IMU, servidor web, generación de datos JSON con campos SenML, transferencia de archivos mediante FTP y comunicación MQTT con Adafruit IO.

El trabajo está dividido en proyectos independientes de PlatformIO. Cada carpeta conserva una etapa del desarrollo y permite cargar en el ESP32 la aplicación correspondiente. Se mantiene la organización por capas utilizada en las prácticas anteriores, separando el acceso al hardware, los sensores, las comunicaciones y el control de la aplicación.

## Organización y arquitectura

| **Carpeta** | **Aplicación** |
| --- | --- |
| [P3_01_IDENTIFICAR_IP](P3_01_IDENTIFICAR_IP/) | Escaneo Wi-Fi, conexión como estación, consulta de IP y ping a Google. |
| [P3_02_CONEXION_SERVER_NTP](P3_02_CONEXION_SERVER_NTP/) | Obtención de fecha y hora local mediante NTP. |
| [P3_03_COMUNICACION_ENTRE_SOCKETS](P3_03_COMUNICACION_ENTRE_SOCKETS/) | Chat entre el ESP32 y un servidor TCP en Python. |
| [P3_04_ENVIO_HORA_PERIODICA](P3_04_ENVIO_HORA_PERIODICA/) | Envío de fecha y hora al PC una vez por segundo. |
| [P3_05_ORDENAR_ENVIO_HORA_PERIODICA](P3_05_ORDENAR_ENVIO_HORA_PERIODICA/) | Control del envío con los comandos `start` y `stop`. |
| [P3_06_ORDENAR_ENVIO_IMU](P3_06_ORDENAR_ENVIO_IMU/) | Lectura del BNO055 y transmisión de orientación y aceleración por TCP. |
| [P3_07_WEB_HORA](P3_07_WEB_HORA/) | Servidor web con reloj y botones Reset y Poner en hora. |
| [P3_08_GENERAR_JSON](P3_08_GENERAR_JSON/) | Generación de temperaturas simuladas en JSON. |
| [P3_09_SERVER_FTP](P3_09_SERVER_FTP/) | Envío de archivos JSON a FileZilla Server. |
| [P3_10_MQTT](P3_10_MQTT/) | Publicación y suscripción a un feed de Adafruit IO mediante MQTT. |
| [BNO055_IMU_Studio](BNO055_IMU_Studio/) | Interfaz Python para visualizar y registrar la telemetría. |
| [chat_wifi](chat_wifi/) | Versión alternativa del chat TCP para un portátil Windows. |

### Esquema de la arquitectura

<picture>
  <source media="(prefers-color-scheme: dark)"
          srcset="doc/img/ARQUITECTURA_MODO_OSCURO.svg">
  <source media="(prefers-color-scheme: light)"
          srcset="doc/img/ARQUITECTURA_MODO_CLARO.svg">
  <img src="doc/img/ESQUEMA_CLARO.svg"
       alt="Estructura de archivos del proyecto">
</picture>

Las carpetas de firmware utilizan la siguiente separación de responsabilidades:

| **Módulo** | **Responsabilidad** |
| --- | --- |
| `src/main.cpp` | Inicialización y llamadas de la aplicación desde `setup()` y `loop()`. |
| `hal/wifi` | Escaneo, conexión Wi-Fi, consulta de IP y apertura del cliente TCP en los proyectos que lo utilizan. |
| `hal/uart` | Inicialización del monitor serie y lectura/escritura de mensajes. |
| `sensors/BNO055` | Inicialización y lectura de la IMU por I²C. |
| `sensors/temp` | Generación de temperaturas simuladas y serialización JSON. |
| `network` | Servicios de ping, NTP, HTTP, FTP y MQTT, según el proyecto. |
| `protocol/coms` | Preparación, envío y recepción de mensajes TCP. |
| `app/mef` | Máquina de estados del control de hora o de la página web, según el apartado. |
| `include` | Cabeceras, declaraciones de funciones y configuración. |

Cada aplicación se describe a partir de las funciones que ejecuta su propio `main.cpp`. Los módulos auxiliares se reutilizan entre carpetas a lo largo del desarrollo.

## Material y entorno de desarrollo

<details>
<summary><strong>Consultar hardware, bibliotecas y preparación de los proyectos</strong></summary>

Se utiliza un ESP32 WROOM, un ordenador con Python y, para la telemetría, un sensor BNO055 conectado mediante I²C. Los proyectos están configurados con PlatformIO, el framework Arduino y la placa `upesy_wroom`. El monitor serie funciona a **115200 baudios** y muestra marcas temporales mediante `monitor_filters = time`.

Las bibliotecas declaradas en los proyectos, según el apartado, son:

- **Adafruit BNO055:** acceso al sensor de orientación y aceleración.
- **ESPping:** comprobación de conectividad mediante ICMP.
- **esp32_ftpclient:** transferencia de archivos a un servidor FTP.
- **Adafruit MQTT Library:** conexión, publicación y suscripción MQTT.

`WiFi`, `WebServer`, `Wire` y las funciones de tiempo se utilizan a través del framework Arduino para ESP32. El monitor de la IMU utiliza Tkinter y módulos de la biblioteca estándar de Python.

Para trabajar con una etapa se abre su carpeta como proyecto de PlatformIO, se configura la red correspondiente, se compila y se carga el firmware. Los servidores del PC deben estar preparados antes de que el ESP32 intente conectarse.

</details>

---

<a name="wifi"></a>

## 01. Wi-Fi, escaneo de redes y obtención de IP

**Estación Wi-Fi · DHCP · RSSI · ping a Google**

Código: [P3_01_IDENTIFICAR_IP](P3_01_IDENTIFICAR_IP/).

El ESP32 funciona como **estación Wi-Fi (STA)**: se conecta a un punto de acceso existente, que puede ser un router o la red compartida por el portátil. El módulo `hal/wifi` concentra esta configuración.

> 📷 **AÑADIR FOTO / CAPTURA:** Captura del escaneo Wi-Fi, la IP asignada y el ping a Google.

> 🎬 **AÑADIR VÍDEO:** Vídeo opcional de la conexión y el resultado del ping.

<details>
<summary><strong>Escaneo, conexión, direcciones IP y código del ping</strong></summary>

La función `init_wifi()` realiza la secuencia de arranque:

1. Inicializa la comunicación serie.
2. Selecciona el modo `WIFI_STA`.
3. Escanea las redes disponibles.
4. Intenta conectarse al SSID configurado.
5. Muestra la IP del ESP32 si la conexión se establece.

### Escaneo

`escanearWiFi()` utiliza `WiFi.scanNetworks()` y presenta, para cada red, el SSID, la intensidad de señal RSSI en dBm, el canal y si está abierta o protegida. El RSSI permite comparar la señal recibida: por ejemplo, `-60 dBm` representa una señal más intensa que `-85 dBm`.

```cpp
Serial.printf(
    "%d | %s | %d | %d | %s\n",
    i + 1,
    WiFi.SSID(i).c_str(),
    WiFi.RSSI(i),
    WiFi.channel(i),
    WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Abierta" : "Protegida"
);
```

Al terminar, `WiFi.scanDelete()` libera la memoria utilizada por los resultados del escaneo.

### Conexión e IP

`conectarWiFi(ssid, password, timeoutMs)` inicia la conexión con `WiFi.begin()` y espera hasta que se alcance `WL_CONNECTED` o se agote el tiempo máximo, que por defecto es de 15 segundos. Para una red abierta se utiliza una contraseña vacía.

La espera se controla mediante la diferencia entre dos valores de `millis()`. Si se supera el tiempo permitido, se cancela el intento con `WiFi.disconnect()` y la función devuelve `false`.

`obtenerIP()` devuelve `WiFi.localIP().toString()` cuando existe conexión, o `"0.0.0.0"` en caso contrario. En esta configuración, la dirección IP se obtiene mediante DHCP de la red del punto de acceso y puede cambiar entre conexiones.

Conviene distinguir las direcciones utilizadas:

| **Dirección** | **Significado** |
| --- | --- |
| IP del ESP32 | Dirección que identifica a la placa dentro de la red. Se usa, por ejemplo, para abrir su página web. |
| Puerta de enlace | Dirección del equipo que proporciona salida de la red. En el montaje con el portátil como AP, se utiliza también como destino del servidor TCP. |
| `0.0.0.0` en el servidor Python | Dirección de escucha en todas las interfaces del PC. El ESP32 se conecta a la IP real del PC. |

La dirección MAC identifica la interfaz de red, mientras que la IP identifica su conexión dentro de una red concreta.

### Ping a Google

Desde `loop()` se llama a `ping_Google()`. La función realiza cuatro intentos a `www.google.com` y muestra el tiempo medio de respuesta cuando el ping tiene éxito:

```cpp
if (Ping.ping("www.google.com", 4)) {
    Serial.print("Ping correcto. Tiempo medio: ");
    Serial.print(Ping.averageTime());
    Serial.println(" ms");
    return true;
}
```

Se utiliza ICMP para comprobar que existe conectividad con un equipo externo. Al emplear un nombre de dominio, interviene también su resolución DNS. El bucle introduce una pausa de un segundo después de cada ejecución del ping.

</details>

---

<a name="ntp"></a>

## 02. Sincronización de hora mediante NTP

**pool.ntp.org · hora de Madrid · cambio CET/CEST**

Código: [P3_02_CONEXION_SERVER_NTP](P3_02_CONEXION_SERVER_NTP/).

Una vez conectado a la red, el ESP32 obtiene la hora de `pool.ntp.org`. NTP permite sincronizar el reloj del dispositivo con una referencia de tiempo de la red. La configuración utiliza la zona horaria de Madrid, con cambio automático entre CET y CEST:

> 📷 **AÑADIR FOTO / CAPTURA:** Captura de la fecha y hora sincronizadas por NTP.

<details>
<summary><strong>Configuración NTP, lectura del reloj y ejemplo de salida</strong></summary>

```cpp
configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org");
```

`mostrarHoraMadrid()` comprueba primero la conexión Wi-Fi, configura el servicio y consulta la fecha y hora con `getLocalTime()`. Se permite una espera de hasta 10 segundos para obtener una hora válida.

```cpp
struct tm fechaHora;
if (!getLocalTime(&fechaHora, 10000)) {
    Serial.println("NTP: no se pudo obtener la hora.");
    return false;
}

Serial.println(&fechaHora, "%d/%m/%Y %H:%M:%S");
```

En esta etapa se muestra la hora por el monitor serie, con una pausa de un segundo entre llamadas. En los módulos posteriores se separa la lectura del reloj de su presentación, utilizando funciones que devuelven la hora como `String` para enviarla por TCP o servirla por HTTP.

Un resultado recogido durante el desarrollo fue:

```text
WiFi conectado.
IP del ESP32: 192.168.1.155
Fecha y hora de Madrid: 30/09/2026 21:53:26
```

</details>

---

<a name="chat"></a>

## 03. Chat entre el ESP32 y el PC mediante sockets TCP

**PC servidor · ESP32 cliente · TCP 5000 · mensajes delimitados por LF**

Código: [P3_03_COMUNICACION_ENTRE_SOCKETS](P3_03_COMUNICACION_ENTRE_SOCKETS/).

El portátil genera el punto de acceso Wi-Fi y ejecuta el servidor TCP. El ESP32 se conecta como estación a esa red y actúa como cliente TCP. La comunicación permite escribir mensajes en el terminal del PC y en el monitor serie del ESP32.

> 📷 **AÑADIR FOTO / CAPTURA:** Captura de los dos terminales intercambiando mensajes.

> 🎬 **AÑADIR VÍDEO:** Vídeo del chat bidireccional ESP32 ↔ PC.

<details>
<summary><strong>Sockets, envío/recepción y pasos para iniciar el chat</strong></summary>

El programa [chat_ESP32_PC.py](P3_03_COMUNICACION_ENTRE_SOCKETS/chat_ESP32_PC.py) escucha en `0.0.0.0`, puerto `5000`. Utiliza un socket IPv4 de tipo `SOCK_STREAM`:

```python
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as servidor:
    servidor.bind((HOST, PUERTO))
    servidor.listen(1)
    conexion, direccion = servidor.accept()
```

`bind()` fija la dirección y el puerto de escucha, `listen()` habilita la espera de conexiones y `accept()` obtiene el socket con el que se intercambian datos con el ESP32.

En la placa, `abrirConexionTCP()` utiliza el objeto `WiFiClient cliente` y abre la conexión mediante:

```cpp
cliente.connect(WiFi.gatewayIP(), PUERTO);
```

Este destino corresponde al montaje en el que el portátil es el punto de acceso. Si el servidor se ejecuta en otro equipo de la red, el destino debe ser la IP de ese equipo.

### Envío y recepción

Los mensajes escritos por UART se transmiten con `cliente.println(mensaje)`. Los datos recibidos desde el PC se acumulan hasta encontrar un salto de línea y después se muestran por UART.

En Python, la recepción se ejecuta en un hilo separado para poder escribir en el terminal mientras llegan mensajes. `recv()` obtiene los bytes recibidos y `sendall()` envía el texto codificado en UTF-8, terminado en `\n`.

TCP entrega un flujo de bytes: una recepción puede contener parte de un mensaje o varios mensajes juntos. Por ello, ambos extremos mantienen un búfer y utilizan el salto de línea como delimitador. Se aceptan líneas terminadas en LF o CRLF, ignorando el retorno de carro cuando corresponde.

### Puesta en marcha

1. Activar el punto de acceso del portátil y configurar su SSID y contraseña en el proyecto.
2. Ejecutar `python chat_ESP32_PC.py` desde la carpeta del apartado.
3. Permitir la entrada TCP al puerto `5000` para Python en el perfil de red utilizado.
4. Arrancar o reiniciar el ESP32 con el servidor escuchando.
5. Escribir mensajes y pulsar Enter en cualquiera de los dos terminales. El monitor serie debe enviar un final de línea LF o CRLF.
6. Utilizar `/salir` en el terminal del PC para cerrar la sesión.

El chat funciona dentro de la red local. Para iniciar otra sesión tras el cierre se vuelve a ejecutar el servidor y se reinicia el ESP32.

</details>

---

<a name="hora-tcp"></a>

## ⏱ 04. Envío periódico de hora por TCP

**Fecha y hora local · intervalo de 1 s · temporización con millis()**

Código: [P3_04_ENVIO_HORA_PERIODICA](P3_04_ENVIO_HORA_PERIODICA/).

Esta etapa utiliza la conexión TCP para transmitir la fecha y hora local del ESP32 al ordenador. La inicialización abre el socket y ejecuta la rutina de hora inicial del módulo de comunicaciones. El bucle principal llama repetidamente a `enviarHoraPeriodicamente()`.

> 📷 **AÑADIR FOTO / CAPTURA:** Captura del PC recibiendo la hora cada segundo.

<details>
<summary><strong>Temporización, formato de mensaje y envío al servidor</strong></summary>

La función comprueba que el cliente siga conectado y utiliza `millis()` para espaciar los envíos un segundo:

```cpp
const unsigned long ahora = millis();
if (ahora - ultimoEnvioHora < 1000) {
    return;
}
ultimoEnvioHora = ahora;
```

Antes de preparar el mensaje se comprueba la disponibilidad de una hora válida con `getLocalTime(&fechaHora, 0)`. El tiempo de espera cero evita detener la ejecución mientras se espera una sincronización.

Cuando existe una hora disponible, se envía al servidor Python con `cliente.println(hora)` y se muestra una copia en el monitor serie. El mensaje contiene fecha y hora en el formato `dd/mm/aaaa hh:mm:ss` y termina con un salto de línea.

La temporización del envío se realiza leyendo el reloj local del ESP32; enviar un mensaje al PC cada segundo no implica hacer una petición NTP por cada mensaje.

</details>

---

<a name="control"></a>

## 05. Control del envío mediante start y stop

**MEF PARADO/ENVIANDO · comandos desde el PC · seguimiento por UART**

Código: [P3_05_ORDENAR_ENVIO_HORA_PERIODICA](P3_05_ORDENAR_ENVIO_HORA_PERIODICA/).

Se añade una máquina de estados finitos que controla cuándo se transmite la hora. La comunicación queda separada de la interpretación de los comandos: `protocol/coms` entrega líneas completas y `app/mef` decide qué hacer con ellas.

> 📷 **AÑADIR FOTO / CAPTURA:** Captura de los comandos start/stop y de los mensajes de estado.

> 🎬 **AÑADIR VÍDEO:** Vídeo: start → recepción de hora → stop, con el monitor serie visible.

<details>
<summary><strong>Estados, comandos y funcionamiento del control</strong></summary>

| **Estado** | **Comportamiento** |
| --- | --- |
| `PARADO` | Atiende las comunicaciones y mantiene detenido el envío de hora. Es el estado inicial. |
| `ENVIANDO` | Atiende las comunicaciones y llama a la función de envío periódico. |

| **Comando recibido desde el PC** | **Acción** |
| --- | --- |
| `start` | Pasa a `ENVIANDO`. |
| `stop` | Pasa a `PARADO`. |
| Otro mensaje | Se muestra por UART y conserva el estado. |

Los comandos se comparan literalmente con las cadenas `"start"` y `"stop"`, por lo que deben escribirse en minúsculas y terminar con un salto de línea.

```cpp
if (comando == "start") {
    estado = ENVIANDO;
    escribirMensajeUART("Envio de hora activado.");
} else if (comando == "stop") {
    estado = PARADO;
    escribirMensajeUART("Envio de hora detenido.");
}
```

Desde `loop()` se ejecutan `actualizarComs()` y `actualizarMEF()`. La MEF procesa todas las líneas completas pendientes antes de intentar enviar otra hora, de modo que un `stop` ya recibido se atiende primero. Si deja de existir conexión TCP, el control vuelve al estado `PARADO`.

El monitor serie permite observar los comandos recibidos, los cambios de estado y la hora enviada. Los mensajes introducidos por UART se reenvían al PC mediante el módulo de comunicaciones; los comandos que gobiernan la MEF se reciben desde el socket del PC.

Para mostrar el funcionamiento se inicia el servidor Python, se conecta el ESP32, se envía `start`, se observa la recepción periódica y se envía `stop` para detenerla manteniendo la sesión.

</details>

---

<a name="imu"></a>

## 06. Orientación y aceleración del BNO055 por Wi-Fi

**6 valores · 100 ms · 10 Hz nominales · gráfica XYZ y orientación 3D**

Código: [P3_06_ORDENAR_ENVIO_IMU](P3_06_ORDENAR_ENVIO_IMU/).

El ESP32 lee el BNO055 por I²C y transmite las muestras al ordenador mediante TCP sobre Wi-Fi. El firmware configura SDA en el GPIO `21`, SCL en el GPIO `22` y la dirección I²C del sensor en `0x29`.

> 📷 **AÑADIR FOTO / CAPTURA:** Foto del montaje ESP32 + BNO055 y sus conexiones.

> 📷 **AÑADIR FOTO / CAPTURA:** Captura de la app: gráfica XYZ, valores numéricos y vista 3D.

> 🎬 **AÑADIR VÍDEO:** Vídeo de la app recibiendo las medidas mientras se mueve la IMU.

> 📷 **AÑADIR FOTO / CAPTURA:** Captura opcional del terminal de la app o del CSV guardado.

<details>
<summary><strong>I²C, lectura del BNO055, trama TCP y aplicación Python</strong></summary>

`initBNO055()` inicializa el bus mediante `Wire.begin()` y comprueba la respuesta del sensor con `bno.begin()`. El resultado se guarda en `imuListo`, de manera que la aplicación solo llama al envío de muestras si la inicialización del sensor ha tenido éxito.

### Lectura del sensor

`leerBNO055()` consulta dos vectores:

- `VECTOR_EULER`: rumbo, roll y pitch, expresados en grados.
- `VECTOR_LINEARACCEL`: aceleraciones lineales X, Y y Z, expresadas en m/s² y con la componente gravitatoria compensada por el sensor.

```cpp
imu::Vector<3> orientacion =
    bno.getVector(Adafruit_BNO055::VECTOR_EULER);

imu::Vector<3> aceleracion =
    bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
```

Los resultados se guardan en la estructura `DatosBNO055`, que separa la lectura del hardware del formato de transmisión.

### Periodicidad y trama

Desde el bucle principal se llama a:

```cpp
enviarIMUPeriodicamente(100);
```

El argumento fija un intervalo de **100 ms**, equivalente a una frecuencia nominal de **10 muestras por segundo**. La función comprueba el tiempo transcurrido mediante `millis()` y solo realiza la lectura y el envío cuando corresponde y existe conexión TCP.

La trama actual contiene seis valores con tres decimales:

```text
rumbo;roll;pitch;accX;accY;accZ
359.437;5.125;0.312;-0.010;0.040;-0.030
```

La primera línea muestra el significado de los campos; las muestras enviadas contienen los números separados por `;`. `cliente.println(mensaje)` añade el final de línea que delimita cada muestra.

### Visualización con BNO055 IMU Studio

La aplicación se encuentra en [BNO055_IMU_Studio/bno055_monitor](BNO055_IMU_Studio/bno055_monitor/). Utiliza **Python 3.10 o posterior con Tkinter** y no necesita instalar paquetes adicionales con `pip`.

Se ejecuta mediante `INICIAR_WINDOWS.bat` o con:

```console
python app.py
```

El panel abre un servidor TCP en `0.0.0.0:5000`. Se prepara primero el panel y después se reinicia el ESP32. Durante esta prueba se utiliza este servidor en lugar del chat de terminal, ya que ambos emplean el mismo puerto.

La aplicación incluye:

- Valores numéricos de Ax, Ay y Az.
- Módulo de la aceleración, calculado como `sqrt(ax² + ay² + az²)`.
- **Una gráfica común con las tres componentes de aceleración**, que muestra los últimos diez segundos y ajusta automáticamente la escala.
- Vista 3D de una placa, sus ejes y el vector de aceleración.
- Visualización de rumbo/yaw, roll y pitch cuando la trama contiene orientación.
- Botón para centrar la orientación con respecto a una referencia visual.
- Terminal independiente con historial de recepción, pausa de vista, copia y limpieza.
- Modo demo para probar la interfaz con datos simulados.
- Grabación de muestras en un archivo CSV separado por `;`.

La recepción de sockets se ejecuta en un hilo separado y entrega los eventos a la interfaz mediante una cola. Así se pueden actualizar las gráficas y utilizar los controles mientras llegan datos de red.

`protocol.py` separa las líneas aunque lleguen fragmentadas y valida el número de campos y los valores numéricos. Admite la trama actual de seis valores, el formato simple `ax;ay;az` y el formato alternativo `IMU;ax;ay;az;roll;pitch;yaw`.

El CSV incorpora la hora de recepción en el PC, la unidad seleccionada y el origen TCP o demo. La selección de unidad modifica la etiqueta y la escala visual del vector; las muestras conservan sus valores originales.

La vista 3D utiliza los ángulos recibidos para orientar la placa y las aceleraciones para dibujar el vector. El centrado cambia la referencia de la representación sin modificar los datos recibidos ni los guardados. Los ejes y signos de la visualización deben interpretarse según el montaje físico del sensor.

El servidor Python vuelve a esperar conexiones si el cliente se desconecta. Para restablecer la sesión con este firmware se reinicia el ESP32. El historial y la cola tienen límites de memoria y el panel muestra un contador si se descartan eventos.

</details>

---

<a name="web"></a>

## 07. Servidor web de hora en el ESP32

**HTTP 80 · reloj · Reset · Poner en hora · actualización sin recargar**

Código: [P3_07_WEB_HORA](P3_07_WEB_HORA/).

En este apartado el ESP32 actúa como **servidor HTTP** en el puerto `80`. Un móvil u ordenador de la misma red puede abrir la página introduciendo `http://IP_DEL_ESP32/`. La dirección aparece en el monitor serie al iniciar el servidor.

> 📷 **AÑADIR FOTO / CAPTURA:** Captura de la página web en ordenador o móvil.

> 🎬 **AÑADIR VÍDEO:** Vídeo: Poner en hora → reloj actualizado → Reset → 00:00:00.

<details>
<summary><strong>Rutas HTTP, JavaScript, máquina de estados y HTML</strong></summary>

La página contiene un reloj y dos botones: **Reset** y **Poner en hora**. El HTML, CSS y JavaScript están separados del servidor en `include/network/web_html.h` y almacenados en flash mediante `PROGMEM`. La página se entrega con `servidor.send_P()`.

### Rutas HTTP

| **Método y ruta** | **Función** | **Respuesta** |
| --- | --- | --- |
| `GET /` | Entrega la página web. | `200`, contenido HTML. |
| `GET /hora` | Consulta el reloj que debe mostrar la interfaz. | `200`, texto `hh:mm:ss`. |
| `POST /reset` | Ejecuta la acción de Reset. | `204`, sin contenido. |
| `POST /poner-hora` | Solicita la puesta en hora mediante NTP. | `204`, sin contenido. |
| Ruta no registrada | Informa de que la ruta no existe. | `404`, texto de error. |

Al abrir la página, el navegador descarga el HTML y ejecuta su JavaScript. Después solicita `/hora` periódicamente con `fetch()` y actualiza únicamente el texto del reloj. La siguiente consulta se programa un segundo después de completar la anterior.

```javascript
const respuesta = await fetch('/hora', { cache: 'no-store' });
const hora = await respuesta.text();
reloj.textContent = hora;
```

El navegador inicia estas consultas y el ESP32 responde con el dato solicitado. Los botones envían peticiones POST a sus rutas, sin recargar la página.

### Control mediante MEF

La máquina de estados tiene dos estados:

| **Estado** | **Hora mostrada** |
| --- | --- |
| `SIN_HORA` | `00:00:00` de forma fija. Es el estado inicial y el resultado de pulsar Reset. |
| `EN_HORA` | Hora local del reloj sincronizado mediante NTP; se muestran ceros mientras todavía no existe una hora válida. |

Al pulsar Poner en hora se llama a `initNTP()` y se pasa a `EN_HORA`. Al pulsar Reset se vuelve a `SIN_HORA`. El reset controla la hora presentada por la MEF; el estado reside en el ESP32, por lo que recargar el navegador no cambia ese estado.

`obtenerHoraMEF()` devuelve el texto correspondiente. La ruta `/hora` lo entrega con una cabecera que evita reutilizar una respuesta almacenada:

```cpp
servidor.on("/hora", HTTP_GET, []() {
    servidor.sendHeader("Cache-Control", "no-store");
    servidor.send(200, "text/plain; charset=utf-8", obtenerHoraMEF());
});
```

Desde `loop()` se llama a `actualizarWeb()`, que ejecuta `servidor.handleClient()` para atender las peticiones entrantes y lanzar la función asociada a cada ruta.

### Detalles de la interfaz

La página se adapta a móvil y ordenador. Utiliza `touch-action: manipulation` para evitar el zoom por doble toque en los botones, mantiene el zoom manual con dos dedos y señala los errores al enviar una pulsación.

El JavaScript comprueba el formato `hh:mm:ss`, evita consultas simultáneas y utiliza una revisión para descartar respuestas anteriores a una pulsación más reciente. Así una consulta pendiente no sobrescribe el reloj con un dato anterior al reset.

<details>
<summary><strong>Ver el HTML, CSS y JavaScript completos de la página</strong></summary>

Contenido de `P3_07_WEB_HORA/include/network/web_html.h`, servido desde la memoria flash del ESP32.

```html
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32</title>
  <style>
    * { box-sizing: border-box; }
    html, body { touch-action: manipulation; }
    body {
      margin: 0;
      min-height: 100vh;
      min-height: 100svh;
      display: grid;
      place-items: center;
      background: #f3f5f8;
      font-family: system-ui, -apple-system, sans-serif;
    }
    main {
      display: flex;
      flex-wrap: wrap;
      max-width: 480px;
      justify-content: center;
      gap: 16px;
      padding: 24px;
    }
    #hora {
      flex-basis: 100%;
      text-align: center;
      font-size: clamp(42px, 12vw, 64px);
      font-weight: 700;
      font-variant-numeric: tabular-nums;
      color: #172033;
      margin-bottom: 16px;
    }
    button {
      appearance: none;
      -webkit-appearance: none;
      touch-action: manipulation;
      -webkit-user-select: none;
      user-select: none;
      -webkit-tap-highlight-color: transparent;
      border: 0;
      border-radius: 16px;
      padding: 18px 26px;
      min-height: 60px;
      font: 600 18px system-ui, -apple-system, sans-serif;
      color: white;
      background: #2563eb;
      box-shadow: 0 5px 0 #1743aa;
      cursor: pointer;
    }
    #reset { background: #e44848; box-shadow: 0 5px 0 #a62e2e; }
    button:active { transform: translateY(3px); box-shadow: 0 2px 0 #1743aa; }
    #reset:active { box-shadow: 0 2px 0 #a62e2e; }
    button:focus-visible { outline: 3px solid #111827; outline-offset: 6px; }
    button.error { outline: 3px solid #111827; outline-offset: 6px; }
  </style>
</head>
<body>
  <main>
    <output id="hora" aria-label="Hora">00:00:00</output>
    <button id="reset" type="button">Reset</button>
    <button id="poner-hora" type="button">Poner en hora</button>
  </main>
  <script>
    const reloj = document.getElementById('hora');
    let revision = 0;
    let consultando = false;

    async function actualizarHora() {
      if (consultando) return;
      consultando = true;
      const revisionConsulta = revision;
      try {
        const respuesta = await fetch('/hora', { cache: 'no-store' });
        if (!respuesta.ok) throw new Error('HTTP ' + respuesta.status);
        const hora = await respuesta.text();
        // Ignorar respuestas anteriores a una pulsación más reciente.
        if (revisionConsulta === revision && /^\d{2}:\d{2}:\d{2}$/.test(hora)) {
          reloj.textContent = hora;
        }
      } catch (error) {
        reloj.textContent = '00:00:00';
        console.error('No se pudo consultar la hora:', error);
      } finally {
        consultando = false;
      }
    }

    async function consultarPeriodicamente() {
      await actualizarHora();
      setTimeout(consultarPeriodicamente, 1000);
    }

    async function enviarPulsacion(ruta, boton) {
      revision++;
      try {
        // Cada click envía un POST al propio ESP32 sin recargar la página.
        const respuesta = await fetch(ruta, { method: 'POST' });
        if (!respuesta.ok) throw new Error('HTTP ' + respuesta.status);
        boton.classList.remove('error');
        boton.removeAttribute('title');
        if (ruta === '/reset') reloj.textContent = '00:00:00';
        await actualizarHora();
      } catch (error) {
        boton.classList.add('error');
        boton.title = 'No se pudo enviar la pulsación';
        console.error('Error de conexión con el ESP32:', error);
      }
    }

    const reset = document.getElementById('reset');
    const ponerHora = document.getElementById('poner-hora');
    reset.addEventListener('click', () => enviarPulsacion('/reset', reset));
    ponerHora.addEventListener('click', () => enviarPulsacion('/poner-hora', ponerHora));

    consultarPeriodicamente();

    // touch-action: manipulation evita el zoom por doble toque en Safari.
    // No se bloquea el zoom manual con dos dedos.
  </script>
</body>
</html>
```

</details>

</details>

---

<a name="json"></a>

## 08. Temperatura en JSON con campos SenML

**Temperatura simulada · campos n/u/v · unidad Cel · dos decimales**

Código: [P3_08_GENERAR_JSON](P3_08_GENERAR_JSON/).

Se desarrolla el módulo `sensors/temp` para producir temperaturas simuladas y representarlas mediante JSON. Se utilizan los campos SenML `n`, `u` y `v`, que permiten indicar de forma compacta el nombre de la medida, su unidad y su valor numérico.

> 📷 **AÑADIR FOTO / CAPTURA:** Captura de los JSON generados en el monitor serie.

<details>
<summary><strong>Generación de temperatura, campos y serialización JSON</strong></summary>

`generar_temp()` obtiene una temperatura mediante `random()` y la pasa a `generarJSON(float dato)`. La simulación suma una parte entera entre 0 y 50 y una fracción entre 0 y 0,50; puede producir valores entre **0,00 y 50,50 °C**.

```cpp
float temp = random(0, 50 + 1) + random(0, 50 + 1) / 100.0;
```

`generarJSON()` construye una cadena con un array de un registro:

```json
[{"n":"temp","u":"Cel","v":23.50}]
```

| **Campo** | **Significado en el mensaje** |
| --- | --- |
| `n` | Nombre de la medida: `temp`. |
| `u` | Unidad: `Cel`, grados Celsius. |
| `v` | Valor numérico de la temperatura. |

El valor se escribe con dos decimales y conserva el tipo numérico dentro del JSON. La serialización se realiza directamente con `String`:

```cpp
String generarJSON(float dato) {
    String json = "[{\"n\":\"temp\",\"u\":\"Cel\",\"v\":";
    json += String(dato, 2);
    json += "}]";
    return json;
}
```

En este proyecto, `loop()` imprime un nuevo JSON por el monitor serie e introduce una pausa de un segundo. En el siguiente apartado se reutiliza el módulo para crear el contenido de los archivos enviados por FTP.

</details>

---

<a name="ftp"></a>

## 09. Envío de archivos JSON mediante FTP

**ESP32 cliente · FileZilla Server · temperaturaN.json · pausa de 10 s**

Código: [P3_09_SERVER_FTP](P3_09_SERVER_FTP/).

El ESP32 actúa como **cliente FTP** y envía los registros de temperatura a **FileZilla Server**, instalado en el portátil. El módulo `network/FTP` utiliza la biblioteca `ESP32_FTPClient` y concentra la conexión, la creación del archivo y la transmisión del contenido.

> 📷 **AÑADIR FOTO / CAPTURA:** Captura de FileZilla Server y de la carpeta con los archivos recibidos.

> 📷 **AÑADIR FOTO / CAPTURA:** Captura del mismo JSON en el monitor serie y abierto en el PC.

> 🎬 **AÑADIR VÍDEO:** Vídeo de la creación y recepción de archivos JSON por FTP.

<details>
<summary><strong>Nombres de archivo, transferencia y configuración de FileZilla</strong></summary>

### Generación y nombres de archivo

En cada vuelta del bucle se genera una temperatura, se prepara su JSON y se construye un nombre con un contador:

```cpp
String temp = generar_temp();
String nombre = "temperatura" + String(i) + ".json";

Serial.println(nombre);
Serial.println(temp);
subirArchivoFTP(nombre.c_str(), temp);
```

Los nombres son `temperatura0.json`, `temperatura1.json`, `temperatura2.json`, etc. Después del envío se incrementa `i` y se introduce una pausa de **10 segundos**. El intervalo entre transferencias incluye también el tiempo empleado en conectar y enviar.

El contenido se prepara como una cadena en memoria y se transmite al servidor para crear el archivo remoto. El monitor serie muestra el nombre y el JSON asociado, lo que permite compararlos con el archivo recibido en el PC.

### Secuencia de transferencia

`subirArchivoFTP(const char* nombre, const String& texto)` ejecuta:

```cpp
ftp.OpenConnection();
ftp.InitFile("Type I");
ftp.NewFile(nombre);
ftp.Write(texto.c_str());
ftp.CloseFile();
ftp.CloseConnection();
```

`OpenConnection()` abre la sesión y realiza la autenticación. `InitFile("Type I")` selecciona la transferencia binaria. `NewFile()` prepara el archivo remoto, `Write()` transmite el texto JSON y las llamadas de cierre finalizan el archivo y la sesión.

El contador comienza en cero cada vez que se reinicia la placa. Al reutilizar un nombre, la transferencia puede sobrescribir el archivo remoto correspondiente.

### Configuración de FileZilla Server

Se configura un usuario con autenticación, una carpeta de destino y permisos para crear y escribir archivos. En el firmware se establece la IPv4 del portátil accesible desde el ESP32 junto con las credenciales de ese usuario.

FTP utiliza una conexión de control y otra de datos. En el montaje utilizado se habilita en el firewall el puerto TCP `21` y el rango pasivo `5000–5010`, configurado en FileZilla para las conexiones de datos. Las reglas deben aplicarse al perfil de red por el que se comunica el ESP32.

Para mostrar el resultado se abre uno de los archivos de la carpeta de destino y se compara su JSON con el que aparece en el monitor serie bajo el mismo nombre.

</details>

---

<a name="mqtt"></a>

## 10. Comunicación con Adafruit IO

**HTTP POST · MQTT 1883 · publicación y suscripción al feed**

Código: [P3_10_MQTT](P3_10_MQTT/).

Adafruit IO permite almacenar y consultar medidas mediante feeds. Cada feed tiene una clave que se utiliza tanto en su API HTTP como en los topics MQTT. La autenticación se realiza con el nombre de usuario y una AIO Key configurados localmente.

> 🖼️ **AÑADIR ESQUEMA:** Esquema de comunicación: ESP32, broker Adafruit IO y navegador.

> 📷 **AÑADIR FOTO / CAPTURA:** Captura de la petición HTTP POST y su resultado, con la AIO Key oculta.

> 📷 **AÑADIR FOTO / CAPTURA:** Captura del feed de Adafruit IO con los valores enviados por el ESP32.

> 📷 **AÑADIR FOTO / CAPTURA:** Captura de un valor añadido desde el navegador y recibido por UART.

> 🎬 **AÑADIR VÍDEO:** Vídeo de la comunicación en ambos sentidos: ESP32 → feed y navegador → ESP32.

<details>
<summary><strong>API HTTP, broker, topics, funciones MQTT y recepción desde el navegador</strong></summary>

### Escritura de datos mediante HTTP POST

Para introducir un dato desde una herramienta de peticiones HTTP en el navegador se utiliza el endpoint de creación de datos de Adafruit IO:

```text
POST https://io.adafruit.com/api/v2/USUARIO/feeds/CLAVE_DEL_FEED/data
```

La petición utiliza las cabeceras:

```text
X-AIO-Key: TU_AIO_KEY
Content-Type: application/json
```

Y un cuerpo JSON con el valor que se quiere guardar:

```json
{"value":23.5}
```

`USUARIO` y `CLAVE_DEL_FEED` se sustituyen por los identificadores de la cuenta y del feed. La respuesta de la API y el historial del feed permiten comprobar el resultado de la petición. El mismo feed sirve como punto de intercambio para la comunicación MQTT del ESP32.



### MQTT: clientes, broker y topics

El ESP32 utiliza **Adafruit MQTT Library** y se conecta al broker `io.adafruit.com` por el puerto TCP `1883`. `WiFiClient` proporciona el transporte TCP y `Adafruit_MQTT_Client` gestiona la sesión MQTT.

El ESP32 es un cliente MQTT. Cuando publica envía un valor al broker; cuando se suscribe solicita recibir las actualizaciones de un topic. Adafruit IO actúa como broker y distribuye los mensajes a los clientes suscritos.

| **Elemento** | **Función** |
| --- | --- |
| ESP32 | Cliente que puede publicar y recibir valores. |
| Broker Adafruit IO | Recibe publicaciones y distribuye actualizaciones. |
| Feed | Almacena los datos asociados a una clave. |
| Navegador | Permite añadir y consultar datos a través de la web o de la API HTTP. |

El topic tiene la estructura:

```text
USUARIO/feeds/CLAVE_DEL_FEED
```

El módulo construye el topic a partir de `AIO_USERNAME` y `AIO_FEED_KEY`. Esta última debe coincidir con la clave real del feed. El usuario y la AIO Key se pasan al cliente MQTT para autenticar la conexión.

### Organización de MQTT.cpp

Los objetos de conexión, publicación y suscripción se mantienen dentro del módulo. La aplicación utiliza tres funciones públicas:

| **Función** | **Comportamiento** |
| --- | --- |
| `bool initMQTT(bool suscriptor = false)` | Comprueba el Wi-Fi, registra la suscripción si se solicita e intenta conectar al broker. Devuelve si la sesión queda conectada. |
| `bool envioMQTT(float dato)` | Comprueba o restablece la conexión y publica un número en el feed configurado. |
| `String recibirMQTT()` | Habilita la suscripción, procesa mensajes y devuelve el último valor recibido, o una cadena vacía. |

`initMQTT(true)` registra la suscripción una sola vez. Si ya existía una conexión sin esa suscripción, la cierra y conecta de nuevo para solicitarla al broker. Si la conexión falla, muestra el error mediante `connectErrorString()`.

`envioMQTT()` publica mediante el objeto `Adafruit_MQTT_Publish`:

```cpp
const bool enviado = feed.publish(static_cast<double>(dato));
```

La publicación utiliza QoS 0. El retorno `true` indica que la biblioteca consiguió enviar el paquete; representa el resultado del envío local, sin confirmación de almacenamiento de la medida en Adafruit IO.

`recibirMQTT()` llama a `mqtt.readSubscription(10)` y copia como texto el contenido de `suscripcion.lastread`. Si procesa varias actualizaciones durante una llamada, devuelve la última de ellas.

### Recepción desde el navegador

El `main.cpp` de esta etapa inicializa UART, Wi-Fi y MQTT. Después consulta los mensajes desde `loop()`:

```cpp
String dato = recibirMQTT();

if (!dato.isEmpty()) {
    Serial.print("MQTT: dato recibido: ");
    Serial.println(dato);
}
delay(500);
```

La llamada a `recibirMQTT()` activa la suscripción mediante `initMQTT(true)`. Para comprobar la recepción se añade un valor desde **Add Data** en la página del feed. Adafruit IO actualiza el feed y el broker entrega el valor al ESP32, que lo muestra en el monitor serie.

La función `envioMQTT(float dato)` proporciona el sentido ESP32 hacia Adafruit IO. Por ejemplo, una llamada a `envioMQTT(23.5f)` publica ese valor y devuelve el resultado del envío. Si el cliente está suscrito al mismo feed en el que publica, también puede recibir su propia publicación.

Para observar ambos sentidos se compara el valor publicado por el ESP32 con el mostrado en el feed y, después, se añade otro valor desde el navegador y se comprueba su recepción por UART.



La conexión de este módulo utiliza MQTT sin TLS por el puerto `1883`. Las credenciales y la AIO Key se configuran localmente; las capturas y ejemplos del README utilizan únicamente valores de referencia.

</details>

---

## Desarrollo adicional: chat_wifi

<details>
<summary><strong>Consultar la variante autónoma del chat para Windows</strong></summary>

La carpeta [chat_wifi](chat_wifi/) incluye una variante autónoma del chat para Windows. El firmware está concentrado en `src/main.cpp` y el servidor se ejecuta con `python chat_servidor.py`.

Mantiene el portátil como punto de acceso y servidor TCP, el ESP32 como estación y cliente y el puerto `5000`. Utiliza mensajes UTF-8 terminados en LF con un límite de 512 bytes, controla el cierre de la sesión y permite utilizar `/salir` desde el monitor serie.

En Python se utilizan `select` y `msvcrt` para recibir mensajes y atender el teclado desde un terminal de Windows. Esta variante se puede usar para comprobar la comunicación básica antes de trabajar con las aplicaciones por capas.

</details>

---

## Resumen de las comunicaciones utilizadas

| **Comunicación** | **Papel del ESP32** | **Extremo remoto** | **Información intercambiada** |
| --- | --- | --- | --- |
| Wi-Fi | Estación | Punto de acceso | Conectividad de red. |
| ICMP | Emisor de ping | Google | Solicitudes y respuestas de conectividad. |
| NTP | Cliente | `pool.ntp.org` | Referencia de tiempo para el reloj local. |
| TCP, puerto 5000 | Cliente | Servidor Python del PC | Chat, comandos, fecha/hora y muestras IMU, según el proyecto. |
| HTTP, puerto 80 | Servidor | Navegador del PC o móvil | HTML, consulta de hora y acciones de los botones. |
| FTP | Cliente | FileZilla Server | Archivos JSON con temperatura simulada. |
| MQTT, puerto 1883 | Publicador y/o suscriptor | Broker Adafruit IO | Valores del feed. |

Los apartados muestran cómo se reutiliza la conexión Wi-Fi para distintas aplicaciones y cómo cambia el papel del ESP32: cliente en TCP, NTP, FTP y MQTT, y servidor en la aplicación web.

## Referencias

<details>
<summary><strong>Consultar documentación y bibliotecas</strong></summary>

- [Formato SenML · RFC 8428](https://datatracker.ietf.org/doc/html/rfc8428).
- [API de Adafruit IO · creación de datos](https://io.adafruit.com/api/docs/?shell#create-data).
- [Feeds de Adafruit IO](https://learn.adafruit.com/adafruit-io-basics-feeds).
- [Adafruit MQTT Library](https://github.com/adafruit/Adafruit_MQTT_Library).
- [Biblioteca esp32_ftpclient](https://registry.platformio.org/libraries/ldab/esp32_ftpclient).
- [FileZilla](https://filezilla-project.org/).
- [BNO055 · documentación del fabricante](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bno055-ds000.pdf).
- [Sockets en Python](https://docs.python.org/3/library/socket.html).
- [Tkinter](https://docs.python.org/3/library/tkinter.html).

</details>

</div>
