import asyncio, struct, sys, platform, threading, queue
import time
from bleak import BleakClient, BleakScanner
import matplotlib.pyplot as plt
from matplotlib.widgets import Button, TextBox
from matplotlib.patches import Rectangle


# --------- CONFIG ---------
ADDRESS = None            # ex: "AA:BB:CC:DD:EE:FF" ou None para auto
LIGAR = False             # True, False ou None

SERVICE_UUID = "496335ec-f73e-46c6-b743-c1ebd052703c"
DATA_CHAR_UUID = "6761501f-a33c-446f-aba8-c615b56392ec"
CONTROL_CHAR_UUID = "099f102d-d5c3-4a8d-9b0c-36f21f6ed4d9"
P_CHAR_UUID = "9cec4ad9-5ac2-4bce-a6fc-95807282c60f"
I_CHAR_UUID = "f99b0cad-3ebe-46a9-b68a-265fa7be0fe6"
D_CHAR_UUID = "d1f23061-e268-4e96-8e56-974711ab37bb"
LINE_CHAR_UUID = "2c7f3f4e-0c8a-4d5f-9f23-4a6b7a1c9b10"
TIME_CHAR_UUID = "7b2c1f8a-9453-4b6c-a4f0-22c2df8b0c21"
POS_CHAR_UUID = "eda62e40-db53-405b-be92-b08efcfefc36"

DEVICE_NAME = "32ESP"

PKT_FMT = "<8HHH8B"
PKT_SIZE = struct.calcsize(PKT_FMT)
LINE_FMT = "<HB"
LINE_SIZE = struct.calcsize(LINE_FMT)
TIME_FMT = "<I"
TIME_SIZE = struct.calcsize(TIME_FMT)
POS_FMT = "<HH"   
POS_SIZE = struct.calcsize(POS_FMT)

# Fila de updates UI
ui_queue: "queue.Queue[tuple]" = queue.Queue()

