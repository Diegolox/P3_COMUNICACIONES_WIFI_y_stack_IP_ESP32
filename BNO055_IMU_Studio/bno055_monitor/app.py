"""Panel BNO055. Ejecutar con: python app.py"""
import csv
from collections import deque
from datetime import datetime
import ipaddress
import math
import queue
import socket
import time
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
from network import Connection
from protocol import Sample, parse_sample
from view3d import draw, COLORS, BG


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title('BNO055 · IMU Studio')
        self.geometry('1120x800')
        self.minsize(920, 700)
        self.configure(bg='#091321')
        self.net = Connection()
        self.history = deque(maxlen=1200)
        self.latest = None
        self.reference = None
        self.demo = False
        self.start_time = time.monotonic()
        self.received = 0
        self.bad_lines = 0
        self.csv_file = None
        self.writer = None
        self.mode = tk.StringVar(value='Servidor')
        self.host = tk.StringVar(value='0.0.0.0')
        self.port = tk.StringVar(value='5000')
        self.unit = tk.StringVar(value='m/s²')
        self.status = tk.StringVar(value='Iniciando servidor…')
        self.info = tk.StringVar(value='Sin muestras')
        self.values = [tk.StringVar(value='—') for _ in range(7)]
        self.make_ui()
        self.protocol('WM_DELETE_WINDOW', self.close)
        self.after(200, self.connect)
        self.after(40, self.tick)

    def make_ui(self):
        style = ttk.Style(self)
        style.theme_use('clam')
        style.configure('TFrame', background='#091321')
        style.configure('TLabel', background='#091321', foreground='#dce7f7', font=('Segoe UI', 10))
        style.configure('TButton', font=('Segoe UI', 10), padding=7)
        style.configure('TCombobox', padding=5)
        root = ttk.Frame(self, padding=20)
        root.pack(fill='both', expand=True)
        ttk.Label(root, text='IMU STUDIO  /  BNO055', font=('Segoe UI', 22, 'bold')).pack(anchor='w')
        ttk.Label(root, text='Telemetría Wi-Fi · aceleración y orientación', foreground='#8ca2bd').pack(anchor='w', pady=(0,14))
        row = ttk.Frame(root)
        row.pack(fill='x')
        modebox = ttk.Combobox(row, textvariable=self.mode, values=['Servidor','Cliente'], state='readonly', width=10)
        modebox.pack(side='left', padx=(0,8))
        modebox.bind('<<ComboboxSelected>>', self.mode_changed)
        ttk.Label(row, text='IP').pack(side='left')
        ttk.Entry(row, textvariable=self.host, width=17).pack(side='left', padx=6)
        ttk.Label(row, text='Puerto').pack(side='left')
        ttk.Entry(row, textvariable=self.port, width=6).pack(side='left', padx=6)
        ttk.Button(row, text='Conectar / escuchar', command=self.connect).pack(side='left', padx=4)
        ttk.Button(row, text='Detener', command=self.stop).pack(side='left', padx=4)
        self.demo_btn = ttk.Button(row, text='Demo', command=self.toggle_demo)
        self.demo_btn.pack(side='right')
        ttk.Label(root, textvariable=self.status, foreground='#4ce0b3').pack(anchor='w', pady=(10,2))
        # Muestra IPs locales como ayuda, sin suponer cuál pertenece al hotspot.
        try:
            ips = sorted({r[4][0] for r in socket.getaddrinfo(socket.gethostname(), None, socket.AF_INET)})
        except OSError:
            ips = []
        ttk.Label(root, text='IPs del portátil: '+(', '.join(ips) or 'consulta ipconfig')+' · El ESP32 usa la IP del adaptador del punto de acceso.', foreground='#8ca2bd').pack(anchor='w')
        cards = ttk.Frame(root)
        cards.pack(fill='x', pady=14)
        for i, name in enumerate(('Ax','Ay','Az','|a|','Roll °','Pitch °','Yaw °')):
            frame = tk.Frame(cards, bg=BG, padx=12, pady=10)
            frame.pack(side='left', fill='x', expand=True, padx=(0,5))
            tk.Label(frame, text=name, bg=BG, fg=COLORS[i] if i<3 else '#91a5c2', font=('Segoe UI',10)).pack(anchor='w')
            tk.Label(frame, textvariable=self.values[i], bg=BG, fg='#ecf3ff', font=('Consolas',19,'bold')).pack(anchor='w')
        middle = ttk.Frame(root)
        middle.pack(fill='both', expand=True)
        middle.columnconfigure(0, weight=1)
        middle.columnconfigure(1, weight=1)
        middle.rowconfigure(0, weight=1)
        self.scene = tk.Canvas(middle, bg=BG, highlightthickness=0)
        self.scene.grid(row=0, column=0, sticky='nsew', padx=(0,10))
        self.graph = tk.Canvas(middle, bg=BG, highlightthickness=0)
        self.graph.grid(row=0, column=1, sticky='nsew')
        toolbar = ttk.Frame(root)
        toolbar.pack(fill='x', pady=10)
        ttk.Label(toolbar, text='Unidad recibida:').pack(side='left')
        ttk.Combobox(toolbar, textvariable=self.unit, values=['m/s²','g','sin especificar'], state='readonly', width=14).pack(side='left', padx=8)
        ttk.Button(toolbar, text='Centrar orientación', command=self.center).pack(side='left', padx=4)
        self.record_btn = ttk.Button(toolbar, text='Grabar CSV', command=self.record)
        self.record_btn.pack(side='left', padx=4)
        ttk.Label(root, textvariable=self.info, foreground='#8ca2bd').pack(anchor='w')
        self.log = tk.Text(root, height=5, bg='#0d192a', fg='#9db2cc', insertbackground='white', relief='flat', font=('Consolas',10), state='disabled')
        self.log.pack(fill='x', pady=(8,0))
        self.add_log('Servidor: abre este programa antes de reiniciar el ESP32. No necesitas escribir la IP del ESP32.')

    def mode_changed(self, event=None):
        self.host.set('0.0.0.0' if self.mode.get() == 'Servidor' else '192.168.137.2')

    def add_log(self, text):
        self.log.configure(state='normal')
        self.log.insert('end', f'{datetime.now():%H:%M:%S}  {text}\n')
        if int(self.log.index('end-1c').split('.')[0]) > 160:
            self.log.delete('1.0','40.0')
        self.log.see('end')
        self.log.configure(state='disabled')

    def reset(self):
        self.history.clear()
        self.latest = None
        self.reference = None
        self.received = 0
        self.bad_lines = 0
        self.start_time = time.monotonic()
        for v in self.values:
            v.set('—')

    def connect(self):
        try:
            port = int(self.port.get())
            if not 1 <= port <= 65535:
                raise ValueError('El puerto debe estar entre 1 y 65535.')
            host = str(ipaddress.IPv4Address(self.host.get().strip()))
            if self.mode.get() == 'Cliente' and host == '0.0.0.0':
                raise ValueError('En modo cliente escribe la IP del ESP32.')
            self.end_recording()
            self.net.start(self.mode.get(), host, port)
            self.demo = False
            self.demo_btn.configure(text='Demo')
            self.reset()
            self.status.set('Abriendo conexión…')
        except (ValueError, RuntimeError) as exc:
            messagebox.showerror('Conexión', str(exc))

    def stop(self):
        try:
            self.net.stop()
        except RuntimeError as exc:
            self.add_log(str(exc))
        while not self.net.events.empty():
            self.net.events.get_nowait()
        self.demo = False
        self.demo_btn.configure(text='Demo')
        self.end_recording()
        self.status.set('Detenido')

    def toggle_demo(self):
        if self.demo:
            self.connect()
            return
        self.stop()
        self.reset()
        self.demo = True
        self.demo_btn.configure(text='Salir de demo')
        self.status.set('DEMO · datos simulados, sin conexión al ESP32')

    def center(self):
        if self.latest is None or self.latest.roll is None:
            self.add_log('Para centrar la orientación hacen falta roll, pitch y yaw; tu trama actual solo contiene aceleración.')
        else:
            self.reference = (self.latest.roll, self.latest.pitch, self.latest.yaw)
            self.add_log('Referencia visual fijada. Los valores y el CSV conservan los ángulos originales.')

    def record(self):
        if self.csv_file:
            self.end_recording()
            return
        path = filedialog.asksaveasfilename(defaultextension='.csv', initialfile=f'bno055_{datetime.now():%Y%m%d_%H%M%S}.csv', filetypes=[('CSV','*.csv')])
        if not path:
            return
        try:
            self.csv_file = open(path, 'w', newline='', encoding='utf-8-sig')
            self.writer = csv.writer(self.csv_file, delimiter=';')
            self.writer.writerow(['fecha_pc','tiempo_s','ax','ay','az','modulo','unidad','roll_deg','pitch_deg','yaw_deg','origen'])
            self.csv_file.flush()
            self.record_btn.configure(text='Parar grabación')
            self.add_log('Grabando CSV: '+path)
        except OSError as exc:
            self.end_recording()
            messagebox.showerror('CSV',str(exc))

    def end_recording(self):
        if self.csv_file:
            try:
                self.csv_file.close()
            except OSError as exc:
                self.add_log('Error cerrando CSV: '+str(exc))
        self.csv_file = self.writer = None
        if hasattr(self, 'record_btn'):
            self.record_btn.configure(text='Grabar CSV')

    def accept_sample(self, sample):
        self.latest = sample
        self.history.append(sample)
        self.received += 1
        vals = (sample.ax, sample.ay, sample.az, sample.magnitude, sample.roll, sample.pitch, sample.yaw)
        for var, val in zip(self.values, vals):
            var.set('—' if val is None else f'{val:.2f}')
        if self.writer:
            try:
                self.writer.writerow([datetime.now().isoformat(timespec='milliseconds'), f'{sample.received-self.start_time:.4f}', sample.ax,sample.ay,sample.az,sample.magnitude,self.unit.get(),sample.roll,sample.pitch,sample.yaw,'demo' if self.demo else 'TCP'])
                self.csv_file.flush()
            except OSError as exc:
                self.add_log('Error guardando CSV: '+str(exc))
                self.end_recording()

    def plot(self):
        c = self.graph
        c.delete('all')
        w,h = c.winfo_width(), c.winfo_height()
        if w < 100 or h < 100:
            return
        c.create_text(16,20,anchor='nw',text='Aceleración · últimos 10 s',fill='#dce7f7',font=('Segoe UI',12,'bold'))
        end = time.monotonic()
        data = [s for s in self.history if end-s.received <= 10]
        bound = max(1, max((max(abs(s.ax),abs(s.ay),abs(s.az)) for s in data), default=10)*1.15)
        left,right,top,bottom = 56,w-18,60,h-45
        for i in range(5):
            y = top+(bottom-top)*i/4
            c.create_line(left,y,right,y,fill='#24354c')
            c.create_text(left-6,y,anchor='e',text=f'{bound*(1-i/2):.1f}',fill='#91a5c2',font=('Consolas',9))
        for i in range(6):
            x = left+(right-left)*i/5
            c.create_text(x,bottom+15,text=f'{-10+2*i}s',fill='#91a5c2',font=('Segoe UI',9))
        for attr,color,name in zip(('ax','ay','az'),COLORS,('X','Y','Z')):
            pts=[]
            for s in data:
                x = right-(end-s.received)/10*(right-left)
                y = (top+bottom)/2-getattr(s,attr)/bound*(bottom-top)/2
                pts.extend((x,y))
            if len(pts)>=4:
                c.create_line(*pts,fill=color,width=2)
            elif pts:
                x,y=pts
                c.create_oval(x-2,y-2,x+2,y+2,fill=color,outline='')
            c.create_text(w-105+'XYZ'.index(name)*30,27,text=name,fill=color,font=('Segoe UI',11,'bold'))
        c.create_text(left,h-12,anchor='w',text=self.unit.get()+' · escala automática',fill='#91a5c2')

    def tick(self):
        if self.demo:
            t = time.monotonic()-self.start_time
            factor = 1 if self.unit.get() == 'g' else 9.81
            self.accept_sample(Sample(time.monotonic(),factor*.3*math.sin(t),factor*.25*math.cos(t*.8),factor*.95,25*math.sin(t*.7),20*math.cos(t*.5),(t*22)%360))
        else:
            for _ in range(300):
                try:
                    kind,value = self.net.events.get_nowait()
                except queue.Empty:
                    break
                if kind == 'status':
                    self.status.set(value)
                    self.add_log(value)
                elif kind == 'log':
                    self.add_log(value)
                else:
                    sample = parse_sample(value)
                    if sample:
                        self.accept_sample(sample)
                    else:
                        self.bad_lines += 1
                        self.add_log('Texto / trama no IMU: '+value[:200])
        now = time.monotonic()
        hz = 0
        recent = [s for s in self.history if now-s.received <= 2]
        if len(recent)>1:
            hz=(len(recent)-1)/max(.001,recent[-1].received-recent[0].received)
        age = 'sin datos' if self.latest is None else f'última muestra hace {now-self.latest.received:.1f} s'
        if self.latest and now-self.latest.received>2:
            age += ' · DATOS ANTIGUOS'
        self.info.set(f'{self.received} muestras · {hz:.1f} Hz · {age} · {self.bad_lines} líneas de texto/no IMU · cola descartada: {self.net.dropped}')
        draw(self.scene,self.latest,self.unit.get(),self.reference)
        self.plot()
        self.after(40,self.tick)

    def close(self):
        self.stop()
        self.destroy()


if __name__ == '__main__':
    App().mainloop()
