#pragma once
#include <Arduino.h>

// HTML separado del servidor y almacenado en la memoria flash del ESP32.
// No necesita subir archivos a LittleFS ni instalar librerías externas.
static const char PAGINA_WEB[] PROGMEM = R"html(
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
      justify-content: center;
      gap: 16px;
      padding: 24px;
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
    <button id="reset" type="button">Reset</button>
    <button id="poner-hora" type="button">Poner en hora</button>
  </main>
  <script>
    async function enviarPulsacion(ruta, boton) {
      try {
        // Cada click envía un POST al propio ESP32 sin recargar la página.
        const respuesta = await fetch(ruta, { method: 'POST' });
        if (!respuesta.ok) throw new Error('HTTP ' + respuesta.status);
        boton.classList.remove('error');
        boton.removeAttribute('title');
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

    // touch-action: manipulation evita el zoom por doble toque en Safari.
    // No se bloquea el zoom manual con dos dedos.
  </script>
</body>
</html>
)html";
