#!/usr/bin/env python3

"""
host_script.py - Sends CPU usage to an Arduino-based VU-meter.

This script reads the overall CPU utilization of the system and sends it
via a serial port to an Arduino. The Arduino then displays this information
on an RGB LED strip as a VU-meter.

The serial protocol consists of a 4-byte packet:
[HEADER_1, HEADER_2, CPU_LOAD, COMMAND]
- HEADER_1 (0xAB): First byte of the packet header.
- HEADER_2 (0xBA): Second byte of the packet header.
- CPU_LOAD (0-100): Current overall CPU load percentage.
- COMMAND (0x00 for none, 0x01 to toggle LED display direction).
"""

import serial
import psutil
import time
import sys

# --- CONFIGURAZIONE ---
# Porta seriale a cui è collegato l'Arduino.
# Modifica questo valore in base al tuo sistema operativo:
# - Linux: '/dev/ttyUSB0', '/dev/ttyACM0' o simile
# - Windows: 'COM3', 'COM4' o simile
# - macOS: '/dev/cu.usbserial-XXXX' o simile
SERIAL_PORT = '/dev/ttyACM0'  # <--- MODIFICARE QUESTO VALORE
BAUD_RATE = 9600              # Deve corrispondere a Serial.begin() sull'Arduino
SEND_INTERVAL_SEC = 0.25      # Intervallo tra gli invii (secondi)

# --- PROTOCOLLO SERIALE ---
HEADER_1 = 0xAB
HEADER_2 = 0xBA
COMMAND_NONE = 0x00
COMMAND_TOGGLE_REVERSE = 0x01

def get_cpu_usage():
    """
    Returns the overall CPU usage percentage.
    """
    # psutil.cpu_percent() calcola l'utilizzo complessivo.
    # interval=None per una lettura non bloccante.
    # Si basa sull'ultima chiamata (o sulla prima dal boot/modulo importato).
    # Per letture accurate, si dovrebbe chiamare con un intervallo,
    # ma lo gestiamo noi con time.sleep().
    return psutil.cpu_percent(interval=None)

def main():
    """
    Main function to initialize serial, read CPU, and send data.
    """
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Connessione stabilita con Arduino su {SERIAL_PORT} a {BAUD_RATE} baud.")
    except serial.SerialException as e:
        print(f"Errore di connessione seriale: {e}", file=sys.stderr)
        print("Assicurati che la porta seriale sia corretta e non sia già in uso.", file=sys.stderr)
        sys.exit(1)

    # Inizializza psutil per ottenere una lettura significativa al primo giro
    psutil.cpu_percent(interval=None)

    try:
        while True:
            cpu_load = int(get_cpu_usage())
            # Assicurati che il valore sia tra 0 e 100
            cpu_load = max(0, min(100, cpu_load))

            # Crea il pacchetto di 4 byte
            # Per ora, il comando è sempre 0x00 (nessun comando)
            packet = bytearray([HEADER_1, HEADER_2, cpu_load, COMMAND_NONE])

            ser.write(packet)
            # print(f"Inviato: CPU={cpu_load}%") # Debug: decommenta per vedere gli invii

            time.sleep(SEND_INTERVAL_SEC)

    except KeyboardInterrupt:
        print("\nScript interrotto dall'utente.")
    except Exception as e:
        print(f"Si è verificato un errore inaspettato: {e}", file=sys.stderr)
    finally:
        if ser.is_open:
            ser.close()
            print("Porta seriale chiusa.")

if __name__ == "__main__":
    main()
