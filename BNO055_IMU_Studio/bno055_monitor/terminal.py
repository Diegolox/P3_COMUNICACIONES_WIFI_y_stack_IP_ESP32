"""Terminal de recepción: historial limitado y ventana independiente reutilizable."""
from collections import deque
from datetime import datetime
import tkinter as tk
from tkinter import ttk


class ReceiveTerminal:
    def __init__(self, parent):
        self.parent = parent
        self.history = deque(maxlen=1500)
        self.window = None
        self.paused = False
        self.text = None
        self.counter = None
        self.total = 0

    def open(self):
        """Un segundo clic trae al frente la misma ventana, sin duplicarla."""
        if self.window is not None and self.window.winfo_exists():
            self.window.deiconify()
            self.window.lift()
            self.window.focus_set()
            return
        self.paused = False
        win = self.window = tk.Toplevel(self.parent)
        win.title('IMU Studio · Terminal de recepción')
        win.geometry('850x500')
        win.minsize(650, 360)
        win.configure(bg='#091321')
        win.protocol('WM_DELETE_WINDOW', self.close)
        frame = ttk.Frame(win, padding=18)
        frame.pack(fill='both', expand=True)
        ttk.Label(frame, text='TERMINAL / DATOS RECIBIDOS', font=('Segoe UI', 16, 'bold')).pack(anchor='w')
        ttk.Label(frame, text='RX: líneas del ESP32 · DEMO: datos simulados · INFO: estado del programa', foreground='#91a5c2').pack(anchor='w', pady=(4,12))
        bar = ttk.Frame(frame)
        bar.pack(fill='x', pady=(0,12))
        self.pause_button = ttk.Button(bar, text='Pausar vista', style='Secondary.TButton', command=self.toggle_pause)
        self.pause_button.pack(side='left', padx=(0,8))
        ttk.Button(bar, text='Copiar todo', style='Secondary.TButton', command=self.copy).pack(side='left', padx=(0,8))
        ttk.Button(bar, text='Limpiar', style='Danger.TButton', command=self.clear).pack(side='left')
        self.counter = tk.StringVar()
        ttk.Label(bar, textvariable=self.counter, foreground='#91a5c2').pack(side='right')
        area = ttk.Frame(frame)
        area.pack(fill='both', expand=True)
        self.text = tk.Text(area, bg='#080f1c', fg='#dce7f7', insertbackground='#dce7f7',
                            selectbackground='#264c70', relief='flat', padx=12, pady=12,
                            font=('Consolas',11), wrap='none', state='disabled')
        vertical = ttk.Scrollbar(area, orient='vertical', command=self.text.yview)
        horizontal = ttk.Scrollbar(area, orient='horizontal', command=self.text.xview)
        self.text.configure(yscrollcommand=vertical.set, xscrollcommand=horizontal.set)
        area.rowconfigure(0, weight=1)
        area.columnconfigure(0, weight=1)
        self.text.grid(row=0, column=0, sticky='nsew')
        vertical.grid(row=0, column=1, sticky='ns')
        horizontal.grid(row=1, column=0, sticky='ew')
        for tag, color in [('RX','#4ce0b3'),('DEMO','#ffd174'),('INFO','#91a5c2')]:
            self.text.tag_configure(tag, foreground=color)
        ttk.Label(frame, text='Pausar congela la vista; la recepción y la grabación CSV siguen funcionando.', foreground='#91a5c2').pack(anchor='w', pady=(10,0))
        self.refresh()

    @staticmethod
    def format_entry(entry):
        stamp, source, line = entry
        return f'[{stamp}] {source:<4}  {line}\n'

    def push(self, line, source='RX'):
        # El contenido recibido se conserva sin interpretar; solo se añade hora y origen.
        entry = (datetime.now().strftime('%H:%M:%S.%f')[:-3], source, line)
        self.history.append(entry)
        self.total += 1
        if self.text is not None and not self.paused:
            follow = self.text.yview()[1] >= .99
            self.text.configure(state='normal')
            self.text.insert('end', self.format_entry(entry), source)
            # Text conserva un salto final adicional: 1500 entradas = 1501 líneas.
            excess = int(self.text.index('end-1c').split('.')[0])-1501
            if excess > 0:
                self.text.delete('1.0', f'{excess+1}.0')
            self.text.configure(state='disabled')
            if follow:
                self.text.see('end')
        self.update_counter()

    def update_counter(self):
        if self.counter is not None:
            self.counter.set(f'{self.total} líneas'+(' · PAUSA' if self.paused else ' · EN VIVO'))

    def refresh(self):
        if self.text is None:
            return
        self.text.configure(state='normal')
        self.text.delete('1.0','end')
        for entry in self.history:
            self.text.insert('end', self.format_entry(entry), entry[1])
        self.text.configure(state='disabled')
        self.text.see('end')
        self.update_counter()

    def toggle_pause(self):
        self.paused = not self.paused
        self.pause_button.configure(text='Reanudar vista' if self.paused else 'Pausar vista')
        if not self.paused:
            self.refresh()
        self.update_counter()

    def clear(self):
        self.history.clear()
        self.total = 0
        self.refresh()

    def copy(self):
        # Copia lo que está visible, también cuando la vista está pausada.
        self.parent.clipboard_clear()
        self.parent.clipboard_append(self.text.get('1.0','end-1c'))

    def close(self):
        self.window.destroy()
        self.window = self.text = self.counter = None
        self.paused = False