# ---------- UI ----------
class UI32ESP:
    def __init__(self, title="32ESP"):
        self.fig = None
        self.ax1 = None
        self.ax2 = None
        self.ax3 = None
        self.ax4 = None
        self.btn = None
        self.line_qtr = None
        self.centroid_line = None
        self.bars = None
        self.x_mm = None
        self._botao_estado = 0
        self._control_cb = None
        self._p_cb = None
        self._i_cb = None
        self._d_cb = None
        self.ax1 = None
        self.ax4 = None
        self.dig_bars = None
        self.txt_dist = None
        self.txt_tipo = None
        self.bar_labels = []
        self.txt_time = None
        self.ax_sens = None
        self.sens_bars = None
        self.sens_texts = []
        self.bars_texts = []
        self.ax_dig = None
        self.dig_patches = []

        self.lines_history = [] 
        self.time_history = [] 
        self.lines_texts = []   
        self.time_texts = []  
        self.dist_sum = 0
        self.sum_text = None

        self.txt_pos = None
        self.ax_pos = None            # novo eixo
        self.pos_t = []               # tempo relativo (s)
        self.pos_target = []          # série target
        self.pos_real = []            # série real
        self.line_target = None
        self.line_real = None
        self.t0 = time.time()



        self._build(title)

    def _build(self, title):
        self.fig = plt.figure(title, constrained_layout=True)
        self.fig.set_size_inches(10, 7)
        plt.ion()

        grid = self.fig.add_gridspec(3, 3, width_ratios=[0.8, 1.2, 1], height_ratios=[1, 1, 2])

        # Gráfico posição (target x real) ao longo do tempo
        self.ax_pos = self.fig.add_subplot(grid[2, 0:2])
        (self.line_target,) = self.ax_pos.plot([], [], color="tab:blue")     # sem label
        (self.line_real,)   = self.ax_pos.plot([], [], color="tab:orange")   # sem label
        self.hline_target   = self.ax_pos.axhline(0, linestyle="--", color="tab:green")  # sem label
        self.ax_pos.set_xlabel("t (s)")
        self.ax_pos.set_ylabel("posição")
        self.ax_pos.set_xlim(0, 10)
        self.ax_pos.set_ylim(0, 1000)   # mudar max grafico


        # barras RPM
        self.ax2 = self.fig.add_subplot(grid[1, 0])
        x_pos = [0.1, 0.6]
        self.bars = self.ax2.bar(x_pos, [0, 0], width=0.4, color=["tab:orange"],)
        self.ax2.set_xticks([])
        self.ax2.set_yticks([])
        for s in self.ax2.spines.values():
            s.set_visible(False)
        self.ax2.set_ylim(-12.75, 255)
        self.bars_texts = [
            self.ax2.text(i, -0.075, "0", ha="center", va="top") for i in (0,1)
        ]
        self.ax2.set_xlim(-0.2, 1.2)
        # self.ax2.axis("off")

        # gráfico dos 8 sensores
        self.ax_sens = self.fig.add_subplot(grid[1, 1])
        x_idx = list(range(8))
        self.sens_bars = self.ax_sens.bar(x_idx, [0] * 8, width=0.8)
        self.ax_sens.set_xticks([])
        self.ax_sens.set_yticks([])
        for s in self.ax_sens.spines.values():
            s.set_visible(False)
        self.ax_sens.set_ylim(-204.8, 4096)
        self.sens_texts = [
            self.ax_sens.text(i, -0.075, "0", ha="center", va="top") for i in x_idx
        ]

        # textos no topo
        self.ax4 = self.fig.add_subplot(grid[:, 2])
        self.ax4.axis("off")

        self.ax3 = self.fig.add_subplot(grid[1, 2])
        self.ax3.axis("off")
        self.sum_text = self.ax3.text(
            0.9, 0.1, "", transform=self.ax3.transAxes,
            ha="right", va="bottom", fontsize=10, fontweight="bold", visible=False
        )

        # Botão L/D
        ax_btn = plt.axes([0.03, 0.85, 0.1, 0.075])
        self.btn = Button(ax_btn, "On/Off")
        self.btn.on_clicked(self._on_button)

        # Faixa de inputs PID
        ax_pid_band = self.fig.add_subplot(grid[0, 0])
        ax_pid_band.axis("off")

        left, bottom, width, height = ax_pid_band.get_position().bounds
        pad_x = 0.01
        box_w = ((width - 2 * pad_x) / 4.0)
        box_h = height * 0.2
        y = bottom + (height - box_h) / 1.5
        shift = -0.08

        ax_p = plt.axes([left + shift + 0 * box_w, y, box_w - pad_x, box_h])
        ax_i = plt.axes([left + shift + 1.2 * box_w, y, box_w - pad_x, box_h])
        ax_d = plt.axes([left + shift + 2.5 * box_w, y, box_w - pad_x, box_h])

        self.tb_p = TextBox(ax_p, "P:", initial="0.0", color="none", hovercolor="none")
        self.tb_i = TextBox(ax_i, "I:", initial="0.0", color="none", hovercolor="none")
        self.tb_d = TextBox(ax_d, "D:", initial="0.0", color="none", hovercolor="none")

        for tb in [self.tb_p, self.tb_i, self.tb_d]:
            for s in tb.ax.spines.values():
                s.set_visible(False)

        self.tb_p.on_submit(self._on_submit_p)
        self.tb_i.on_submit(self._on_submit_i)
        self.tb_d.on_submit(self._on_submit_d)

        # 8 quadrados digitais em fila
        self.ax_dig = self.fig.add_subplot(grid[0, 1])
        self.ax_dig.set_xlim(0, 8)
        self.ax_dig.set_ylim(0, 1)
        self.ax_dig.set_aspect('equal', adjustable='box')
        self.ax_dig.axis("off")

        self.dig_patches = []
        for i in range(8):
            rect = Rectangle((i*0.9 + 0.55, 0.25), 0.5, 0.5,  
                            linewidth=1, edgecolor="black", facecolor="0.85")
            self.ax_dig.add_patch(rect)
            self.dig_patches.append(rect)


        # Timer para drenar fila sem bloquear a UI
        self._timer = self.fig.canvas.new_timer(interval=50)
        self._timer.add_callback(self._drain_queue)
        self._timer.start()

        plt.show(block=False)

    def set_control_callback(self, cb):
        self._control_cb = cb

    def set_pid_callbacks(self, p_cb, i_cb, d_cb):
        self._p_cb, self._i_cb, self._d_cb = p_cb, i_cb, d_cb

    def _on_button(self, _event):
        self._botao_estado = 1 - self._botao_estado
        print(f"Botão -> {self._botao_estado}")
        if self._botao_estado:
            self.sum_text.set_visible(False) 
        else:
            self.sum_text.set_text(f"Soma das distâncias: {self.dist_sum}")
            self.sum_text.set_visible(True) 
        if self._control_cb:
            try:
                self._control_cb(self._botao_estado)
            except Exception as e:
                print("Falha ao enviar controle:", e)

    def _on_submit_p(self, text):
        try:
            v = float(text)
            if self._p_cb:
                self._p_cb(v)
        except ValueError:
            print("gP inválido")

    def _on_submit_i(self, text):
        try:
            v = float(text)
            if self._i_cb:
                self._i_cb(v)
        except ValueError:
            print("gI inválido")

    def _on_submit_d(self, text):
        try:
            v = float(text)
            if self._d_cb:
                self._d_cb(v)
        except ValueError:
            print("gD inválido")

    def _drain_queue(self):
        try:
            while True:
                item = ui_queue.get_nowait()
                tag = item[0]
                if tag == "data":
                    _, sensors, rpm1, rpm2, digitais = item
                    self.update_data(sensors, rpm1, rpm2, digitais)
                elif tag == "line":
                    _, distancia, linha_tipo = item
                    self.update_line(distancia, linha_tipo)
                elif tag == "time":
                    _, millis_now = item
                    self.update_time(millis_now)
                elif tag == "pos":
                    _, target, real = item
                    self.update_pos(target, real)
        except queue.Empty:
            pass

    def update_data(self, sensors, rpm1, rpm2, digitais):
        valores = [rpm1, rpm2]
        for bar, txt, val in zip(self.bars, self.bars_texts, valores):
            bar.set_height(val)
            txt.set_text(str(val))
            txt.set_position((bar.get_x() + bar.get_width() / 2, -0.075))

        # Sensores: alturas e rótulos embaixo
        for i, b in enumerate(self.sens_bars):
            b.set_height(sensors[i])

        self.ax_sens.set_ylim(-204.8, 4086)
        label_y = -204.8 * 0.5  # meio da margem inferior

        for i, v in enumerate(sensors):
            self.sens_texts[i].set_position((i, label_y))
            self.sens_texts[i].set_text(str(v))
        # Digitais -> quadrados
        for i, v in enumerate(digitais):
            self.dig_patches[i].set_facecolor("tab:blue" if v else "0.9")

        if self.fig:
            self.fig.canvas.draw_idle()


    def update_line(self, distancia, linha_tipo):
        self.dist_sum += int(distancia)
        # guarda
        self.lines_history.append((distancia, linha_tipo))
        # limpa textos antigos
        for t in self.lines_texts:
            t.remove()
        self.lines_texts.clear()

        y0 = 0.9          # ponto inicial vertical
        dy = 0.06          # espaçamento entre linhas

        for idx, (d, lt) in enumerate(self.lines_history):  # mais recente primeiro
            y = y0 - idx * dy
            self.lines_texts.append(
                self.ax4.text(0.12, y, f"dist={d}  |  linha={lt}  |",
                            transform=self.ax4.transAxes,
                            ha="left", va="bottom", fontsize=9)
            )
            
        if self.fig:
            self.fig.canvas.draw_idle()

    def update_time(self, millis_now):
        self.time_history.append((millis_now))
        # limpa textos antigos
        for t in self.time_texts:
            t.remove()
        self.time_texts.clear()

        y0 = 0.9          # ponto inicial vertical
        dy = 0.06          # espaçamento entre linhas
        for idx, (t) in enumerate(self.time_history):  # mais recente primeiro
            y = y0 - idx * dy
            self.time_texts.append(
                self.ax4.text(0.55, y, f" tempo={t}",
                            transform=self.ax4.transAxes,
                            ha="left", va="bottom", fontsize=9)
            )            
        if self.fig:
            self.fig.canvas.draw_idle()

    def update_pos(self, target, real):
        t = time.time() - self.t0
        self.pos_t.append(t)
        self.pos_target.append(float(target))
        self.pos_real.append(float(real))

        WINDOW = 60.0
        while self.pos_t and (t - self.pos_t[0] > WINDOW):
            self.pos_t.pop(0); self.pos_target.pop(0); self.pos_real.pop(0)

        self.line_target.set_data(self.pos_t, self.pos_target)
        self.line_real.set_data(self.pos_t, self.pos_real)

        # janela X dinâmica, Y fixo em 0–1000
        if self.pos_t:
            x0 = max(0, self.pos_t[0])
            x1 = self.pos_t[-1] if self.pos_t[-1] > 10 else 10
            self.ax_pos.set_xlim(x0, x1)

        # atualiza linha horizontal do target atual
        y = float(target)
        self.hline_target.set_ydata([y, y])

        if self.txt_pos is None:
            self.txt_pos = self.ax_pos.text(0.02, 0.95, "", transform=self.ax_pos.transAxes, va="top")
        self.txt_pos.set_text(f"tgt={target}  real={real}")

        if self.fig:
            self.fig.canvas.draw_idle()



    


