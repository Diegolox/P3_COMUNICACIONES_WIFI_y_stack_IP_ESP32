"""Chat TCP para Windows. Ejecutar: python chat_servidor.py"""
import msvcrt
import select
import socket

HOST = "0.0.0.0"  # Escuchar en las interfaces del portátil.
PORT = 5000
MAX_LINE = 512  # Bytes UTF-8 por mensaje, sin el salto de línea.


def chat(conn):
    """Atender teclado y socket sin bloquear esperando input()."""
    pending = bytearray()
    typed = ""
    print("Escribe y pulsa Enter. /salir cierra el chat.")
    print("Tú> ", end="", flush=True)
    while True:
        if select.select([conn], [], [], 0.05)[0]:
            data = conn.recv(1024)
            if not data:
                print("\nESP32 desconectado.")
                return
            pending.extend(data)
            # TCP transporta bytes: un recv puede contener varias líneas
            # o solo una parte. Guardar lo incompleto hasta el próximo recv.
            while b"\n" in pending:
                line, _, rest = pending.partition(b"\n")
                pending = bytearray(rest)
                if len(line) > MAX_LINE:
                    raise ValueError("Mensaje recibido demasiado largo")
                print("\r\033[2KESP32> " + line.decode("utf-8", errors="replace"))
                print("Tú> " + typed, end="", flush=True)
            if len(pending) > MAX_LINE:
                raise ValueError("Mensaje recibido demasiado largo")

        while msvcrt.kbhit():
            char = msvcrt.getwch()
            if char in ("\x00", "\xe0"):
                msvcrt.getwch()  # Descartar teclas especiales, como flechas.
                continue
            if char == "\x03":
                raise KeyboardInterrupt
            if char == "\r":
                print()
                if typed == "/salir":
                    return
                encoded = typed.encode("utf-8")
                if len(encoded) > MAX_LINE:
                    print("Máximo 512 bytes UTF-8. Mensaje no enviado.")
                elif typed:
                    conn.sendall(encoded + b"\n")
                typed = ""
                print("Tú> ", end="", flush=True)
            elif char == "\b":
                if typed:
                    typed = typed[:-1]
                    print("\b \b", end="", flush=True)
            elif char.isprintable():
                typed += char
                print(char, end="", flush=True)


def main():
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
            server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            server.bind((HOST, PORT))
            server.listen(1)
            # Timeout para que Ctrl+C también funcione esperando al ESP32.
            server.settimeout(0.5)
            print(f"Esperando al ESP32 en TCP {PORT}… (Ctrl+C para cerrar)")
            while True:
                try:
                    conn, address = server.accept()
                    break
                except socket.timeout:
                    continue
            with conn:
                print(f"ESP32 conectado: {address[0]}:{address[1]}")
                chat(conn)
    except KeyboardInterrupt:
        print("\nChat cerrado.")
    except (OSError, ValueError) as error:
        print(f"\nChat terminado: {error}")


if __name__ == "__main__":
    main()
