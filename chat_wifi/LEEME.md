# Chat TCP entre portátil Windows y ESP32

El portátil es AP WiFi y servidor TCP. El ESP32 es estación WiFi y cliente TCP.
Un único ESP32, puerto 5000, mensajes UTF-8 terminados en LF, máximo 512 bytes.
No requiere paquetes Python externos. Python 3.11 o posterior en Windows.

1. Activa el punto de acceso móvil del portátil con banda 2,4 GHz.
2. Edita SSID y PASSWORD en src/main.cpp.
3. Abre una consola de Windows (preferiblemente Windows Terminal) en esta carpeta:
   `python chat_servidor.py`
4. Si Windows solicita acceso de red para Python, permítelo en la red del AP.
   Si no conecta, revisa que el firewall permita entrada TCP 5000 para Python
   en el perfil de red usado por el AP.
5. Abre esta carpeta en PlatformIO, carga el firmware y abre el monitor serie.
   Si al abrir el monitor no ves el inicio, pulsa RESET en el ESP32.
6. Escribe mensajes y pulsa Enter en cualquiera de los dos terminales.
   En el monitor serie se usa LF como final de línea, con envío al pulsar Enter.
7. Escribe /salir para cerrar. También puedes cerrar Python con Ctrl+C.
   Después de cerrar, vuelve a ejecutar Python y reinicia el ESP32.

El ESP32 usa WiFi.gatewayIP() como IP del servidor: en el escenario descrito,
la puerta de enlace del AP es el portátil. Comprueba la IP impresa. Si no coincide,
ejecuta ipconfig en Windows y sustituye la variable servidor por la IPv4 del
adaptador del punto de acceso, por ejemplo IPAddress(192, 168, 137, 1).
No uses la dirección del adaptador que conecta el portátil a Internet.

El programa escucha en 0.0.0.0 (todas las interfaces); esa no es la IP a la que
se conecta el ESP32. No necesita conexión a Internet para el chat.

Python usa select para recibir sin bloquear el teclado; msvcrt lee el teclado
de Windows. Se ejecuta en un terminal real, no en la consola de depuración de VS Code.
bind/listen/accept crean el servidor; recv recibe bytes y sendall envía la línea completa.
El búfer de recepción conserva líneas incompletas y separa mensajes agrupados.

Al detectar cierre TCP, error de socket o pérdida de WiFi, el chat termina.
Una caída silenciosa puede tardar en ser detectada por TCP; este ejemplo no tiene
latidos ni detección inmediata de desconexiones silenciosas.

Validación: sintaxis Python comprobada. No probado en Windows ni en ESP32 físico;
la compilación y la comunicación real deben verificarse con tu placa.
