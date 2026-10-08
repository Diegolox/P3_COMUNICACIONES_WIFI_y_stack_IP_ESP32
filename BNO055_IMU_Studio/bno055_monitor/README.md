# BNO055 · IMU Studio

Panel Python con aceleración XYZ, módulo, gráficas de los últimos 10 segundos,
vector de aceleración 3D, orientación opcional, modo demo y grabación CSV.
La interfaz usa Tkinter y bibliotecas estándar: no necesita instalar paquetes con pip.
Requiere Python 3.10 o posterior con Tkinter (habitualmente incluido en Python para Windows).

## Arranque con tu ESP32

1. Extrae el ZIP completo en una carpeta.
2. Activa el punto de acceso Wi-Fi del portátil al que se conecta tu ESP32.
3. Ejecuta `INICIAR_WINDOWS.bat`, o abre un terminal en la carpeta y escribe `python app.py`.
4. El programa abre automáticamente un servidor en `0.0.0.0:5000`.
   `0.0.0.0` significa escuchar en los adaptadores del portátil; no es la IP que usa el ESP32.
5. Si Windows pregunta por el acceso de Python a la red, permite el acceso en el
   perfil de red utilizado por tu punto de acceso. Si no conecta, revisa que ese
   perfil permita conexiones TCP entrantes al puerto 5000.
6. Reinicia el ESP32 después de que el panel indique «Esperando ESP32».
   Tu firmware intenta conectarse a la puerta de enlace Wi-Fi en el puerto 5000.
   No necesitas introducir la IP del ESP32 en el panel.
7. Inicializa el BNO055 en el firmware y llama repetidamente a
   `enviarIMUPeriodicamente(100)` para recibir aproximadamente 10 muestras/s.
   Esa función debe estar declarada en tu cabecera y ejecutarse desde el bucle o tarea correspondiente.

El `main(6).cpp` adjunto es un chat: por sí solo NO lee ni transmite el BNO055.
El panel es compatible con la función de envío IMU de tu módulo `coms`:
`accX;accY;accZ` terminado en LF. Los mensajes de hora y chat aparecen en el registro
sin confundirse con muestras. Puedes probarlo enviando `0;0;9.81` desde el monitor
serie con el firmware de chat, pero será una muestra escrita manualmente.

Si hay desconexión, Python vuelve a esperar otra conexión. El firmware adjunto no
reconecta solo: debes reiniciar el ESP32. «Cliente» sirve únicamente para otro
firmware que abra un servidor TCP en el propio ESP32: introduce su IP real.
No hay búsqueda automática ni cambio de firmware desde el panel.

## Qué muestra el 3D

Con tres valores la placa permanece fija y la flecha amarilla representa la
aceleración en los ejes de referencia. La longitud se limita para mantenerla visible;
los valores numéricos conservan la medida original. No es una reconstrucción de la
posición del sensor ni de su giro. Tres aceleraciones no determinan una orientación
completa; una inclinación aproximada requeriría que predominase la gravedad.

La unidad se elige manualmente y debe coincidir con la del firmware. Cambiarla solo
cambia la etiqueta y la escala del vector: NO convierte las medidas. El valor por
defecto es m/s²; comprueba cómo se configuran y convierten los registros en tu driver.
El módulo es sqrt(ax² + ay² + az²). Si se envía aceleración total, normalmente incluye
la gravedad; si se envía aceleración lineal, no la incluye.

## Orientación opcional real

El panel también admite exclusivamente esta trama con prefijo:

```text
IMU;ax;ay;az;roll;pitch;yaw\n
```

Los tres ángulos deben estar en grados. Ejemplo:

```text
IMU;0.000;0.000;9.810;15.000;25.000;90.000
```

La app usa la convención visual Rz(yaw) · Ry(pitch) · Rx(roll).
Comprueba signos, orden y remapeo de ejes de tu BNO055 antes de comparar la placa
virtual con la física: los formatos de orientación Windows/Android del sensor y la
orientación del montaje pueden requerir adaptación. No envíes valores de giroscopio
como si fueran ángulos. «Centrar orientación» fija una referencia visual relativa;
no cambia los datos recibidos ni los guardados.

Fuente del sensor: https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bno055-ds000.pdf
Tkinter: https://docs.python.org/3/library/tkinter.html

## Controles y archivos

- Conectar / escuchar: aplica IP, puerto y modo, y reinicia la sesión.
- Detener: cierra la red y termina la grabación activa.
- Demo: genera muestras y ángulos simulados para probar la interfaz sin hardware.
  Para volver a recibir datos, sal de demo y reinicia el ESP32 si perdió la conexión.
- Grabar CSV: pide un destino y guarda las nuevas muestras hasta pulsar de nuevo.
  El archivo usa `;`, incluye unidad, hora de recepción del PC y origen demo/TCP.
  No registra la hora de adquisición del sensor: el firmware no la transmite.

`app.py`: interfaz, demo, gráficas y CSV.
`network.py`: sockets en un hilo, separado del hilo de la interfaz.
`protocol.py`: separación por líneas y validación de tramas.
`view3d.py`: proyección 3D de la placa, ejes y vector.

La cola de red y el historial tienen límites de memoria. Si llegan datos más rápido
de lo que la interfaz puede procesar, se descartan eventos y se muestra el contador:
no uses este panel como registrador sin pérdidas a frecuencias muy altas.
Solo se atiende una conexión TCP simultánea. No utiliza credenciales Wi-Fi: la
conexión a la red se configura en el sistema operativo y en el ESP32.

## Verificación sin hardware

Desde esta carpeta:

```console
python -m unittest discover -s tests -v
```

Las pruebas comprueban fragmentación TCP, líneas agrupadas, rechazo de tramas
inválidas, recepción con sockets reales de localhost y reconexión de un cliente.
El hardware ESP32/BNO055 debe verificarse en tu portátil.
