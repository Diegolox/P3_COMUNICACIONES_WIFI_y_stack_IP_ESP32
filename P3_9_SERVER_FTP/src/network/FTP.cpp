#include <Arduino.h>
#include <WiFi.h>
#include <ESP32_FTPClient.h>
#include "network/FTP.h"

// Datos del servidor de tu portátil.
char servidor[] = "10.1.64.88";
char usuario[] = "ESP32";
char password[] = "barrabasada";

ESP32_FTPClient ftp(servidor, usuario, password, 5000, 0);

void subirArchivoFTP(const char* nombre, const String& texto) {
    ftp.OpenConnection();        // Conecta al servidor.
    ftp.InitFile("Type I");       // Prepara la transferencia.
    ftp.NewFile(nombre);         // Crea o sobrescribe el archivo.
    ftp.Write(texto.c_str());     // Envía el contenido.
    ftp.CloseFile();             // Termina el archivo.
    ftp.CloseConnection();       // Desconecta.
}