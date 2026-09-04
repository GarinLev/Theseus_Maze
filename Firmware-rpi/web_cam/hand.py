import socket
import subprocess
import time
import sys
import os
import json
import select
import serial

stat_bfs = 0 
start = 1

ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=0.5)

def send_to_arduino(cmd):
    if ser:
        encoded_data = cmd.encode('ascii') 
        ser.write(encoded_data)

def read_from_arduino():
    if not ser:
        time.sleep(0.5)
        print("noardsend")
        return ""
    
    if ser.in_waiting > 0:
        byte = ser.read(1)
        s = byte.decode('ascii', errors='ignore')
        rest = ser.readline().decode('ascii', errors='ignore').strip()
        full_str = s + rest
        return full_str
    
    return ""

def wait_for_5_chars():
    while True:
        res = read_from_arduino()
        if len(res) == 5:
            break
        time.sleep(0.01)

def load_data_from_monitor():
    global stat_bfs
    filename = "maze_shared.json"
    while True:
        if os.path.exists(filename) and os.path.getsize(filename) > 0:
            with open(filename, "r") as f:
                content = f.read().strip()
            if content.startswith("{") and content.endswith("}"):
                data = json.loads(content)
                if isinstance(data, dict):
                    return data.get("matrix")
                return data
        time.sleep(0.01)

def get_global_walls(corn, sf, sr, sb, sl):
    walls = []
    if corn == 0 or corn == 360:
        if sf == 1: walls.append('up')
        if sr == 1: walls.append('right')
        if sb == 1: walls.append('down')
        if sl == 1: walls.append('left')
    elif corn == 90:
        if sf == 1: walls.append('right')
        if sr == 1: walls.append('down')
        if sb == 1: walls.append('left')
        if sl == 1: walls.append('up')
    elif corn == 180:
        if sf == 1: walls.append('down')
        if sr == 1: walls.append('left')
        if sb == 1: walls.append('up')
        if sl == 1: walls.append('right')
    elif corn == 270:
        if sf == 1: walls.append('left')
        if sr == 1: walls.append('up')
        if sb == 1: walls.append('right')
        if sl == 1: walls.append('down')
    return ",".join(walls)

def main():
    global stat_bfs, start
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    monitor_ip = "127.0.0.1"
    monitor_port = 5002

    def send_msg(text):
        sock.sendto(text.encode('utf-8'), (monitor_ip, monitor_port))

    rx, ry = 8, 8
    corn = 0

    go = []

    if start == 1:
        send_msg(f"pos:{rx},{ry}:{corn}")
        ser.write(b"s")
        start = 0

    while True:
        arduino_data = read_from_arduino()
        if not arduino_data:
            time.sleep(0.1)
            continue
        
        parts = arduino_data.split(',')
        if len(parts) < 4:
            continue
            
        if not (parts[0].strip().lstrip('-').isdigit() and 
                parts[1].strip().lstrip('-').isdigit() and 
                parts[2].strip().lstrip('-').isdigit() and 
                parts[3].strip().lstrip('-').isdigit()):
            continue
            
        sf = int(parts[0])
        sr = int(parts[1])
        sb = int(parts[2])
        sl = int(parts[3])

        lab = load_data_from_monitor()
        stat_bfs = 0
        
        if 0 <= ry * 2 < 33 and 0 <= rx * 2 < 33:
            if lab[ry * 2][rx * 2] == 1:
                stat_bfs = 1

        calculated_walls = get_global_walls(corn, sf, sr, sb, sl)
        send_msg(f"wall:{rx},{ry}:{calculated_walls}")
        send_msg(f"pos:{rx},{ry}:{corn}")

        n_walls = sum([sf, sr, sb, sl])

        if not go and (n_walls == 3 or stat_bfs == 1):
            print("RUN EXTERNAL BFS SCRIPT")
            send_msg("BFS started")
            try:
                result = subprocess.run([sys.executable, "bfs.py"], capture_output=True, text=True)
                out_str = result.stdout.strip()
                if out_str:
                    go = [cmd for cmd in out_str.split(',') if cmd]
                else:
                    go = []
            except Exception as e:
                print(f"Error executing bfs.py: {e}")
                go = []
            print("Path received from bfs.py:", go)

        if go:
            move = go.pop(0)
            
            target_corn = corn
            if move == 'd':   target_corn = 0
            elif move == 'r': target_corn = 90
            elif move == 'u': target_corn = 180
            elif move == 'l': target_corn = 270
            
            while corn != target_corn:
                diff = (target_corn - corn) % 360
                if diff == 90:
                    send_to_arduino('r')
                    wait_for_5_chars()
                    corn = (corn + 90) % 360
                elif diff == 270:
                    send_to_arduino('l')
                    wait_for_5_chars()
                    corn = (corn - 90) % 360
                else:
                    send_to_arduino('r')
                    wait_for_5_chars()
                    corn = (corn + 90) % 360
            
            send_to_arduino('u')
            wait_for_5_chars()
            if corn == 0 or corn == 360:   ry += 1
            elif corn == 180:              ry -= 1
            elif corn == 270:              rx -= 1
            elif corn == 90:               rx += 1
            
            send_msg(f"pos:{rx},{ry}:{corn}")

        else:
            if not sr:
                send_to_arduino('r')
                time.sleep(1.2)
                corn = (corn + 90) % 360
                if corn == 0 or corn == 360:  ry += 1
                elif corn == 180:             ry -= 1
                elif corn == 270:             rx -= 1
                elif corn == 90:              rx += 1
            elif not sf:
                send_to_arduino('u')
                time.sleep(1.5)
                if corn == 0 or corn == 360:  ry += 1
                elif corn == 180:             ry -= 1
                elif corn == 270:             rx -= 1
                elif corn == 90:              rx += 1
            elif not sl:
                send_to_arduino('l')
                time.sleep(1.2)
                corn = (corn - 90) % 360
                if corn == 0 or corn == 360:  ry += 1
                elif corn == 180:             ry -= 1
                elif corn == 270:             rx -= 1
                elif corn == 90:              rx += 1
            else:
                send_to_arduino('b')
                time.sleep(2.0)
                corn = (corn + 180) % 360
                if corn == 0 or corn == 360:  ry += 1
                elif corn == 180:             ry -= 1
                elif corn == 270:             rx -= 1
                elif corn == 90:              rx += 1
                
            send_msg(f"pos:{rx},{ry}:{corn}")

if __name__ == "__main__":
    main()