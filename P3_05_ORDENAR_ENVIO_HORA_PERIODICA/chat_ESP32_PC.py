"""Servidor TCP de terminal para el chat con el ESP32. Solo necesita Python 3."""

import socket
import threading

HOST = "0.0.0.0"  # Escucha en las interfaces de red del PC.
PUERTO = 5000     # Debe coincidir con el puerto configurado en el ESP32.


def recibir_mensajes(conexion, terminado):
    """Recibe en otro hilo para poder escribir mientras llegan mensajes."""
    entrada = b""
    try:
        while not terminado.is_set():
            datos = conexion.recv(1024)
            if not datos:
                break  # El ESP32 ha cerrado la conexion.

            entrada += datos

            # TCP puede entregar parte de un mensaje o varios juntos.
            # Cada salto de linea indica que hay un mensaje completo.
            while b"\n" in entrada:
                linea, entrada = entrada.split(b"\n", 1)
                mensaje = linea.rstrip(b"\r").decode("utf-8", errors="replace")
                if mensaje:
                    print(f"\nESP32: {mensaje}", flush=True)
    except OSError as error:
        if not terminado.is_set():
            print(f"\nError al recibir: {error}", flush=True)
    finally:
        if not terminado.is_set():
            print("\nESP32 desconectado. Pulsa Enter para cerrar.", flush=True)
        terminado.set()


def main():
    # Crea el servidor TCP y espera una conexion del ESP32.
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as servidor:
        servidor.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        servidor.bind((HOST, PUERTO))
        servidor.listen(1)

        print(f"Servidor TCP abierto en el puerto {PUERTO}.", flush=True)
        print("Esperando al ESP32... Arranca o reinicia el ESP32 ahora.", flush=True)
        conexion, direccion = servidor.accept()

        with conexion:
            print(f"ESP32 conectado desde {direccion[0]}.", flush=True)
            print("Escribe y pulsa Enter. Usa /salir para cerrar el chat.", flush=True)

            terminado = threading.Event()
            hilo = threading.Thread(
                target=recibir_mensajes,
                args=(conexion, terminado),
                daemon=True,
            )
            hilo.start()

            try:
                while not terminado.is_set():
                    mensaje = input("PC: ")
                    if terminado.is_set() or mensaje == "/salir":
                        break
                    if mensaje:
                        # El ESP32 necesita '\n' para mostrar la linea recibida.
                        conexion.sendall((mensaje + "\n").encode("utf-8"))
            except (KeyboardInterrupt, EOFError):
                pass
            except OSError as error:
                print(f"\nError al enviar: {error}")
            finally:
                terminado.set()
                # Desbloquea recv() para que el hilo pueda terminar.
                try:
                    conexion.shutdown(socket.SHUT_RDWR)
                except OSError:
                    pass
                hilo.join(timeout=1)

    print("Chat cerrado.")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nServidor cerrado.")
    except OSError as error:
        print(f"No se pudo abrir o utilizar el servidor: {error}")
