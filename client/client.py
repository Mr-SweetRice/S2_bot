import asyncio, argparse, struct, sys, signal, platform
from bleak import BleakClient, BleakScanner
import matplotlib.pyplot as plt
from matplotlib.widgets import Button


SERVICE_UUID = "496335ec-f73e-46c6-b743-c1ebd052703c"
DATA_CHAR_UUID = "6761501f-a33c-446f-aba8-c615b56392ec"
CONTROL_CHAR_UUID = "099f102d-d5c3-4a8d-9b0c-36f21f6ed4d9"
DEVICE_NAME = "32ESP"

async def find_device():
    # tenta por nome/serviço usando AdvertisementData
    try:
        dev = await BleakScanner.find_device_by_filter(
            lambda d, ad: (d.name == DEVICE_NAME) or (
                ad and getattr(ad, "service_uuids", None) and
                SERVICE_UUID.lower() in [u.lower() for u in ad.service_uuids]
            ),
            timeout=8.0
        )
        if dev:
            return dev.address
    except Exception:
        pass

    # fallback: descoberta simples por nome
    devices = await BleakScanner.discover(timeout=6.0)
    for d in devices:
        if d.name == DEVICE_NAME:
            return d.address

    # escolha manual
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

async def conexao():  
    addr = await find_device()
    if not addr:
        print("Dispositivo não encontrado.")
        sys.exit(2)

    async with BleakClient(addr) as client:
        if not client.is_connected:
            print("Falha na conexão.")
            sys.exit(3)
    return client

async def loopy(client): 
    stop = asyncio.Event()
    def _handle_sigint(*_):
        stop.set()
    try:
        signal.signal(signal.SIGINT, _handle_sigint)
    except Exception:
        pass

    await stop.wait()
    await client.stop_notify(DATA_CHAR_UUID)
async def run():
    client = conexao()
    print("conectado")
    loopy(client)

if __name__ == "__main__":
    asyncio.run(run())