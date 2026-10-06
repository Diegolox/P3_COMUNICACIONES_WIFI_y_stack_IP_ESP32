#pragma once
// Abre la conexión TCP y activa el chat.
// Se ejecuta después de conectar al Wi-Fi.
void initComs();

// Atiende el envío, la recepción y la desconexión.
// Se ejecuta continuamente desde loop().
void actualizarComs();

// Se llama desde loop(); envia la hora cada 1000 ms si hay conexion.
void enviarHoraPeriodicamente();
