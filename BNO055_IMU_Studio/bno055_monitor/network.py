"""Red en un hilo independiente: nunca accede a widgets de Tkinter."""
import queue
import socket
import threading
from protocol import LineDecoder


class Connection:
    def __init__(self):
        self.events = queue.Queue(maxsize=2000)
        self.stop_event = threading.Event()
        self.thread = None
        self.dropped = 0

    def emit(self, kind, value):
        try:
            self.events.put_nowait((kind, value))
        except queue.Full:
            self.dropped += 1

    def start(self, mode, host, port):
        self.stop()
        while not self.events.empty():
            self.events.get_nowait()
        self.dropped = 0
        self.stop_event.clear()
        self.thread = threading.Thread(target=self.run, args=(mode, host, port), daemon=True)
        self.thread.start()

    def stop(self):
        self.stop_event.set()
        if self.thread:
            self.thread.join(timeout=1.5)
            if self.thread.is_alive():
                raise RuntimeError('La conexión anterior sigue cerrándose; vuelve a intentarlo.')
            self.thread = None

    def receive(self, sock, address):
        self.emit('status', f'Conectado a {address[0]}:{address[1]}')
        decoder = LineDecoder()
        sock.settimeout(0.25)
        while not self.stop_event.is_set():
            try:
                data = sock.recv(4096)
            except socket.timeout:
                continue
            if not data:
                self.emit('log', 'El ESP32 ha cerrado la conexión.')
                return
            for line in decoder.feed(data):
                self.emit('line', line)

    def run(self, mode, host, port):
        if mode == 'Servidor':
            self.server(host, port)
        else:
            self.client(host, port)

    def server(self, host, port):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as listener:
                listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                listener.bind((host, port))
                listener.listen(1)
                listener.settimeout(0.25)
                while not self.stop_event.is_set():
                    self.emit('status', f'Esperando ESP32 · {host}:{port}')
                    try:
                        sock, address = listener.accept()
                    except socket.timeout:
                        # No repite el estado en cada timeout.
                        while not self.stop_event.is_set():
                            try:
                                sock, address = listener.accept()
                                break
                            except socket.timeout:
                                continue
                        else:
                            return
                    with sock:
                        try:
                            self.receive(sock, address)
                        except (OSError, ValueError) as exc:
                            self.emit('log', str(exc))
        except OSError as exc:
            self.emit('status', f'Error de servidor: {exc}')

    def client(self, host, port):
        # Este modo requiere firmware con servidor TCP en el ESP32.
        while not self.stop_event.is_set():
            self.emit('status', f'Conectando a {host}:{port}…')
            try:
                with socket.create_connection((host, port), timeout=0.75) as sock:
                    self.receive(sock, (host, port))
            except (OSError, ValueError) as exc:
                self.emit('log', str(exc))
            if self.stop_event.wait(2):
                return
