"""Tramas delimitadas por LF; TCP puede fragmentarlas o agruparlas."""
from dataclasses import dataclass
import math
import time


@dataclass(frozen=True)
class Sample:
    received: float
    ax: float
    ay: float
    az: float
    roll: float | None = None
    pitch: float | None = None
    yaw: float | None = None

    @property
    def magnitude(self):
        return math.sqrt(self.ax**2 + self.ay**2 + self.az**2)


def parse_sample(line):
    """Admite ax;ay;az o IMU;ax;ay;az;roll;pitch;yaw (grados)."""
    fields = line.strip().split(';')
    if fields[0] == 'IMU':
        fields = fields[1:]
        if len(fields) != 6:
            return None
    elif len(fields) != 3:
        return None
    try:
        values = [float(v) for v in fields]
    except ValueError:
        return None
    if not all(math.isfinite(v) and abs(v) <= 1e6 for v in values):
        return None
    return Sample(time.monotonic(), *values)


class LineDecoder:
    """Limita cada línea a 4096 bytes; rechaza una conexión que supere el límite."""
    def __init__(self, limit=4096):
        self.buffer = bytearray()
        self.limit = limit

    def feed(self, data):
        self.buffer.extend(data)
        lines = []
        while True:
            end = self.buffer.find(b'\n')
            if end == -1:
                if len(self.buffer) > self.limit:
                    raise ValueError('Línea TCP demasiado larga')
                return lines
            if end > self.limit:
                raise ValueError('Línea TCP demasiado larga')
            lines.append(bytes(self.buffer[:end]).decode('utf-8', errors='replace').rstrip('\r'))
            del self.buffer[:end + 1]
