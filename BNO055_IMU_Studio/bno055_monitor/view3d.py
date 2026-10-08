"""Proyección 3D sencilla sobre Canvas, sin OpenGL ni dependencias externas."""
import math

BG = '#101c2e'
COLORS = ('#ff6981', '#4ce0b3', '#66b7ff')


def rotate(point, roll, pitch, yaw):
    """Convención de la app: Rz(yaw) Ry(pitch) Rx(roll), ángulos en grados."""
    x, y, z = point
    r, p, h = map(math.radians, (roll, pitch, yaw))
    y, z = y*math.cos(r)-z*math.sin(r), y*math.sin(r)+z*math.cos(r)
    x, z = x*math.cos(p)+z*math.sin(p), -x*math.sin(p)+z*math.cos(p)
    return x*math.cos(h)-y*math.sin(h), x*math.sin(h)+y*math.cos(h), z


def draw(canvas, sample, unit, reference=None):
    canvas.delete('all')
    w, h = canvas.winfo_width(), canvas.winfo_height()
    scale = min(w/6.3, h/5.2)

    def project(point):
        # Cámara fija inclinada para ver los tres ejes.
        x, y, z = rotate(point, 58, 0, -35)
        return w/2+x*scale, h/2-y*scale, z

    def line(a, b, **kwargs):
        aa, bb = project(a), project(b)
        canvas.create_line(*aa[:2], *bb[:2], **kwargs)

    for i in range(-2, 3):
        line((-2, i, -0.3), (2, i, -0.3), fill='#22334b')
        line((i, -2, -0.3), (i, 2, -0.3), fill='#22334b')

    oriented = sample is not None and sample.roll is not None
    angles = (sample.roll, sample.pitch, sample.yaw) if oriented else (0, 0, 0)
    # Centrado relativo: R_ref^T R_current, no resta de ángulos de Euler.
    def transform(v):
        v = rotate(v, *angles)
        if reference is not None and oriented:
            r, p, y = reference
            v = rotate(v, 0, 0, -y)
            v = rotate(v, 0, -p, 0)
            v = rotate(v, -r, 0, 0)
        return v

    points = [(-1,-.65,-.1),(1,-.65,-.1),(1,.65,-.1),(-1,.65,-.1),
              (-1,-.65,.1),(1,-.65,.1),(1,.65,.1),(-1,.65,.1)]
    points = [project(transform(p)) for p in points]
    faces = [(0,1,2,3),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7),(4,5,6,7)]
    for face in sorted(faces, key=lambda f: sum(points[i][2] for i in f)):
        coords = [v for i in face for v in points[i][:2]]
        canvas.create_polygon(*coords, fill='#1c766d' if face == faces[-1] else '#17443f', outline='#5adbc2', width=2)
    center = project(transform((0, 0, .13)))
    canvas.create_text(*center[:2], text='BNO055', fill='white', font=('Segoe UI', 13, 'bold'))
    for axis, color, name in zip(((1.65,0,0),(0,1.65,0),(0,0,1.65)), COLORS, 'XYZ'):
        line((0,0,0), transform(axis), fill=color, width=3, arrow='last')
        pos = project(transform(tuple(v*1.14 for v in axis)))
        canvas.create_text(*pos[:2], text=name, fill=color, font=('Segoe UI', 12, 'bold'))

    if sample:
        values = (sample.ax, sample.ay, sample.az)
        norm = sample.magnitude
        if norm > 1e-9:
            # Longitud acotada para que el vector siempre permanezca visible.
            vector = tuple(v/norm*min(2.1, .4+norm/(9.81 if unit == 'm/s²' else 1)) for v in values)
            line((0,0,0), transform(vector), fill='#ffd174', width=5, arrow='last')
    title = 'Orientación recibida · placa + aceleración' if oriented else 'Aceleración 3D · placa fija como referencia'
    canvas.create_text(18, 20, anchor='nw', text=title, fill='#dce7f7', font=('Segoe UI', 12, 'bold'))
    caption = 'Convención visual Rz · Ry · Rx' if oriented else 'La flecha amarilla representa la aceleración; no calcula posición ni orientación.'
    canvas.create_text(18, h-18, anchor='sw', text=caption, fill='#91a5c2', width=max(100,w-36), font=('Segoe UI', 10))
