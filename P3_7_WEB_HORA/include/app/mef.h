#pragma once

// Establece SIN_HORA como estado inicial.
void initMEF();

// Procesa una pulsación. Si ambos argumentos son true, tiene prioridad Reset.
void actualizarMEF(bool reset, bool ponerEnHora);
