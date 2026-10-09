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
)html";
