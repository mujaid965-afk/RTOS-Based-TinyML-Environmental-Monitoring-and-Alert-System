import serial
import csv
import time
import re

# --- CONFIGURATION ---
SERIAL_PORT = 'COM4' 
BAUD_RATE = 115200
OUTPUT_FILE = "environmental_dataset.csv"

print(f"[INFO] Initializing Serial Monitor on {SERIAL_PORT}...")

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Allow port connection to stabilize
    print("[INFO] Connection established successfully.")
except Exception as e:
    print(f"[ERROR] Could not open port {SERIAL_PORT}. Close Tera Term first! Error: {e}")
    exit()

# Initialize CSV file with dataset headers
with open(OUTPUT_FILE, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(["Timestamp", "Temperature", "Pressure", "Gas_Level", "Label"])

print(f"[INFO] Dataset creation active. Saving records to '{OUTPUT_FILE}'...")
print("Press Ctrl+C to safely halt data collection.")

try:
    # State variables to store the parsed features
    current_temp = None
    current_press = None
    current_gas = None
    current_status = None

    while True:
        if ser.in_waiting > 0:
            # Read line and decode from raw binary to text string
            raw_line = ser.readline().decode('utf-8', errors='ignore').strip()
            
            # Master diagnostic trace print
            if raw_line:
                print(f"-> Master Debug Line: {raw_line}")
            
            # Unified parsing chain aligned with STM32 uppercase printing strings
            if "TEMP" in raw_line:
                match = re.search(r"[-+]?\d*\.\d+|\d+", raw_line)
                if match: 
                    current_temp = float(match.group())
                
            elif "GAS" in raw_line:
                match = re.search(r"\d+", raw_line)
                if match: 
                    current_gas = int(match.group())
                    
            elif "STAT" in raw_line:
                parts = raw_line.split(':')
                if len(parts) > 1:
                    status_str = parts[1].strip()
                    
                    # Map textual states to numerical labels (0=Normal, 1=Warning, 2=Danger)
                    if "NORMAL" in status_str or status_str == "":
                        current_status = 0
                    elif "WARNING" in status_str:
                        current_status = 1
                    elif "DANGER" in status_str:
                        current_status = 2

            # Hardcoded simulation backup for missing pressure sensor parameter
            if current_press is None:
                current_press = 1013.25

            # Save to file once an entire cluster has arrived
            if all(v is not None for v in [current_temp, current_press, current_gas, current_status]):
                timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
                
                with open(OUTPUT_FILE, mode='a', newline='') as file:
                    writer = csv.writer(file)
                    writer.writerow([timestamp, current_temp, current_press, current_gas, current_status])
                
                print(f"=== [RECORDED TO CSV] Temp: {current_temp}C, Gas: {current_gas}PPM, Label: {current_status} ===")
                
                # Reset tracking registers for the next upcoming cycle block
                current_temp = current_gas = current_status = None

except KeyboardInterrupt:
    print("\r\n[INFO] Data collection paused safely by user choice.")
    ser.close()
    print("[INFO] Serial connection closed cleanly.")