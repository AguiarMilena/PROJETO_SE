import serial
import json
import time
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

# =========================
# CONFIGURAÇÕES
# =========================

SERIAL_PORT = "COM4"        # ajuste conforme necessário
BAUDRATE = 115200

INFLUX_URL = "http://localhost:8086"
INFLUX_TOKEN = "mesa_token_123"
INFLUX_ORG = "mesa"
INFLUX_BUCKET = "mesa_bucket"

# =========================
# CONEXÕES
# =========================

ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)

client = InfluxDBClient(
    url=INFLUX_URL,
    token=INFLUX_TOKEN,
    org=INFLUX_ORG
)

write_api = client.write_api(write_options=SYNCHRONOUS)

print("📡 Lendo serial e enviando para InfluxDB...")

print("⏳ Aguardando InfluxDB...")

while True:
    try:
        health = client.health()
        if health.status == "pass":
            break
    except:
        pass
    time.sleep(1)

print("✅ InfluxDB pronto")

# =========================
# LOOP PRINCIPAL
# =========================

while True:
    try:
        line = ser.readline().decode("utf-8").strip()
        if not line:
            continue

        data = json.loads(line)

        pitch = float(data["pitch"])
        roll = float(data["roll"])

        point = (
            Point("mesa")
            .field("pitch", pitch)
            .field("roll", roll)
        )

        write_api.write(
            bucket=INFLUX_BUCKET,
            org=INFLUX_ORG,
            record=point
        )

        print(f"✔ Pitch={pitch:.2f} | Roll={roll:.2f}")

    except json.JSONDecodeError:
        print("⚠ JSON inválido:", line)

    except Exception as e:
        print("❌ Erro:", e)
        time.sleep(1)