# ---------- BLE ----------
class BLE32ESP:
    def __init__(
        self,
        device_name=DEVICE_NAME,
        svc_uuid=SERVICE_UUID,
        data_uuid=DATA_CHAR_UUID,
        ctrl_uuid=CONTROL_CHAR_UUID,
        p_uuid=P_CHAR_UUID,
        i_uuid=I_CHAR_UUID,
        d_uuid=D_CHAR_UUID,
        line_uuid=LINE_CHAR_UUID,
        time_uuid=TIME_CHAR_UUID,
        pos_uuid=POS_CHAR_UUID
    ):
        self.device_name = device_name
        self.svc_uuid = svc_uuid
        self.data_uuid = data_uuid
        self.ctrl_uuid = ctrl_uuid
        self.p_uuid = p_uuid
        self.i_uuid = i_uuid
        self.d_uuid = d_uuid
        self.line_uuid = line_uuid
        self.time_uuid = time_uuid
        self.pos_uuid = pos_uuid
        self.client: BleakClient | None = None
        self.loop: asyncio.AbstractEventLoop | None = None

    @staticmethod
    def parse_data(data: bytes):
        if len(data) != PKT_SIZE:
            return None
        s0, s1, s2, s3, s4, s5, s6, s7, rpm1, rpm2, d0, d1, d2, d3, d4, d5, d6, d7 = struct.unpack(
            PKT_FMT, data
        )
        return (s0, s1, s2, s3, s4, s5, s6, s7), rpm1, rpm2, (d0, d1, d2, d3, d4, d5, d6, d7)

    @staticmethod
    def parse_line(data: bytes):
        if len(data) != LINE_SIZE:
            return None
        distancia, linha_tipo = struct.unpack(LINE_FMT, data)
        return distancia, linha_tipo

    @staticmethod
    def parse_time(data: bytes):
        if len(data) != TIME_SIZE:
            return None
        (millis_now,) = struct.unpack(TIME_FMT, data)
        return millis_now
    
    @staticmethod
    def parse_pos(data: bytes):
        if len(data) != POS_SIZE:
            return None
        target, real = struct.unpack(POS_FMT, data)
        return target, real



    async def find_device(self, address: str | None):
        if address:
            return address
        try:
            dev = await BleakScanner.find_device_by_filter(
                lambda d, ad: (d.name == self.device_name)
                or (
                    ad
                    and getattr(ad, "service_uuids", None)
                    and self.svc_uuid.lower() in [u.lower() for u in ad.service_uuids]
                ),
                timeout=8.0,
            )
            if dev:
                return dev.address
        except Exception:
            pass

        devices = await BleakScanner.discover(timeout=6.0)
        for d in devices:
            if d.name == self.device_name:
                return d.address

        if not devices:
            return None

        print("Selecione o dispositivo:")
        for i, d in enumerate(devices):
            print(f"[{i}] {d.name} {d.address}")
        try:
            idx = int(input("Índice: ").strip())
            return devices[idx].address
        except Exception:
            return None

    def _on_notify_data(self, _sender, data: bytearray):
        parsed = self.parse_data(bytes(data))
        if not parsed:
            return
        sensors, rpm1, rpm2, digitais = parsed
        ui_queue.put(("data", sensors, rpm1, rpm2, digitais))

    def _on_notify_line(self, _sender, data: bytearray):
        parsed = self.parse_line(bytes(data))
        if not parsed:
            return
        distancia, linha_tipo = parsed
        ui_queue.put(("line", distancia, linha_tipo))

    def _on_notify_time(self, _sender, data: bytearray):
        millis_now = self.parse_time(bytes(data))
        if millis_now is None:
            return
        ui_queue.put(("time", millis_now))
    
    def _on_notify_pos(self, _sender, data: bytearray):
        parsed = self.parse_pos(bytes(data))
        if not parsed:
            return
        target, real = parsed
        ui_queue.put(("pos", target, real))


    async def _write_bool(self, uuid, flag: bool):
        if not self.client:
            return
        val = bytes([1 if flag else 0])
        try:
            await self.client.write_gatt_char(uuid, val, response=True)
        except Exception:
            await self.client.write_gatt_char(uuid, val, response=False)

    async def _write_float(self, uuid, value: float):
        if not self.client:
            return
        data = struct.pack("<f", value)
        try:
            await self.client.write_gatt_char(uuid, data, response=True)
        except Exception:
            await self.client.write_gatt_char(uuid, data, response=False)

    async def write_control(self, estado: int):
        await self._write_bool(self.ctrl_uuid, bool(estado))
        print(f"controle={'ligado' if estado else 'desligado'}")

    async def write_p(self, val: float):
        await self._write_float(self.p_uuid, val)
        print(f"gP -> {val}")

    async def write_i(self, val: float):
        await self._write_float(self.i_uuid, val)
        print(f"gI -> {val}")

    async def write_d(self, val: float):
        await self._write_float(self.d_uuid, val)
        print(f"gD -> {val}")

    # wrappers thread-safe
    def send_control(self, estado: int):
        if self.loop is None:
            return
        return asyncio.run_coroutine_threadsafe(self.write_control(estado), self.loop)

    def send_p(self, val: float):
        if self.loop is None:
            return
        return asyncio.run_coroutine_threadsafe(self.write_p(val), self.loop)

    def send_i(self, val: float):
        if self.loop is None:
            return
        return asyncio.run_coroutine_threadsafe(self.write_i(val), self.loop)

    def send_d(self, val: float):
        if self.loop is None:
            return
        return asyncio.run_coroutine_threadsafe(self.write_d(val), self.loop)

    async def run(self, address: str | None, ligar: bool | None):
        self.loop = asyncio.get_running_loop()
        addr = await self.find_device(address)
        if not addr:
            print("Dispositivo não encontrado.")
            return

        async with BleakClient(addr) as client:
            self.client = client
            if not client.is_connected:
                print("Falha na conexão.")
                return

            try:
                svcs = await client.get_services()
                if getattr(svcs, "get_characteristic", None):
                    if not svcs.get_characteristic(self.data_uuid):
                        print("DATA_CHAR_UUID não encontrado, prosseguindo mesmo assim.")
                    if not svcs.get_characteristic(self.ctrl_uuid):
                        print("CONTROL_CHAR_UUID não encontrado, prosseguindo mesmo assim.")
            except Exception:
                pass

            await client.start_notify(self.data_uuid, self._on_notify_data)
            await client.start_notify(self.line_uuid, self._on_notify_line)
            await client.start_notify(self.time_uuid, self._on_notify_time)
            await client.start_notify(self.pos_uuid,  self._on_notify_pos) 

            if ligar is not None:
                await self.write_control(1 if ligar else 0)

            print("BLE")
            try:
                await asyncio.Future()
            except asyncio.CancelledError:
                pass
            finally:
                try:
                    await client.stop_notify(self.data_uuid)
                except Exception:
                    pass


# ---------- Thread BLE ----------
def start_ble_thread(ble: BLE32ESP, address, ligar):
    def _runner():
        if platform.system() == "Windows":
            try:
                asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
            except Exception:
                pass
        asyncio.run(ble.run(address, ligar))

    t = threading.Thread(target=_runner, daemon=True)
    t.start()
    return t


# ---------- main ----------
if __name__ == "__main__":
    ui = UI32ESP(title="32ESP")
    ble = BLE32ESP()

    ui.set_control_callback(ble.send_control)
    ui.set_pid_callbacks(ble.send_p, ble.send_i, ble.send_d)

    # apenas enviar quando clicar
    start_ble_thread(ble, ADDRESS, LIGAR)

    while plt.get_fignums():
        plt.pause(0.05)
